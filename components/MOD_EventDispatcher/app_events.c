 /**
  ******************************************************************************
  * @file    app_events.c
  * @author  The Embedded Dude
  * @brief   System event definitions are stored in this file. 
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

/* Includes ------------------------------------------------------------------*/
#include "app_events.h"
#include "esp_event_base.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
ESP_EVENT_DEFINE_BASE(MOD_WIFI_EVENTS);
ESP_EVENT_DEFINE_BASE(MOD_BACKEND_EVENTS);
ESP_EVENT_DEFINE_BASE(MOD_POWER_EVENTS);
ESP_EVENT_DEFINE_BASE(MOD_ESPNOW_EVENTS);


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
/// @brief             "Translates" MOD_WIFI_EVENTS_ENUM_t event into string
/// @param WiFi_event  MOD_WIFI_EVENTS_ENUM_t event
/// @return            translated string
const char *app_wifi_event_to_str(MOD_WIFI_EVENTS_ENUM_t WiFi_event)
{
    switch (WiFi_event) 
    {
        case WIFI_CONNECTED_EVENT:        return "WiFi Connection established";
        case WIFI_DISCONNECTED_EVENT:     return "WiFi Connection closed";
        case WIFI_CONNECT_FAILED_EVENT:   return "WiFi Connection failed";
        case WIFI_ITWT_ESTABLISHED:       return "iTWT established";
        case WIFI_ITWT_CLOSED:            return "iTWT closed";
        default:                          return "UNKNOWN MOD_WIFI_EVENT";
    }
}


/// @brief               "Translates" MOD_BACKEND_EVENTS_ENUM_t event into string
/// @param backend_event MOD_BACKEND_EVENTS_ENUM_t event
/// @return              translated string
const char *app_backend_event_to_str(MOD_BACKEND_EVENTS_ENUM_t backend_event)
{
    switch (backend_event) 
    {
        case BACKEND_CONNECTED_EVENT:      return "Backend Connection established";  
        case BACKEND_DISCONNECTED_EVENT:   return "Backend Connection closed";  
        case BACKEND_CONNECT_FAILED_EVENT: return "Backend Connection failed";
        case BACKEND_SEND_MESSAGE:         return "Sent Message to Backend";        
        case BACKEND_SEND_MESSAGE_DONE:    return "Ack that Message received by backend";         
        case BACKEND_MESSAGE_RECEIVED:     return "Message from Backend received";
        default:                           return "UNKNOWN MOD_BACKEND_EVENT";
    }
}


/// @brief               "Translates" MOD_POWER_EVENTS_ENUM_t event into string
/// @param backend_event MOD_POWER_EVENTS_ENUM_t event
/// @return              translated string
const char *app_power_event_to_str(MOD_POWER_EVENTS_ENUM_t power_event)
{
    switch (power_event) 
    {        
        case PWR_GO_TO_SLEEP:    return "Going to sleep.";
        default:                 return "UNKNOWN MOD_POWER_EVENT";                                        
    }
}


/// @brief               "Translates" MOD_ESPNOW_EVENTS_ENUM_t event into string
/// @param backend_event MOD_ESPNOW_EVENTS_ENUM_t event
/// @return              translated string 
const char *app_espnow_event_to_str(MOD_ESPNOW_EVENTS_ENUM_t espnow_event)
{
    switch (espnow_event) 
    {
        case ESPNOW_DATA_SENT:        return "ESP-NOW data sent successfully.";
        case ESPNOW_DATA_SENT_FAILED: return "ESP-NOW data failed to send";
        default:                      return "UNKNOWN MOD_ESPNOW_EVENT";
    }
}






/*****************************END OF FILE**************************************/

