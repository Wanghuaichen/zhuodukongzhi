#include <stdint.h>
#include "lcd12864.h"
#include "12864_ui.h"
#include "bsp_button.h"

//static void disp_number_box(uint8_t x,uint8_t y,char *p,uint8_t width,uint8_t reverse_index);
static void disp_number_box(NumberBox_t* pBox);

BOX_STAT set_number_box(uint8_t msg,NumberBox_t* pBox)
{
    uint8_t ascii,y;
    uint8_t data_type = pBox->data_type;
	double retVal;
	const char* yesno[] = {"<-Yes       No->","<-是        否->"};
    static uint8_t buf[2][128];	
    
	switch(pBox->box_stat)
	{
		case MODIFING:
		//disp_number_box(pBox->x,pBox->y,pBox->set_buf,pBox->box_width,pBox->set_index);
        disp_number_box(pBox);
		break;
		case SAVING:
		//disp_number_box(pBox->x,pBox->y,pBox->set_buf,pBox->box_width,pBox->set_index);
		if(pBox->y != 32)
		{
			y = 32;
		}
		else
		{
			y = 16;
		}
		LCD_DisplayString(0,y,(char*)yesno[1],NOT_REVERSE);
		break;
		default: break;
		
	}

	switch(pBox->box_stat)
	{
		case UNFOCUSED:
			pBox->set_index = 0;
			switch(data_type)
			{   
			case INTEGER:  
				snprintf(pBox->set_buf,pBox->box_width+1,"%-*d",pBox->box_width,pBox->set_integer); 
				break;
			case FLOAT:
				snprintf(pBox->set_buf,pBox->box_width+1,"%-*f",pBox->box_width,pBox->set_float);
				break;
			default: break;
			} 
			LCD_DisplayString(pBox->x,pBox->y,pBox->set_buf,NOT_REVERSE);
			return pBox->box_stat;
		case FOCUSED:
			pBox->set_index = 0;
			switch(data_type)
			{   
			case INTEGER:  
				snprintf(pBox->set_buf,pBox->box_width+1,"%-*d",pBox->box_width,pBox->set_integer); 
				break;
			case FLOAT:
				snprintf(pBox->set_buf,pBox->box_width+1,"%-*f",pBox->box_width,pBox->set_float); 		
				break;
			default: break;
			} 
			LCD_DisplayString(pBox->x,pBox->y,pBox->set_buf,REVERSE);
			if(msg == KEY_DOWN_OK && pBox->is_readonly == 0)
			{
                if(pBox->y != 32)
                {
                    get_line_dat(4,&buf[0][0]);
                    get_line_dat(5,&buf[1][0]);
                }
                else
                {
                    get_line_dat(2,&buf[0][0]);
                    get_line_dat(3,&buf[1][0]);
                }
                
				pBox->box_stat = MODIFING;
                switch(data_type)
                {   
                case INTEGER:  
                    snprintf(pBox->set_buf,pBox->box_width+1,"%0*d",pBox->data_width,pBox->set_integer);
                    break;
                case FLOAT:
                    snprintf(pBox->set_buf,pBox->box_width+1,"%0*f",pBox->data_width,pBox->set_float);
                    break;
                default: break;
                } 
			}
			else if( msg == KEY_DOWN_DOWN || msg == KEY_DOWN_UP || msg == KEY_DOWN_MENU)
			{
				pBox->box_stat = UNFOCUSED;
			}
			return pBox->box_stat;
		case MODIFING:
			switch(msg)
			{
			case KEY_DOWN_UP:
					ascii = ++pBox->set_buf[pBox->set_index];					
                    switch(ascii)
                    {
                    case '9' + 1:
                        if(pBox->data_type == FLOAT) 
                            pBox->set_buf[pBox->set_index] = '.';
                        else
                            pBox->set_buf[pBox->set_index] = '0';
                        return pBox->box_stat;
                    case '.' + 1: pBox->set_buf[pBox->set_index] = '0'; return pBox->box_stat;
                    default: return pBox->box_stat;
                    }					
					return pBox->box_stat;
			case KEY_DOWN_DOWN:			
				ascii = --pBox->set_buf[pBox->set_index];				
                switch(ascii)
                {
                case '0' - 1:
                    if(pBox->data_type == FLOAT)
                        pBox->set_buf[pBox->set_index] = '.';
                    else
                        pBox->set_buf[pBox->set_index] = '9';
                    return pBox->box_stat;
                case '.' - 1: pBox->set_buf[pBox->set_index] = '9'; return pBox->box_stat;
                default: return pBox->box_stat;
                }
				return pBox->box_stat;
			case KEY_DOWN_MENU:
				pBox->box_stat = SAVING;
				return pBox->box_stat;
			case KEY_DOWN_OK:
				if(++pBox->set_index == pBox->data_width) pBox->set_index = 0; return pBox->box_stat;
				default:
				break;
			}
		case SAVING:
				switch(msg)
				{
					case KEY_DOWN_MENU:
						switch(data_type)
						{
						case INTEGER: sscanf(pBox->set_buf,"%lf",&retVal); break;
						case FLOAT: sscanf(pBox->set_buf,"%lf",&retVal); break;	
						default: break;
						}
						if(retVal < pBox->max && retVal > pBox->min)
						{
							switch(data_type)
							{
							case INTEGER: pBox->set_integer = retVal; break;
							case FLOAT: pBox->set_float = retVal; break;
							default: break;
							}
						}
						pBox->box_stat = FOCUSED; 
                        if(pBox->y != 32)
                        {
                            set_line_dat(4,&buf[0][0]);
                            set_line_dat(5,&buf[1][0]);
                        }
                        else
                        {
                            set_line_dat(2,&buf[0][0]);
                            set_line_dat(3,&buf[1][0]);
                        }
                        return pBox->box_stat;
					break;
					case KEY_DOWN_OK:
						pBox->box_stat = FOCUSED; 
                        if(pBox->y != 32)
                        {
                            set_line_dat(4,&buf[0][0]);
                            set_line_dat(5,&buf[1][0]);
                        }
                        else
                        {
                            set_line_dat(2,&buf[0][0]);
                            set_line_dat(3,&buf[1][0]);
                        }
                        return pBox->box_stat;
					break;
					default:
					break;
				}
	}
	return pBox->box_stat ;
}

static void disp_number_box(NumberBox_t* pBox)
{
    char buf[17];
    uint8_t *p = pBox->set_buf;
    int len = strlen(p);
    uint8_t x = pBox->x, y = pBox->y , width = pBox->box_width,reverse_index = pBox->set_index;
    if(pBox->is_show == 1)
    {
        snprintf(buf,width+1,"%.*s",reverse_index,p);    
        LCD_DisplayString(x,y,buf,NOT_REVERSE);
        buf[0] = p[reverse_index];buf[1] = 0;
        LCD_DisplayString( x+reverse_index*8,y,buf,REVERSE);
        snprintf(buf,width+1,"%s",&p[reverse_index + 1]);    
        LCD_DisplayString( x+(1+reverse_index)*8,y,buf,NOT_REVERSE);  
    }
}

void label_show(Label_t *label)
{
    if(label->is_show == 1)
    {
        LCD_DisplayString(label->x,label->y,label->label_buf,NOT_REVERSE);
    }
}

