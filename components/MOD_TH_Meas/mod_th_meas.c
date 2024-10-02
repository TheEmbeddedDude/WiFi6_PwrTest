 /**
  ******************************************************************************
  * @file    mod_th_meas.c
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

/* Includes ------------------------------------------------------------------*/
#include "mod_th_meas.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define I2C_MASTER_SDA_PIN  CONFIG_APP_I2C_MASTER_SDA_PIN
#define I2C_MASTER_SCL_PIN  CONFIG_APP_I2C_MASTER_SCL_PIN


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
static sht4x_t dev;


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

/// @brief  Initializes the module and sensor
/// @param  void
/// @return ESP_OK on success 
/// @note   This function must be called before any other function of this module is called.
/// @note   Its required that i2cdev_init() is called prior to this function is called.
/// @note   The sensor must be powered before calling this function.The function will block the calling task by 1-2ms
esp_err_t mod_th_meas_init(void)
{    
    esp_err_t ret = ESP_OK;
    
    //Waiting for max 1ms for sensor to power up. Could be handled via timer and callback if too long
    vTaskDelay(pdMS_TO_TICKS(1));

    //Clear sht4x object with 0 values
    memset(&dev, 0, sizeof(sht4x_t));

    ret = sht4x_init_desc(&dev, 0, I2C_MASTER_SDA_PIN, I2C_MASTER_SCL_PIN);
        
    if(ret == ESP_OK)
      ret = sht4x_init(&dev);    

    return ret;
}


/// @brief  Re-Initializes only the sensor after it has been turned off and on again
/// @param  void
/// @return ESP_OK on success 
esp_err_t mod_th_meas_reinit(void)
{ 
    return(sht4x_init(&dev));
}


/// @brief  Free the sensor device descriptor again. You need to call mod_th_meas_init(..)
///         again.
/// @param  void
/// @return ESP_OK on success
esp_err_t mod_th_meas_Deinit(void)
{
    return( sht4x_free_desc(&dev));
}


/// @brief             Reads the temperature in Celsius and the humidity in % from sensor
/// @param p_TH_Values Out paramter will be written with new temp and humid values
/// @return            ESP_OK on sccuess
esp_err_t mod_th_meas_GetValues(TEMP_HUMID_VALUES_t *p_TH_Values)
{    
    return(sht4x_measure(&dev, &p_TH_Values->f_Temp_C, &p_TH_Values->f_Humi_PCT));
}


/// @brief             Returns a float temperature in celsius as string
/// @param str_Data    Writes temp as string into str_Data
/// @param DataLenMax  Max buffer length of str_Data
/// @param p_TH_Values Pointer to measured values as float
/// @return            Length of the string written to str_Data. -1 if str_Data = NULL
int32_t s32_ConvertTemp_f_to_str( char* str_Data, size_t DataLenMax, TEMP_HUMID_VALUES_t *p_TH_Values )
{   
    if(str_Data == NULL)
        return -1;
    
    //Convert to string and write to str_Data
    return( snprintf(str_Data, DataLenMax, "%.2f", p_TH_Values->f_Temp_C) );
}


/// @brief             Returns a float humidity in % as string
/// @param str_Data    Writes humidity as string into str_Data
/// @param DataLenMax  Max buffer length of str_Data
/// @param p_TH_Values Pointer to measured values as float
/// @return            Length of the string written to str_Data. -1 if str_Data = NULL
int32_t s32_ConvertHumi_f_to_str( char* str_Data, size_t DataLenMax, TEMP_HUMID_VALUES_t *p_TH_Values )
{    
    if(str_Data == NULL)
        return -1;
    
    //Convert to string and write to str_Data
    return( snprintf(str_Data, DataLenMax, "%0.2f", p_TH_Values->f_Humi_PCT ));
}

/*****************************END OF FILE**************************************/

