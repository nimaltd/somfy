#ifndef _SOMFY_H
#define _SOMFY_H

#include "main.h"
#include "somfy_config.h"

#define   _SOMFY_TIME      12

typedef struct
{
  TIM_HandleTypeDef *tim; 
  GPIO_TypeDef      *gpio;
  uint16_t          pin;
  uint16_t          tim_cnt_last;  
  uint16_t          buff_index;
  uint8_t           buff[_SOMFY_BUFF_SIZE * 8];  
  
}somfy_t;

//###############################################################################################################
void  somfy_init(somfy_t *somfy, TIM_HandleTypeDef *tim, GPIO_TypeDef *gpio, uint16_t pin); // 100 us timer tick 
void  somfy_pinchange_callback(somfy_t *somfy);
void  somfy_loop(somfy_t *somfy);
//###############################################################################################################

#endif
