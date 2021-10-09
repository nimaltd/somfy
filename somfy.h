#ifndef _SOMFY_H
#define _SOMFY_H


/*
  Author:     Nima Askari
  WebSite:    http://www.github.com/NimaLTD
  Instagram:  http://instagram.com/github.NimaLTD
  Youtube:    https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw
  
  Version:    1.0.0
  
  
  Reversion History:
  
  (1.0.0)
  First release.
  
*/

#include <stdbool.h>
#include "main.h"
#include "somfy_config.h"


typedef struct
{
  TIM_HandleTypeDef *tim; 
  GPIO_TypeDef      *gpio;
  uint16_t          pin;
  uint16_t          tim_cnt_last;  
  int16_t           buff_index;
  uint8_t           buff[128]; 
  uint8_t           detect_frame;
  uint8_t           data[7];
  
}somfy_t;

//###############################################################################################################
void    somfy_init(somfy_t *somfy, TIM_HandleTypeDef *tim, GPIO_TypeDef *gpio, uint16_t pin); // 100 us timer tick 
void    somfy_pinchange_callback(somfy_t *somfy);
bool    somfy_available(somfy_t *somfy);
void    somfy_reset_available(somfy_t *somfy);
uint8_t somfy_read(somfy_t *somfy, uint16_t *rolling, uint8_t *address_3byte, uint8_t *key);
//###############################################################################################################

#endif
