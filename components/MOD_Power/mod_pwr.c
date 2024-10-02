/**
  ******************************************************************************
  * @file    mod_pwr.c
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

/* Includes ------------------------------------------------------------------*/
#include "esp_err.h"
#include "esp_pm.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "mod_pwr.h"
#include "mod_eventDispatcher.h"
#include "mod_wifi.h"
#include "app_events.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/

#ifdef CONFIG_APP_AUTO_LIGHT_SLEEP
// Reporting frequency - how often readings are taken and published. Should be the same as TWT
/* TWT Wake Interval_µSec = TWT Wake Interval Mantissa * (2 ^ TWT Wake Interval Exponent)*/
static uint32_t u32_SleepTimeSec = (uint32_t)(CONFIG_APP_ITWT_WAKE_INVL_MANT * (1 << CONFIG_APP_ITWT_WAKE_INVL_EXPN)/1000000);
#else
static uint32_t u32_SleepTimeSec = (uint32_t)(CONFIG_APP_REPORTING_INTERVAL_SEC);
#endif

#define GPIO_PERIPH_PWR       CONFIG_APP_PERIPH_PWR_PIN
#define GPIO_OUTPUT_PIN_SEL   (1ULL<<GPIO_PERIPH_PWR)


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/
const char *TAG_PWR = "mod_pwr";


/* Private variables ---------------------------------------------------------*/
esp_pm_config_t power_management_disabled;
esp_pm_config_t power_management_enabled;


/* Private function prototypes -----------------------------------------------*/
static void mod_pwr_wifi_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data);
static void mod_pwr_GoToSleep(uint32_t u32_SleepPeriodSec);
static void mod_pwr_Init_IOs(void);


/* Exported functions --------------------------------------------------------*/

/// @brief  Init all IOs and store PowerManagement profilesif PM is enabled in SDK config
/// @param  void
/// @note   Subscribes to WIFI_ITWT_ESTABLISHED event. Needed if iTWT is used.
void mod_pwr_init(void)
{      
    // get the current power management configuration and save it as a baseline for when power save mode is disabled
    ESP_ERROR_CHECK(esp_pm_get_configuration(&power_management_disabled));

#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = 
    {
        .max_freq_mhz = CONFIG_APP_MAX_CPU_FREQ_MHZ,
        .min_freq_mhz = CONFIG_APP_MIN_CPU_FREQ_MHZ,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = true
#endif
    };
    power_management_enabled = pm_config;   
#endif

    EventDispatcher_RegisterEventHandler(MOD_WIFI_EVENTS, WIFI_ITWT_ESTABLISHED, mod_pwr_wifi_events_handler, NULL);

    mod_pwr_Init_IOs( );
}


/// @brief  Start power save mode. If AutoLightSleep is configured via SDK config
///         iTWT session will be requested. 
///         If deep sleep is configured deep sleep will be initiated
/// @param  void
/// @note   If deep sleep is configured the WiFi connection will be closed
void mod_pwr_save_start(void)
{
#ifdef CONFIG_APP_AUTO_LIGHT_SLEEP   
    mod_wifi_init_iTWT();             
#endif

#ifdef CONFIG_APP_DEEP_SLEEP
    ESP_ERROR_CHECK_WITHOUT_ABORT(mod_wifi_disconnect(true)); 
    mod_pwr_GoToSleep(u32_SleepTimeSec);
#endif

#ifdef CONFIG_APP_DEEP_SLEEP_ESP_NOW
    //ToDo: We need a clean WiFi disconnect and connect for ESP-NOW
    mod_pwr_GoToSleep(u32_SleepTimeSec);
#endif

#ifdef CONFIG_APP_LIGHT_SLEEP_ESP_NOW    
    ESP_ERROR_CHECK(esp_pm_configure(&power_management_enabled));     
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(u32_SleepTimeSec*1000000));
    ESP_LOGI(TAG_PWR, "Enter Light sleep start with timer wakeup source.");
    //Delay entering deep sleep otherwise the above statement wont be written.
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_light_sleep_start( ));    
#endif
}


