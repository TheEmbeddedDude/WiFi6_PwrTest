/**
  ******************************************************************************
  * @file    mod_esp_now.h
  * @author  The Embedded Dude
  * @brief   ESP now module to handle ESP now communication  
  * @date    Git controlled
  * @version Git controlled

==============================================================================
                     ##### How to use this module #####
  ==============================================================================
    [..]
    {USER TO FILL OUT}
    [..]
    {USER TO FILL OUT USAGE EXAMPLES

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy;
  * The MIT License (MIT) 
  * Copyright (c) 2024, The Embedded Dude, (https://github.com/TheEmbeddedDude)
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * 
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef COMPONENTS_MODULE_ESP_NOW_H_
#define COMPONENTS_MODULE_ESP_NOW_H_


/* Includes ------------------------------------------------------------------*/
#include "mod_th_meas.h"


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

esp_err_t mod_espnow_init( size_t u32_MaxBufferSize);
void mod_espnow_deinit( void );
esp_err_t mod_espnow_add_send_data( void *p_Data, size_t u32_Len );
esp_err_t mod_espnow_send_data( void );



#endif /* COMPONENTS_MODULE_ESP_NOW_H_ */
