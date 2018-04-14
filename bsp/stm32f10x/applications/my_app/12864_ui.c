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
	static uint8_t buf[2][128];	
    
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
			break;
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
			break;
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
                        break;
                    case '.' + 1: pBox->set_buf[pBox->set_index] = '0'; break;
                    default: break;
                    }					
					break;
			case KEY_DOWN_DOWN:			
				ascii = --pBox->set_buf[pBox->set_index];				
                switch(ascii)
                {
                case '0' - 1:
                    if(pBox->data_type == FLOAT)
                        pBox->set_buf[pBox->set_index] = '.';
                    else
                        pBox->set_buf[pBox->set_index] = '9';
                    break;
                case '.' - 1: pBox->set_buf[pBox->set_index] = '9'; break;
                default: break;
                }
				break;
			case KEY_DOWN_MENU:
				pBox->box_stat = SAVING;
				break;
			case KEY_DOWN_OK:
				if(++pBox->set_index == pBox->data_width) pBox->set_index = 0; break;
				default:
				break;
			}
            break;
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
                    break;
                break;
                default:
                break;
            }
            break;
           default: break;
	}
    disp_number_box(pBox);
	return pBox->box_stat ;
}

static void disp_number_box(NumberBox_t* pBox)
{
    char buf[17];
    const char* yesno = "<-保存    退出->";
    uint8_t *p = pBox->set_buf;
    int len = strlen(p);
    uint8_t x = pBox->x, y = pBox->y , width = pBox->box_width,reverse_index = pBox->set_index;
    if(pBox->is_show == 1)
    {
        switch(pBox->box_stat)
        {
            case UNFOCUSED:
                LCD_DisplayString(pBox->x,pBox->y,pBox->set_buf,NOT_REVERSE);                
                break;
            case FOCUSED:
                LCD_DisplayString(pBox->x,pBox->y,pBox->set_buf,REVERSE);
                break;
            case MODIFING:
                snprintf(buf,width+1,"%.*s",reverse_index,p);    
                LCD_DisplayString(x,y,buf,NOT_REVERSE);
                buf[0] = p[reverse_index];buf[1] = 0;
                LCD_DisplayString( x+reverse_index*8,y,buf,REVERSE);
                snprintf(buf,width+1,"%s",&p[reverse_index + 1]);    
                LCD_DisplayString( x+(1+reverse_index)*8,y,buf,NOT_REVERSE);  
                break;
            case SAVING:
                if(pBox->y != 32)
                {
                    y = 32;
                }
                else
                {
                    y = 16;
                }
                LCD_DisplayString(0,y,(char*)yesno,NOT_REVERSE);
                break;
            default:
                break;
        }
        
    }
}

void label_show(Label_t *label)
{
    if(label->is_show == 1)
    {
        LCD_DisplayString(label->x,label->y,label->label_buf,NOT_REVERSE);
    }
}

static void btn_display(Button_t *btn);
uint8_t btn_proc(uint8_t msg,Button_t *btn)
{
    switch(btn->status)
    {
        case UNFOCUSED:
            break;
        case FOCUSED:
            if(msg == KEY_DOWN_OK)
            {
                if(btn->btn_event != NULL)
                {
                    btn->btn_event(btn->para);
                }
            }
            break;
        default:break;
    }
    btn_display(btn);
    return btn->status;
}

void btn_display(Button_t *btn)
{
    if(btn->is_show == 1)
    {
        if(btn->status == FOCUSED)
        {
            LCD_DisplayString(btn->x,btn->y,btn->content,REVERSE);
        }
        else
        {
            LCD_DisplayString(btn->x,btn->y,btn->content,NOT_REVERSE);
        }
    }
}

static void cmb_box_display(ComboBox_t *btn);
uint8_t combo_box_proc(uint8_t msg,ComboBox_t *cmb_box)
{
    switch(cmb_box->status)
    {
        case UNFOCUSED:
            break;
        case FOCUSED:
            switch(msg)
            {
                case KEY_DOWN_DOWN:
                    cmb_box->index++;
                    if(cmb_box->index >= cmb_box->totals)
                    {
                        cmb_box->index = cmb_box->totals - 1;
                    }
                    break;
                case KEY_DOWN_UP:
                    cmb_box->index--;
                    if(cmb_box->index < 0 )
                    {
                        cmb_box->index = 0;
                    }
                    break;
                case KEY_DOWN_OK:
                    if(cmb_box->cmb_box_event != NULL)
                    {
                        cmb_box->cmb_box_event(cmb_box->para);
                    }
                default: break;
            }
            break;
        default:break;
    }
    cmb_box_display(cmb_box);
    return cmb_box->status;
}

void cmb_box_display(ComboBox_t *cmb_box)
{
    if(cmb_box->is_show == 1)
    {
        if(cmb_box->status == FOCUSED)
        {
            LCD_DisplayString(cmb_box->x,cmb_box->y,cmb_box->content[cmb_box->index],REVERSE);
        }
        else
        {
            LCD_DisplayString(cmb_box->x,cmb_box->y,cmb_box->content[cmb_box->index],NOT_REVERSE);
        }
    }
}
