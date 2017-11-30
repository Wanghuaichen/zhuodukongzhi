#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "global.h"
#include "deftypes.h"
#include "platform_init.h"

extern SignalDef signal[2];
ChannelInfoDef g_channels[2];

float getFuelSpeed(uint8_t liquid,uint8_t temprature);
uint8_t findSignalOfChannel(uint8_t chSlct,PeakDef* signals,uint8_t sigLen,PeakDef *out);

static int cmp ( const void *a , const void *b )
{ return *(uint16_t *)a - *(uint16_t *)b; }

static uint16_t timeBufCopy[15];
void fuelSignalProcess()
{
	int i,j;
	uint8_t findRslt;	
    uint16_t levelTime = 0;
	PeakDef peak;
	float fuelSpeed,time,avgStableTime;
	
	findRslt = findSignalOfChannel(global.chSlct,&signal[global.chSlct].peaks[0],signal.numOfPeaks,&peak);
	switch(findRslt)
	{
		case 0: 
            levelTime = 0; 
            break;
		case 1: // keep the last measure value			
			break;
		case 2:
            levelTime = peak.peakIndex;
            break;
		default: break;
	}
    time = 0;
    for(i = 0; i < 15; i++)
    {
        time += g_channels[global.chSlct].timeStableBuf[i];
    }
    time /= 15;
    
    g_channels[global.chSlct].timeLatestBuf[g_channels[global.chSlct].timeLatestIndex ++] = levelTime;
    if(g_channels[global.chSlct].timeLatestIndex == 15)
        g_channels[global.chSlct].timeLatestIndex = 0;
    
	for(i = 0; i < 15; i++)
	{
		timeBufCopy[i] = g_channels[global.chSlct].timeLatestBuf[i];
	}
	qsort(timeBufCopy,15,2,cmp);
	
	if( (fabsf(timeBufCopy[5] - timeBufCopy[10]) < 30) && (timeBufCopy[5] >= 30) )
	{
		time = 0;
		for(i = 5; i < 10; i++)
		{
			time += timeBufCopy[i];                
		}
		time /= 5;
	}
	else
	{
		time = 0;
	}
	
	fuelSpeed = getFuelSpeed(g_channels[global.chSlct].liquidType,25/*g_channels[global.chSlct].temprature*/);
	
    g_channels[global.chSlct].levelValue = lroundf( g_channels[global.chSlct].sensorK *(fuelSpeed * time /1000.0f / 2.0f) + g_channels[global.chSlct].sensorB ) ;
	
}

float getFuelSpeed(uint8_t liquid,uint8_t temprature)
{
	const float speed[3][100] = {{1250,1250,},{1250,},{1250}};
	
	return 1250.0f;
}

uint8_t findSignalOfChannel(uint8_t chSlct,PeakDef* signals,uint8_t sigLen,PeakDef *out)
{
	int i,j,magFlag = 0;
	PeakDef sig_1st = signals[0];
	const float corseV = 1.25f;		//1250m/s 1.25ms/us
	
	if(sigLen <= 1)
    {
		return 0;	// no signal in channel
    }
	for(i = 1; i < sigLen; i++)
	{
		if( (signals[i].peakIndex - sig_1st.peakIndex)*corseV /2 <= g_channels[chSlct].tankDepth )
		{
			magFlag = 1;
			if(g_channels[chSlct].noise.numOfPeaks == 0)
			{
				out->peakIndex = signals[i].peakIndex - sig_1st.peakIndex; 
				out->peakMag = signals[i].peakMag;
				out->peakWidth = signals[i].peakWidth;
				return 2;	// find relect signal
			}			
			for(j = 0; j < g_channels[chSlct].noise.numOfPeaks; j++)
			{
				if( fabsf(g_channels[chSlct].noise.peaks[j].peakIndex - signals[i].peakIndex) <= 8 &&\
				   fabsf(g_channels[chSlct].noise.peaks[j].peakMag - signals[i].peakMag) <= 200 )
				{
					break;
				}
			}
			if(j == g_channels[chSlct].noise.numOfPeaks)
			{
				out->peakIndex = signals[i].peakIndex - sig_1st.peakIndex; 
				out->peakMag = signals[i].peakMag;
				out->peakWidth = signals[i].peakWidth;
				return 2;	// find relect signal
			}
		}		
	}
	if(i == sigLen)
	{
		if(magFlag == 0)
			return 0;	// no signal in channel
		else
			return 1;	// all signals in channel are noises
	}
    return 0;
}

uint8_t noiseExtraction(PeakDef* signals,uint8_t sigLen,uint16_t refIndex,PeakDef* noises,uint8_t noiseLen)
{
	int i,j,k,realLevelIndex,diff,diff_tmp;
	uint16_t reflect[4];
	
	for(i = 1; i < sigLen; i++)
	{
		if( fabsf(signals[i].peakIndex - refIndex) <= 20 )
		{
			realLevelIndex = i;
			break;
		}
	}
	if(i == 1)
	{
		return 0;
	}
	
	k = 0;
	for(i = 0; i < noiseLen; i++)
	{
		noises[i].peakIndex = 0;
		noises[i].peakMag = 0;
		noises[i].peakWidth = 0;
	}	
	diff = signals[realLevelIndex].peakIndex - signals[0].peakIndex;
	reflect[0] = diff * 2; reflect[1] = diff * 3; reflect[2] = diff * 4; reflect[3] = diff * 5;
	for(i = 1; i < sigLen; i++)
	{
		if(i < realLevelIndex)
		{
			noises[k++] = signals[i];
		}
		else if(i == realLevelIndex)
		{}
		else
		{
			diff_tmp = signals[i].peakIndex - signals[0].peakIndex;
			if( fabsf(diff_tmp - reflect[0]) < 5 && signals[i].peakMag <= signals[realLevelIndex].peakMag + 100)
			{
				continue;
			}
			else if(  fabsf(diff_tmp - reflect[1]) < 10 && signals[i].peakMag < signals[realLevelIndex].peakMag )
			{
				continue;
			}
			else if(  fabsf(diff_tmp - reflect[2]) < 15 && signals[i].peakMag < signals[realLevelIndex].peakMag )
			{
				continue;
			}
			else if(  fabsf(diff_tmp - reflect[3]) < 15 && signals[i].peakMag < signals[realLevelIndex].peakMag )
			{
				continue;
			}
			else
			{
				noises[k++] = signals[i];
			}
		}		
	}
	return k;
}

void clearNoise(PeakDef* noises,uint8_t len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		noises[i].peakIndex = 0;
		noises[i].peakMag = 0;
		noises[i].peakWidth = 0;
	}	
}

uint8_t getNoiseLen(PeakDef* noise,uint8_t len)
{
	int i,j = 0;
	for(i = 0; i < len; i++)
	{
		if(noise[i].peakIndex == 0 || noise[i].peakMag == 0)
			j++;
	}
	return j;
}