/// @brief  Stop power save mode. If AutoLightSleep is configured via SDK config
///         Power management will be disabled when calling this function
/// @param  void
void mod_pwr_save_stop(void)
{
#ifdef CONFIG_APP_AUTO_LIGHT_SLEEP    
    //ToDo: Suspending might be the better option to save time and energy. To be tested!
    mod_wifi_stop_iTWT();
    ESP_ERROR_CHECK(esp_pm_configure(&power_management_disabled));
#endif

#ifdef CONFIG_APP_LIGHT_SLEEP_ESP_NOW  
    ESP_ERROR_CHECK(esp_pm_configure(&power_management_disabled));
#endif
}


/// @brief         Turns the peripherals power on or off
/// @param b_OnOff true=Power on, false=Power off
/// @return        ESP_OK Success - ESP_ERR_INVALID_ARG GPIO number error
esp_err_t mod_pwr_PeriphPWR(bool b_OnOff)
{    
    if(b_OnOff == true)
        return(gpio_set_level(GPIO_PERIPH_PWR, 1));
    else
        return(gpio_set_level(GPIO_PERIPH_PWR, 0));
}


/* Private functions ---------------------------------------------------------*/

/// @brief              WiFi events handler
/// @param handler_args NULL - Not used
/// @param base         Event base: check app_events.h for more details
/// @param s32_EventID  Event ID:   check app_events.h for more details
/// @param event_data   Event data provided for event. Depends on event type
static void mod_pwr_wifi_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data)
{    
    ESP_LOGI(TAG_PWR, "%s", app_wifi_event_to_str(s32_EventID));

    if(s32_EventID == WIFI_ITWT_ESTABLISHED)
    {         
        ESP_LOGI(TAG_PWR, "Sleep time: %lu seconds\n", u32_SleepTimeSec);
    
        //We need to send an event that all app tasks need to sleep for x seconds. 
        //This is because we are in another context here (Event handler context) and not main_app context!
        EventDispatcher_PostEvent(MOD_POWER_EVENTS, PWR_GO_TO_SLEEP, (const void*)(&u32_SleepTimeSec), sizeof(u32_SleepTimeSec), portMAX_DELAY);
    
        /*iTWT is now active and we can try to go into AutoLightSleep mode.*/
        ESP_ERROR_CHECK(esp_pm_configure(&power_management_enabled));     
    }  
}


/// @brief                    Puts device into deep sleep unless u32_SleepPeriodSec is 0
/// @param u32_SleepPeriodSec Sleep periode in seconds
/// @note                     For debug reasons entering deep sleep is delayed by 50ms block the calling task 
/// @note                     Remove debug output and delay if not needed
static void mod_pwr_GoToSleep(uint32_t u32_SleepPeriodSec)
{
    if( u32_SleepPeriodSec > 0 )
    {
        ESP_LOGI(TAG_PWR, "Sleep period set to %lusec. Will go to deep sleep now!", u32_SleepPeriodSec);
        
        //Delay entering deep sleep otherwise the above statement wont be written.
        vTaskDelay(pdMS_TO_TICKS(50)); 
        
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup((uint64_t)(u32_SleepPeriodSec*1000000)));
        esp_deep_sleep_start();
    }
    else
        ESP_LOGI(TAG_PWR, "Sleep period set to 0sec. Will not go to sleep!");
}


/// @brief  Init IOs 
/// @param  void
static void mod_pwr_Init_IOs(void)
{    
    gpio_config_t io_conf = {};    
    io_conf.intr_type = GPIO_INTR_DISABLE;    
    io_conf.mode = GPIO_MODE_OUTPUT;    
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;    
    io_conf.pull_down_en = 0;    
    io_conf.pull_up_en = 0;    
    gpio_config(&io_conf);
}



/*****************************END OF FILE**************************************/

