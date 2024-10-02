/**
  ******************************************************************************
  * @file    mod_backend.h
  * @author  The Embedded Dude
  * @brief   Handles connection to backend / MQTT broker
  * @date    Git controlled
  * @version Git controlled

 @verbatim
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
#ifndef COMPONENTS_MODULE_BACKEND_H_
#define COMPONENTS_MODULE_BACKEND_H_


/* Includes ------------------------------------------------------------------*/
#include "sdkconfig.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "mod_eventDispatcher.h"
#include "app_events.h"


/* Exported types ------------------------------------------------------------*/
typedef enum
{     
    AmbientTempC,                /* Ambient temperature in Celsius */
    Humidity,                    /* Humidity in ???                */
    Light,                       /* Ambient Light value in ??      */   

}BACKEND_TOPICS_ENUM_t;

typedef struct BACKEND_MESSAGE_t
{
    BACKEND_TOPICS_ENUM_t topic;
    char str_Data[10];  	         /*The maximum size depends on the max data length across all topics.*/  
    int s32_Msg_ID;

}BACKEND_MESSAGE_t;


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
esp_err_t Backend_Init(void);
esp_err_t Backend_DeInit(void);
esp_err_t Backend_Connect(void);
void Backend_Disconnect(void);
void Backend_SendMessage(BACKEND_MESSAGE_t* Message);


/* Initialization and de-initialization functions *****************************/


/* IO operation functions *****************************************************/


/* Private types -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



#endif /* COMPONENTS_MODULE_BACKEND_H_ */

