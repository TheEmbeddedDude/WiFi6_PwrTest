 /**
  ******************************************************************************
  * @file    mod_th_meas.h
  * @author  The Embedded Dude
  * @brief   Temperature and humidity measurement module
  *          Handles reading of data from I2C sensor(s) and data conversion  
  * @date    Git controlled
  * @version Git controlled

  @verbatim
  ==============================================================================
                     ##### How to use this module #####
  ==============================================================================    
    1. Before calling any function the module must be intialized using 
       mod_th_meas_init(..)
       Note: I2C drivers and power are not intialized by this module. It is 
       required to do that prior to using this module.
    2. Temperature and humidity can be read via mod_th_meas_GetValues(..)
    3. De-Init the module calling mod_th_meas_Deinit(..)
    4. In case an init of the sensor but not the driver is needed use 
       mod_th_meas_reinit(..)  

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
#ifndef COMPONENTS_MODULE_TEMP_HUM_MEAS_H_
#define COMPONENTS_MODULE_TEMP_HUM_MEAS_H_


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "i2cdev.h"
#include "sht4x.h"
#include "mod_pwr.h"


/* Exported types ------------------------------------------------------------*/
typedef struct TEMP_HUMID_VALUES_t
{
    float f_Temp_C;      //!< Measured temperature in degree celsius
    float f_Humi_PCT;    //!< Measure humidity in %

}TEMP_HUMID_VALUES_t;


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

esp_err_t mod_th_meas_init(void);
esp_err_t mod_th_meas_reinit(void);
esp_err_t mod_th_meas_Deinit(void);
esp_err_t mod_th_meas_GetValues(TEMP_HUMID_VALUES_t *p_TH_Values);
int32_t s32_ConvertTemp_f_to_str( char* str_Data, size_t DataLenMax, TEMP_HUMID_VALUES_t *p_TH_Values );
int32_t s32_ConvertHumi_f_to_str( char* str_Data, size_t DataLenMax, TEMP_HUMID_VALUES_t *p_TH_Values );


/* Initialization and de-initialization functions *****************************/


/* IO operation functions *****************************************************/


/* Private types -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



#endif /* COMPONENTS_MODULE_TEMP_HUM_MEAS_H_ */
