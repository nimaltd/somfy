
#include "somfy.h"
#include <string.h>

#define _SOMFY_TOLERANCE_CNV      (_SOMFY_TOLERANCE / 100)
#define _SOMFY_TIME_CNV           (_SOMFY_TIME / 100)  
#define _SOMFY_TIME_SYNC_CNV      (_SOMFY_TIME_SYNC / 100)

//###############################################################################################################
void  somfy_init(somfy_t *somfy, TIM_HandleTypeDef *tim, GPIO_TypeDef *gpio, uint16_t pin)
{
  somfy->tim = tim;
  somfy->gpio = gpio;
  somfy->pin = pin;
  somfy->detect_frame = 0;
  somfy->buff_index = -1;
  HAL_TIM_Base_Start(somfy->tim);    
}
//###############################################################################################################
void somfy_pinchange_callback(somfy_t *somfy)
{
  //  +++ detect start frame
  if (somfy->detect_frame < 0xF0)
  {
    do
    {
      if (somfy->buff_index >= 0)
      {
        somfy->buff[somfy->buff_index] = (somfy->tim->Instance->CNT - somfy->tim_cnt_last);
        if ((somfy->buff[somfy->buff_index] < (_SOMFY_TIME_SYNC_CNV + _SOMFY_TOLERANCE_CNV)) && (somfy->buff[somfy->buff_index] > (_SOMFY_TIME_SYNC_CNV - _SOMFY_TOLERANCE_CNV)))
        {
          somfy->buff_index = 0;
          somfy->detect_frame = 0xFF;
          HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
          break;
        }
        else
        {
          somfy->buff_index = -1;
          somfy->detect_frame = 0;
          break;
        }
      }
      else
      {
        //  detect first High edge
        if (HAL_GPIO_ReadPin(somfy->gpio, somfy->pin) == GPIO_PIN_SET)
          somfy->buff_index++;
        else
          break;
      }      
    }
    while (0);      
  }  
  //  --- detect start frame
  else if (somfy->detect_frame == 0xFF)
  //  +++ data 
  {
    somfy->buff[somfy->buff_index] = (somfy->tim->Instance->CNT - somfy->tim_cnt_last);
    if (HAL_GPIO_ReadPin(somfy->gpio, somfy->pin))
      somfy->buff[somfy->buff_index] |= 0x80;
    if ((somfy->buff[somfy->buff_index] & 0x7F) > _SOMFY_TIME_SYNC_CNV)
      somfy->detect_frame = 0xFE;      
    else if ((somfy->buff[somfy->buff_index] & 0x7F) < (_SOMFY_TIME_CNV / 2) - _SOMFY_TOLERANCE_CNV)
    {
      somfy->detect_frame = 0;
      somfy->buff_index = -1;
    }
    if (somfy->buff_index < sizeof(somfy->buff) - 1)    
      somfy->buff_index++;
  }
  //  --- data 
  somfy->tim_cnt_last = somfy->tim->Instance->CNT;  
}
//###############################################################################################################
uint8_t idx = 0;
bool somfy_available(somfy_t *somfy)
{
  if (somfy->detect_frame == 0xFE)
  {
    
    int8_t bit = 7;
    int8_t byte = 0;
    memset(&somfy->data, 0, sizeof(somfy->data));
    idx = 0;
    while (idx < somfy->buff_index - 1)
    {
      uint8_t data = somfy->buff[idx] & 0x7F;
      if ((data <= (_SOMFY_TIME_CNV + _SOMFY_TOLERANCE_CNV)) && (data >= (_SOMFY_TIME_CNV - _SOMFY_TOLERANCE_CNV)))
      {
        if (somfy->buff[idx] & 0x80)
          somfy->data[byte] |= 1 << bit;
        idx++;
      }
      else if ((data <= ((_SOMFY_TIME_CNV / 2)+ _SOMFY_TOLERANCE_CNV)) && (data >= ((_SOMFY_TIME_CNV / 2) - _SOMFY_TOLERANCE_CNV)))
      {
        idx++;
        if (idx == somfy->buff_index - 1)
        {
          bit = 7;
          if (somfy->buff[idx - 2] & 0x80)
            somfy->data[byte] |= 1 << bit;
          continue;
        }
        data = somfy->buff[idx - 1] & 0x7F;
        if ((data <= ((_SOMFY_TIME_CNV / 2)+ _SOMFY_TOLERANCE_CNV)) && (data >= ((_SOMFY_TIME_CNV / 2) - _SOMFY_TOLERANCE_CNV)))
        {
          if (somfy->buff[idx] & 0x80)
            somfy->data[byte] |= 1 << bit;
          idx++;
        }
        else
          continue;
      }
      else
      {
        somfy->detect_frame = 0;
        somfy->buff_index = -1;
        return false;          
      }
      bit--;
      if (bit == -1)
      {
        bit = 7;       
        byte++;
      }
    }
    if (byte == 7 && bit == 7)
    {
      uint8_t temp[7];
      memcpy(temp, somfy->data, 7);        
      for (idx=1; idx < 7; idx++)
        somfy->data[idx] = temp[idx] ^ temp[idx - 1];
      
      uint8_t checksum = 0;
      for (idx = 0; idx < 7; idx++)
        checksum = checksum ^ somfy->data[idx] ^ (somfy->data[idx] >> 4);
      if ((checksum & 0x0F) != 0)
      {
        somfy->detect_frame = 0;
        somfy->buff_index = -1;
        return false;
      }
      return true;       
    }
    else
    {
      somfy->detect_frame = 0;
      somfy->buff_index = -1;
      return false;
    }
  }
  else  
    return false;
}
//###############################################################################################################
void somfy_reset_available(somfy_t *somfy)
{
  somfy->detect_frame = 0;
  somfy->buff_index = -1;
}
//###############################################################################################################
uint8_t somfy_read(somfy_t *somfy, uint16_t *rolling, uint8_t *address_3byte, uint8_t *key)
{
  if (rolling != NULL)
    *rolling = (somfy->data[2] << 8) | somfy->data[3];
  if (address_3byte != NULL)
    memcpy(address_3byte, &somfy->data[4], 3);
  if (key != NULL)
    *key = somfy->data[0];
  return ((somfy->data[1] & 0xF0) >> 4);  
}
//###############################################################################################################
