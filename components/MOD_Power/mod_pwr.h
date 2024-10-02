/**
  ******************************************************************************
  * @file    mod_pwr.h
  * @author  The Embedded Dude
  * @brief   Power management module. 
  *          Handle power management functions for different modes such as
  *          deep sleep, auto light sleep. 
  *          Future functionality: RTC deep sleep and self-hold functionality
  * @date    Git controlled
  * @version Git controlled

  @verbatim
  ==============================================================================
                     ##### How to use this module #####
  ==============================================================================    
    1. Before calling any function the module must be intialized using 
       mod_pwr_init(..)
    2. Sensor/peripheral power must be turned on via mod_pwr_PeriphPWR(..)
       To power up the sensors and read the data from them
    3. To reduce the power consumption during sleep turn off the sensors/peripherals
       Note: An external hardware circuitry (power switch) is needed for this. 
    4. Enter sleep mode (power save mode) by calling mod_pwr_save_start(..)
       Depending on the settings in SDK config either AutoLightSleep or DeepSleep
       will be entered. 
    5. If AutoLightSleep is used call mod_pwr_save_stop(..) to stop power saving.
       This is relevant if for example I2C is used and no Power Management Locks 
       are used. Withour power locks it can cause issues.   

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
#ifndef COMPONENTS_MODULE_POWER_H_
#define COMPONENTS_MODULE_POWER_H_


/* Includes ------------------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

void mod_pwr_init(void);
void mod_pwr_save_start(void);
void mod_pwr_save_stop(void);
esp_err_t mod_pwr_PeriphPWR(bool b_OnOff);


/* Initialization and de-initialization functions *****************************/


/* IO operation functions *****************************************************/


/* Private types -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



#endif /* COMPONENTS_MODULE_POWER_H_ */
