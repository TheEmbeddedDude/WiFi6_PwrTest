/**
  ******************************************************************************
  * @file    mod_eventDispatcher.h
  * @author  The Embedded Dude
  * @brief   Event dispatching module
  *          Handles events in the system using the ESP event library (simple wrapper)
  * @date    GIT controlled
  * @version GIT controlled

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
       and MOD_EVENT_DISP_STACK_SIZE in mod_eventDispatcher.h

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
#include "mod_eventDispatcher.h"



/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
esp_event_loop_handle_t general_app_event_loop; /*This loop is used for general communication for the components of the app*/


/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

/// @brief  Start the event dispatcher task with queue and stack size 
///         according to MOD_EVENT_DISP_QUEUE_SIZE and MOD_EVENT_DISP_STACK_SIZE
/// @param  void
void EventDispatcher_Start( void )
{
    esp_event_loop_args_t loop_with_task_args = 
    {    
        .queue_size      = MOD_EVENT_DISP_QUEUE_SIZE,
        .task_name       = "app_event_dispatcher",
        .task_priority   = uxTaskPriorityGet(NULL),
        .task_stack_size = MOD_EVENT_DISP_STACK_SIZE,
        .task_core_id    = tskNO_AFFINITY
    };

    ESP_ERROR_CHECK(esp_event_loop_create(&loop_with_task_args, &general_app_event_loop));
}


/// @brief                   Register a new event handler and subscribe to an even base and event.
/// @param event_base        Pre-defined bases are in app_events.h
/// @param s32_EventID       Pre-defined event IDs are in app_events.h
/// @param event_handler     Event handler that gets called once event has been published
/// @param event_handler_arg Optional event arg/data. If not needed set to NULL
void EventDispatcher_RegisterEventHandler(esp_event_base_t event_base, int32_t s32_EventID, esp_event_handler_t event_handler, void* event_handler_arg)
{    
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(general_app_event_loop, event_base, s32_EventID, event_handler, event_handler_arg, NULL));
}


/// @brief                 Post an event for a specfic event base including data (optional)
/// @param event_base      Pre-defined bases are in app_events.h
/// @param s32_EventID     Pre-defined event IDs are in app_events.h
/// @param event_data      A pointer to the event data. Example (const void*)(&MyIntVariable)
/// @param event_data_size Size of the event data
/// @param ticks_to_wait   Number of ticks to wait (blocking calling task) in case the event loop is already full
/// @note A deep copy of the event data will be created on the heap. 
void EventDispatcher_PostEvent(esp_event_base_t event_base, int32_t s32_EventID, const void* event_data, size_t event_data_size, TickType_t ticks_to_wait)
{
    ESP_ERROR_CHECK(esp_event_post_to(general_app_event_loop, event_base, s32_EventID, event_data, event_data_size, ticks_to_wait));
}
/*****************************END OF FILE**************************************/

