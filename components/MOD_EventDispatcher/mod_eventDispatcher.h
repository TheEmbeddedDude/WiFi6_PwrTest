/**
  ******************************************************************************
  * @file    mod_eventDispatcher.h
  * @author  The Embedded Dude
  * @brief   Event dispatching module
  *          Handles events in the system using the ESP event library (simple wrapper)
  * @date    Git controlled
  * @version Git controlled

  @verbatim
  ==============================================================================
                     ##### How to use this module #####
  ==============================================================================    
    1. Before calling any function the module must be started using
       EventDispatcher_Start(..)
    2. Once the module is running clients can register event handler and subscribe
       to events. Events are defined in app_events.h
    3. Clients can post events using EventDispatcher_PostEvent(..)
    4. Queue and stack sizes can be changed by modifying MOD_EVENT_DISP_QUEUE_SIZE 
       and MOD_EVENT_DISP_STACK_SIZE

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
#ifndef COMPONENTS_MOD_EVENT_DISPATCHER_H_
#define COMPONENTS_MOD_EVENT_DISPATCHER_H_


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_event_base.h"



#ifdef __cplusplus
extern "C" {
#endif


/* Exported types ------------------------------------------------------------*/


/* Exported defines ----------------------------------------------------------*/
#define MOD_EVENT_DISP_QUEUE_SIZE  5 
#define MOD_EVENT_DISP_STACK_SIZE  3072


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/
void EventDispatcher_Start( void );
void EventDispatcher_RegisterEventHandler(esp_event_base_t event_base, int32_t s32_EventID, esp_event_handler_t event_handler, void* event_handler_arg);
void EventDispatcher_PostEvent(esp_event_base_t event_base, int32_t s32_EventID, const void* event_data, size_t event_data_size, TickType_t ticks_to_wait);


/* Initialization and de-initialization functions *****************************/


/* IO operation functions *****************************************************/


/* Private types -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_MOD_EVENT_DISPATCHER_H_ */
