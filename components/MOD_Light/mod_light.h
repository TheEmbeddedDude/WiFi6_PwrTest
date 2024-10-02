 /**
  ******************************************************************************
  * @file    mod_light.h
  * @author  The Embedded Dude
  * @brief   Ambient Light measurement module
  *          Handles reading of data from I2C sensor(s) and data conversion  
  * @date    Git controlled
  * @version Git controlled

  @verbatim
  ==============================================================================
                     ##### How to use this module #####
  ==============================================================================    
    1. Before calling any function the module must be intialized using 
       mod_light_init(..)
       Note: I2C drivers and power are not intialized by this module. It is 
       required to do that prior to using this module.
    2. The light value in lux can be read via mod_light_Get(..)
    3. De-Init the module calling mod_light_Deinit(..)

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
#ifndef COMPONENTS_MODULE_LIGHT_MEAS_H_
#define COMPONENTS_MODULE_LIGHT_MEAS_H_


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "i2cdev.h"
#include "mod_pwr.h"


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
esp_err_t mod_light_init(void);
esp_err_t mod_light_Deinit(void);
esp_err_t mod_light_Get(float *pf_Lux);
int32_t s32_ConvertLux_f_to_str( char* str_Data, size_t DataLenMax, float *pf_Lux );


/* Initialization and de-initialization functions *****************************/


/* IO operation functions *****************************************************/


/* Private types -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



#endif /* COMPONENTS_MODULE_LIGHT_MEAS_H_ */
