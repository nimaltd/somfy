
#include "somfy.h"

//###############################################################################################################
void  somfy_init(somfy_t *somfy, TIM_HandleTypeDef *tim, GPIO_TypeDef *gpio, uint16_t pin)
{
  somfy->tim = tim;
  somfy->gpio = gpio;
  somfy->pin = pin;
  HAL_TIM_Base_Start(somfy->tim);    
}
//###############################################################################################################
void somfy_pinchange_callback(somfy_t *somfy)
{
  if (somfy->buff_index < (_SOMFY_BUFF_SIZE * 8) - 1)
  {
    somfy->buff[somfy->buff_index] = (somfy->tim->Instance->CNT - somfy->tim_cnt_last) & 0x7F;
    if (HAL_GPIO_ReadPin(somfy->gpio, somfy->pin))
      somfy->buff[somfy->buff_index] |= 0x80;
    somfy->buff_index++;
  }  
  somfy->tim_cnt_last = somfy->tim->Instance->CNT;  
}
//###############################################################################################################
void somfy_loop(somfy_t *somfy)
{
  
}
//###############################################################################################################
