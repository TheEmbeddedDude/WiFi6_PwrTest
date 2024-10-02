 /**
  ******************************************************************************
  * @file    mod_light.c
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

/* Includes ------------------------------------------------------------------*/
#include "mod_light.h"
#include "tsl2591.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define I2C_MASTER_SDA_PIN  CONFIG_APP_I2C_MASTER_SDA_PIN
#define I2C_MASTER_SCL_PIN  CONFIG_APP_I2C_MASTER_SCL_PIN


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
static tsl2591_t dev = { 0 };


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

/// @brief  Initializes the module and sensor
/// @param  void
/// @return ESP_OK on success 
/// @note   This function must be called before any other function of this module is called.
/// @note   Its required that i2cdev_init() is called prior to this function is called.
/// @note   The sensor must be powered before calling this function.
esp_err_t mod_light_init(void)
{    
    esp_err_t ret = ESP_OK;

    ESP_ERROR_CHECK(tsl2591_init_desc(&dev, 0, CONFIG_APP_I2C_MASTER_SDA_PIN, CONFIG_APP_I2C_MASTER_SCL_PIN));
    ESP_ERROR_CHECK(tsl2591_init(&dev));

    // Turn TSL2591 on
    ESP_ERROR_CHECK(tsl2591_set_power_status(&dev, TSL2591_POWER_ON));
    // Turn ALS on
    ESP_ERROR_CHECK(tsl2591_set_als_status(&dev, TSL2591_ALS_ON));
    // Set gain
    ESP_ERROR_CHECK(tsl2591_set_gain(&dev, TSL2591_GAIN_MEDIUM));
    // Set integration time = 300ms
    ESP_ERROR_CHECK(tsl2591_set_integration_time(&dev, TSL2591_INTEGRATION_300MS));

    return ret;
}


/// @brief  Free the sensor device descriptor again
/// @param  void
/// @return ESP_OK on success
esp_err_t mod_light_Deinit(void)
{
    return( tsl2591_free_desc(&dev));
}


/// @brief             Reads the temperature in Celsius and the humidity in % from sensor
/// @param p_TH_Values Out parameter that will be updated with new light value 
/// @return            ESP_OK on sccuess
esp_err_t mod_light_Get(float *pf_Lux)
{  
    return( tsl2591_get_lux(&dev, pf_Lux));
}


/// @brief             Returns a float lux as string
/// @param str_Data    Writes humidity as string into str_Data
/// @param DataLenMax  Max buffer length of str_Data
/// @param p_TH_Values Pointer to measured values as float
/// @return            Length of the string written to str_Data. -1 if str_Data = NULL
int32_t s32_ConvertLux_f_to_str( char* str_Data, size_t DataLenMax, float *pf_Lux )
{    
    if(str_Data == NULL)
        return -1;
    
    //Convert to string and write to str_Data
    return( snprintf(str_Data, DataLenMax, "%0.2f", *pf_Lux ));
}


/* Private functions ---------------------------------------------------------*/

/// @brief  This function powers the sensor on or off
/// @param  b_OnOff 
/// @return ESP_OK on success
/// @note   The sensor requires maximal power-up time of 1 ms before it can be read after powering it on
/// ToDo:   Remove due to PWR handled by global function?!?
static esp_err_t mod_light_Enable(bool b_OnOff)
{ 
    return(mod_pwr_PeriphPWR(b_OnOff));
}
/*****************************END OF FILE**************************************/

