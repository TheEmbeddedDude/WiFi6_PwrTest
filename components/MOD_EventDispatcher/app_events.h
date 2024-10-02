 /**
  ******************************************************************************
  * @file    app_events.h
  * @author  The Embedded Dude
  * @brief   System event definitions are stored in this file.   
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
#ifndef COMPONENTS_APP_EVENTS_H_
#define COMPONENTS_APP_EVENTS_H_


/* Includes ------------------------------------------------------------------*/
#include "esp_event.h"



/* Exported types ------------------------------------------------------------*/
ESP_EVENT_DECLARE_BASE(MOD_WIFI_EVENTS);
ESP_EVENT_DECLARE_BASE(MOD_BACKEND_EVENTS);
ESP_EVENT_DECLARE_BASE(MOD_POWER_EVENTS);
ESP_EVENT_DECLARE_BASE(MOD_ESPNOW_EVENTS);


typedef enum mod_wifi_events
{
    WIFI_CONNECTED_EVENT,                //!< Will be published once we got an IP address                            
    WIFI_DISCONNECTED_EVENT,             //!< Wi-Fi connection got lost or disconnected on purpose                   
    WIFI_CONNECT_FAILED_EVENT,           //!< Wi-Fi connection could not be established even after retries           
    WIFI_ITWT_ESTABLISHED,               //!< We got an iTWT agreement in place with AP                              
    WIFI_ITWT_CLOSED                     //!< iTWT not active anymore. Will be raised if iTWT was established before.

}MOD_WIFI_EVENTS_ENUM_t;


typedef enum mod_backend_events
{
    BACKEND_CONNECTED_EVENT,             //!< Will be published once we are connected to backend                     
    BACKEND_DISCONNECTED_EVENT,          //!< Backend connection disconnected (lost) not on purpose                  
    BACKEND_CONNECT_FAILED_EVENT,        //!< Backend connection failed                                              
    BACKEND_SEND_MESSAGE,                //!< Send message to backend                                                
    BACKEND_SEND_MESSAGE_DONE,           //!< Message delivered                                                      
    BACKEND_MESSAGE_RECEIVED,            //!< Message from backend received                                          
    
}MOD_BACKEND_EVENTS_ENUM_t;


typedef enum mod_power_events
{    
    PWR_GO_TO_SLEEP                      //!< Will be published to all modules to go to sleep                                
    
}MOD_POWER_EVENTS_ENUM_t;


typedef enum mod_espnow_events
{
    ESPNOW_DATA_SENT,                    //!< Will be published after ESP-Now message has been sent.   Note: Its not a confirmation that this message also got received.                          
    ESPNOW_DATA_SENT_FAILED              //!< Will be published after ESP-Now message sent has failed. 

}MOD_ESPNOW_EVENTS_ENUM_t;


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
const char *app_wifi_event_to_str(MOD_WIFI_EVENTS_ENUM_t wfif_event);
const char *app_backend_event_to_str(MOD_BACKEND_EVENTS_ENUM_t backend_event);
const char *app_power_event_to_str(MOD_POWER_EVENTS_ENUM_t power_event);
const char *app_espnow_event_to_str(MOD_ESPNOW_EVENTS_ENUM_t espnow_event);


#endif /* COMPONENTS_APP_EVENTS_H_ */
