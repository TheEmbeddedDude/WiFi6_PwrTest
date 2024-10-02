/**
  ******************************************************************************
  * @file    main.c
  * @author  The Embedded Dude
  * @brief   WiFi6 power saving test  
  * @date    Git controlled
  * @version Git controlled  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy;
  * The MIT License (MIT) 
  * Copyright (c) 2024, The Embedded Dude, ToDo: Add github link
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
#include <stdio.h>
#include <stdint.h>
#include "esp_pm.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "mod_wifi.h"
#include "mod_backend.h"
#include "mod_eventDispatcher.h"
#include "mod_pwr.h"
#include "mod_th_meas.h"
#include "mod_light.h"
#include "mod_esp_now.h"
#include "i2cdev.h"
#include "esp_mac.h"


/* Private constants ---------------------------------------------------------*/
const char *TAG_APP = "MAIN_APP";


/* Private typedef -----------------------------------------------------------*/
/// @brief Main App State machine states (MAS)
typedef enum 
{
    MAS_Init_Sys          = 0,
    MAS_Not_Connected     = 1,          //!< System is initialized but Wi-Fi is not connected. Try to connect to AP.
    MAS_WiFi_Connected    = 2,          //!< Wi-Fi connected. IP addr received. Try to connect to backend.          
    MAS_Backend_Connected = 3,          //!< Backend connected. Try to publish data.                                
    MAS_Data_Published    = 4,          //!< All data has been sent to backend. Prepare for sleep                   
    MAS_Sleep             = 5,          //!< Go to sleep depending configuration. Handle wake from auto light sleep.
    MAS_Error             = 6           //!< We could not recover from a situation.                                 

} MainApp_State;

/// @brief Main App state machine events (MAE)
typedef enum 
{   
    MAE_Sys_Init_Done,                  //!< System init after boot done               
    MAE_Sys_Init_Failed,                //!< System init failed                        
    MAE_Sensor_Read_Failed,             //!< Sensor data could not be read
    MAE_WiFi_Failed,                    //!< WiFi connection could not be established      
    MAE_WiFi_Connection_Established,    //!< Wi-Fi connected. IP addr received         
    MAE_WiFi_Connection_Lost,           //!< Wi-Fi connection lost - not intended      
    MAE_Backend_Failed,                 //!< Connection or reporting to backend failed 
    MAE_Backend_Connection_Established, //!< Backend connection established            
    MAE_Backend_Connection_Lost,        //!< Backend connection lost - not intended    
    MAE_Data_Sent_To_Backend,           //!< All data has been sent to backend         
    MAE_Enter_Sleep_Mode                //!< Enter the sleep state                     

} MainApp_Event;

typedef struct MAIN_APP_t
{
    MainApp_State CurrentState;         //!< Hold the current state machine state.                            
          
    bool b_WaitingForWiFiCon;           //!< Used in MASH_Not_Connected(..) to avoid multilpe connect atempts  
    bool b_WaitingForBackendCon;        //!< Used in MASH_WiFi_Connected(..) to avoid multilpe connect atempts  
    bool b_WaitingForDataToBeSent;      //!< USed in MASH_Backend_Connected(..) to avoid race conditions with event handler
    bool b_WaitToGoToSleep;             //!< Used in MASH_Data_Published(..) to avoid multilpe shutdown atempts    
     
    int BackendMsgIDs[3];               //!< Holds the backend msg ids to check if all 3 msgs hae been sent   
    uint32_t u32_SleepTimeSec;          //!< Sleep time in seconds to achieve required reporting intervals    
       
    TEMP_HUMID_VALUES_t TH_Values;      //!< Holds the temp and humidity sensor readings to send to backend   
    float f_Light_Lux;                  //!< Holds the light sensor reading in lux which will be send to the backend   
    
}MAIN_APP_t;


typedef void (*fp_StateHandler)(MAIN_APP_t * obj);

/*Main App state handler - MASH*/
static void MASH_Init_Sys(MAIN_APP_t * obj);
static void MASH_Not_Connected(MAIN_APP_t * obj);
static void MASH_WiFi_Connected(MAIN_APP_t * obj);
static void MASH_Backend_Connected(MAIN_APP_t * obj);
static void MASH_Data_Published(MAIN_APP_t * obj);
static void MASH_Sleep(MAIN_APP_t * obj);
static void MASH_Error(MAIN_APP_t * obj);

/// @note Handler position in array must correspond to state value defined in MainApp_State enum
static fp_StateHandler MA_StateHandler[] = 
{
    &MASH_Init_Sys,
    &MASH_Not_Connected,
    &MASH_WiFi_Connected,
    &MASH_Backend_Connected,
    &MASH_Data_Published,
    &MASH_Sleep,
    &MASH_Error   
};

/* Private variables ---------------------------------------------------------*/
MAIN_APP_t MainApp_obj;

/* Private function prototypes -----------------------------------------------*/
static void Backend_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data);
static void PWR_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data);
static void WiFi_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data);
static void ESPNOW_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data);

static void Print_Reset_Reason(esp_reset_reason_t reason);
static MainApp_State MAS_Handle_Transition(MainApp_State currentState, MainApp_Event event);
static esp_err_t GetSensorMeasurements(TEMP_HUMID_VALUES_t *p_TH_Values, float *pf_Lux);
static esp_err_t Backend_PublishData(MAIN_APP_t * obj);
static void GoToSleep(uint32_t u32_SleepTimeSec);


/* Exported functions --------------------------------------------------------*/
void app_main(void)
{    
    fp_StateHandler CurrentHandler = NULL;
    MainApp_obj.CurrentState = MAS_Init_Sys;

    while(1)     
    {
        CurrentHandler = MA_StateHandler[MainApp_obj.CurrentState];
        CurrentHandler(&MainApp_obj);
    }
}


/* Private functions ---------------------------------------------------------*/

/* Event Handler -------------------------------------------------------------*/

/// @brief              Handles all backend events such as connected, disconnected, failures, etc.
/// @param handler_args MainApp_obj
/// @param base         Event base: check app_events.h for more details
/// @param s32_EventID  Event ID:   check app_events.h for more details
/// @param event_data   Event data provided for event. Depends on event type.
/// @note               The s32_EventID can be the same for different event bases. If the event handler has to handle different event bases
/// @note               the "esp_event_base_t base" must be checked as well. Better solution is to have one handler per event base!
static void Backend_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data)
{    
    MAIN_APP_t *obj = (MAIN_APP_t*)(handler_args);

    ESP_LOGI(TAG_APP, "%s", app_backend_event_to_str(s32_EventID));

    switch(s32_EventID)
    {
        case BACKEND_CONNECTED_EVENT:            
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Backend_Connection_Established);
        break;

        case BACKEND_DISCONNECTED_EVENT:            
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Backend_Connection_Lost);
        break;

        case BACKEND_CONNECT_FAILED_EVENT:            
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Backend_Failed);
        break;

        case BACKEND_SEND_MESSAGE_DONE:
        {
            int* ps32_MsgID = (int*)(event_data);        

            if(*ps32_MsgID == obj->BackendMsgIDs[0])
                obj->BackendMsgIDs[0] = 0;
            if(*ps32_MsgID == obj->BackendMsgIDs[1])
                obj->BackendMsgIDs[1] = 0;
            if(*ps32_MsgID == obj->BackendMsgIDs[2])
                obj->BackendMsgIDs[2] = 0;          
        }
        break;

        default:
        //ToDo: implement error handling
    }
}


/// @brief              Handles all power events. For now only the go_to_sleep event
/// @param handler_args MainApp_obj 
/// @param base         Event base: check app_events.h for more details
/// @param s32_EventID  Event ID:   check app_events.h for more details
/// @param event_data   Event data provided for event. Depends on event type.
static void PWR_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data)
{
    MAIN_APP_t *obj = (MAIN_APP_t*)(handler_args);

    ESP_LOGI(TAG_APP, "%s", app_power_event_to_str(s32_EventID));

    if(s32_EventID == PWR_GO_TO_SLEEP)
    {
        obj->u32_SleepTimeSec = *(uint32_t*)(event_data);
        ESP_LOGI(TAG_APP,"Sleep time:%lu", obj->u32_SleepTimeSec);
        
        obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Enter_Sleep_Mode);
    }
}


/// @brief              Handles all WiFi Events.
/// @param handler_args MainApp_obj 
/// @param base         Event base: check app_events.h for more details
/// @param s32_EventID  Event ID:   check app_events.h for more details
/// @param event_data   Event data provided for event. Depends on event type.
static void WiFi_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data)
{
    MAIN_APP_t *obj = (MAIN_APP_t*)(handler_args);

    ESP_LOGI(TAG_APP, "%s", app_wifi_event_to_str(s32_EventID));

    switch(s32_EventID)
    {
        case WIFI_CONNECTED_EVENT:
        {
            //ESP_LOGI(TAG_APP, "Wi-Fi connected %s:%ld", base, s32_EventID);                        
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_WiFi_Connection_Established);            
        }
        break;

        case WIFI_DISCONNECTED_EVENT:
        {            
            //ESP_LOGI(TAG_APP, "Wi-Fi disconnected %s:%ld", base, s32_EventID);            
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_WiFi_Connection_Lost);
        }
        break;

        case WIFI_CONNECT_FAILED_EVENT:
        {            
            //ESP_LOGI(TAG_APP, "Wi-Fi failed to connect %s:%ld", base, s32_EventID);                        
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_WiFi_Failed);      
        }
        break;

        default: //Dont do anything.
    }
}


/// @brief              Handles all ESPNOW Events.
/// @param handler_args MainApp_obj 
/// @param base         Event base: check app_events.h for more details
/// @param s32_EventID  Event ID:   check app_events.h for more details
/// @param event_data   Event data provided for event. Depends on event type.
static void ESPNOW_events_handler(void* handler_args, esp_event_base_t base, int32_t s32_EventID, void* event_data)
{
    MAIN_APP_t *obj = (MAIN_APP_t*)(handler_args);

   //ESP_LOGI(TAG_APP, "%s", app_espnow_event_to_str(s32_EventID));

    switch(s32_EventID)
    {
        case ESPNOW_DATA_SENT:           
        case ESPNOW_DATA_SENT_FAILED: /*If we could sent the data we just continue*/
        {
            obj->b_WaitingForDataToBeSent = false;            
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Data_Sent_To_Backend);     
        }
        
        default: //Dont do anything.
    }
}


/* MainAPP State (MAS) machine logic and state handler -----------------------*/

/// @brief              Main app state machine. Depending on the current state and event occurrence the 
/// @brief              logic switches to a new state or remains in the current state. 
/// @brief              Check the state machine diagram for more details
/// @param currentState The actual state we are in
/// @param event        Event that occured
/// @return             Returns the new state. If no action required it will return the current state
static MainApp_State MAS_Handle_Transition(MainApp_State currentState, MainApp_Event event) 
{    
    switch (currentState) 
    {
        case MAS_Init_Sys:
        {
            if( event == MAE_Sys_Init_Done )
                return MAS_Not_Connected; 

            if( event == MAE_Sys_Init_Failed )
                return MAS_Error; 
        }
        break;

        case MAS_Not_Connected:
        {
            if( event == MAE_WiFi_Connection_Established )
            {
#if defined(CONFIG_APP_DEEP_SLEEP_ESP_NOW) || defined(CONFIG_APP_LIGHT_SLEEP_ESP_NOW)
                return MAS_Backend_Connected; //ESP-NOW does not connect to a backend as such. Dirty workaround
#endif
                return MAS_WiFi_Connected; 
            }   

            if( event == MAE_WiFi_Failed )
                return MAS_Error;    

            if( event == MAE_Sensor_Read_Failed)
                return MAS_Error;    
        }
        break;

        case MAS_WiFi_Connected:
        {
            if( event == MAE_Backend_Connection_Established )
                return MAS_Backend_Connected;

            if( event == MAE_WiFi_Connection_Lost )
                return MAS_Not_Connected;         

            if( event == MAE_Backend_Failed )
                return MAS_Error;
        }
        break;

        case MAS_Backend_Connected:
        {            
            if( event == MAE_Data_Sent_To_Backend ) 
                return MAS_Data_Published;
            
            if( event == MAE_Backend_Connection_Lost )
                return MAS_WiFi_Connected;
            
            if( event == MAE_WiFi_Connection_Lost ) 
                return MAS_Not_Connected;   
            
            if( event == MAE_Backend_Failed || event == MAE_Sensor_Read_Failed)
                return MAS_Error;
        }
        break;

        case MAS_Data_Published:
        {
            if(event == MAE_Enter_Sleep_Mode )
                return MAS_Sleep;                      
        }
        break;

        case MAS_Sleep:
        {
            if(event == MAE_WiFi_Connection_Established)
            {
#ifdef CONFIG_APP_LIGHT_SLEEP_ESP_NOW                
                return MAS_Backend_Connected;
#endif
                return MAS_WiFi_Connected;
            }

            if( event == MAE_WiFi_Connection_Lost )
                return MAS_Not_Connected; 
        }
        break;

        case MAS_Error:          
        default:        
        //ToDo: Implement error handling
    }

    // If no transition occurs, stay in the current state
    return currentState;
}


/// @brief     Call this function after boot before calling any other function to ensure the system is initialized.
/// @param obj MainApp object
static void MASH_Init_Sys(MAIN_APP_t * obj)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Init_Sys");
    
    obj->b_WaitingForWiFiCon      = false;  
    obj->b_WaitingForBackendCon   = false;    
    obj->b_WaitingForDataToBeSent = false;
    obj->b_WaitToGoToSleep        = false;    
    obj->u32_SleepTimeSec         = 0;
    obj->BackendMsgIDs[0]         = -1;
    obj->BackendMsgIDs[1]         = -1;
    obj->BackendMsgIDs[2]         = -1;
    obj->TH_Values.f_Humi_PCT     = 0.0;
    obj->TH_Values.f_Temp_C       = 0.0;
    obj->f_Light_Lux              = 0.0;
    obj->CurrentState             = MAS_Init_Sys;

    Print_Reset_Reason(esp_reset_reason());
    
    ret = nvs_flash_init();    
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
        
    EventDispatcher_Start( );
    EventDispatcher_RegisterEventHandler(MOD_BACKEND_EVENTS, BACKEND_CONNECTED_EVENT,   Backend_events_handler,  (void*)(obj));
    EventDispatcher_RegisterEventHandler(MOD_BACKEND_EVENTS, BACKEND_SEND_MESSAGE_DONE, Backend_events_handler,  (void*)(obj));
    EventDispatcher_RegisterEventHandler(MOD_POWER_EVENTS,   PWR_GO_TO_SLEEP,           PWR_events_handler,      (void*)(obj));        
    EventDispatcher_RegisterEventHandler(MOD_WIFI_EVENTS,    WIFI_CONNECTED_EVENT,      WiFi_events_handler,     (void*)(obj));
    EventDispatcher_RegisterEventHandler(MOD_ESPNOW_EVENTS,  ESPNOW_DATA_SENT,          ESPNOW_events_handler,   (void*)(obj));
    EventDispatcher_RegisterEventHandler(MOD_ESPNOW_EVENTS,  ESPNOW_DATA_SENT_FAILED,   ESPNOW_events_handler,   (void*)(obj));
      
    //Power module should be initialized before other modules except for EventDispatcher
    //Also inits all Power related IOs
    mod_pwr_init();  

    uint8_t u8_Mac[6];
    char s_Mac[18];

    ESP_ERROR_CHECK(esp_read_mac(u8_Mac, ESP_MAC_BASE));
    snprintf(s_Mac, 18, "%02X:%02X:%02X:%02X:%02X:%02X", u8_Mac[0], u8_Mac[1], u8_Mac[2], u8_Mac[3], u8_Mac[4], u8_Mac[5]);
    ESP_LOGI(TAG_APP, "MAC Addr: %s", s_Mac);
    
    sscanf( CONFIG_APP_ESPNOW_PEER_MAC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &u8_Mac[0], &u8_Mac[1], &u8_Mac[2], &u8_Mac[3], &u8_Mac[4], &u8_Mac[5] );
    
    snprintf(s_Mac, 18, "%02X:%02X:%02X:%02X:%02X:%02X", u8_Mac[0], u8_Mac[1], u8_Mac[2], u8_Mac[3], u8_Mac[4], u8_Mac[5]);
    ESP_LOGI(TAG_APP, "MAC Addr: %s", s_Mac);
     

#if defined(CONFIG_APP_DEEP_SLEEP_ESP_NOW) || defined(CONFIG_APP_LIGHT_SLEEP_ESP_NOW)
    ret = mod_espnow_init( 12 );
#else
    ret = Backend_Init( );
#endif

    if( ret != ESP_OK ) 
        obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Sys_Init_Failed);    
    else
        obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Sys_Init_Done);    
}


/// @brief     State machine - Not connected state handler
/// @param obj MainApp object
static void MASH_Not_Connected(MAIN_APP_t * obj)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG_APP,"MASH_Not_Connected state");

#if defined(CONFIG_APP_DEEP_SLEEP_ESP_NOW) || defined(CONFIG_APP_LIGHT_SLEEP_ESP_NOW)
    obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_WiFi_Connection_Established);
    return;
 #endif

    obj->b_WaitingForBackendCon = false;

    if(obj->b_WaitingForWiFiCon == false)
    {
        ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Not_Connected");
    
        /*Make sure esp_event_default_loop, esp_netif and nvs_flash are initiated before calling this*/
        //Note: This will throw an error if WiFi cannot connect which will cause a reset or core dump    
        ret = mod_wifi_connect( );
         
        if( ret != ESP_OK ) 
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_WiFi_Failed);         

        obj->b_WaitingForWiFiCon = true;     
    }
    else
    {
        /*Waiting for WiFi module to connect. Transition triggered in WiFi event handler*/
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}


/// @brief     State machine - WiFi connected state handler
/// @param obj MainApp object
static void MASH_WiFi_Connected(MAIN_APP_t * obj)
{    
    if(obj->b_WaitingForBackendCon == false)
    {
        ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_WiFi_Connected");
        
        esp_err_t ret =  ESP_OK;
        ret = Backend_Connect( );

        ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_WiFi_Connected. Backend_Connect ret= %d", ret);
        
        if(ret != ESP_OK)
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Backend_Failed);

        obj->b_WaitingForBackendCon = true;
    }
    else
    {
        /*Waiting for backend module to connect. Transition triggered in backend event handler*/
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


/// @brief State machine - Backend connected state handler
/// @param obj MainApp object
static void MASH_Backend_Connected(MAIN_APP_t * obj)
{    
    //static bool b_WaitingForDataToBeSent = false;
    static uint32_t u32_Timeout = 0;
    
    obj->b_WaitingForBackendCon = false;
    
    //ToDo: Bug Watchdog timeout if not all messages have been sent and the state machine is locked in this state.
    //A timeout is needed after we continue or throw an error.
    //According to the MQTT backlog all 3 messages have been sent. Must be a bug where we are not getting the feedback correctly
    if(obj->b_WaitingForDataToBeSent == false)
    {
        ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Backend_Connected");
        
        if( GetSensorMeasurements( &obj->TH_Values, &obj->f_Light_Lux) != ESP_OK )
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Sensor_Read_Failed);    
        else
        {
#if defined(CONFIG_APP_DEEP_SLEEP_ESP_NOW) || defined(CONFIG_APP_LIGHT_SLEEP_ESP_NOW)
            ESP_ERROR_CHECK( mod_espnow_add_send_data( (void*) (&obj->TH_Values),   sizeof(obj->TH_Values )));     
            ESP_ERROR_CHECK( mod_espnow_add_send_data( (void*) (&obj->f_Light_Lux), sizeof(obj->f_Light_Lux) ));
            ESP_ERROR_CHECK( mod_espnow_send_data( ));            
            
#else
            ESP_ERROR_CHECK(Backend_PublishData(obj));            
#endif
            obj->b_WaitingForDataToBeSent = true;
            u32_Timeout = 50;
        }
    }
    else if( obj->BackendMsgIDs[0] == 0 && 
             obj->BackendMsgIDs[1] == 0 && 
             obj->BackendMsgIDs[2] == 0)
    {
        ESP_LOGI(TAG_APP,"All messages sent to backend successfully");
        obj->b_WaitingForDataToBeSent = false;                
        obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Data_Sent_To_Backend);
    }
    else
    {
        //We are waiting for all messages to be sent
        vTaskDelay(pdMS_TO_TICKS(10));   
        if(u32_Timeout > 0 )
            u32_Timeout--;
        else
        {
            ESP_LOGW(TAG_APP,"STATE_MACHINE - MAS_Backend_Connected. MSG_Timeout.");
            obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Data_Sent_To_Backend);
        }
    }
}


/// @brief     State machine - Data published state handler
/// @param obj MainApp object
static void MASH_Data_Published(MAIN_APP_t * obj)
{
    if(obj->b_WaitToGoToSleep == false)          
    {
        ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Data_Published");
       
#ifdef CONFIG_APP_DEEP_SLEEP_ESP_NOW
        mod_espnow_deinit( );   
#endif

#if defined(CONFIG_APP_AUTO_LIGHT_SLEEP) || defined(CONFIG_APP_DEEP_SLEEP)
        Backend_Disconnect( );                                
#endif
        mod_pwr_save_start( );

#ifdef CONFIG_APP_LIGHT_SLEEP_ESP_NOW
        //mod_espnow_deinit( );        
        obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_Enter_Sleep_Mode);
#endif 
        obj->b_WaitToGoToSleep = true;
    }
    else
        vTaskDelay(pdMS_TO_TICKS(10));    
}


/// @brief     State machine - Sleep state handler (Only for AutoLighSleep)
/// @param obj MainApp object
static void MASH_Sleep(MAIN_APP_t * obj)
{
    static uint32_t u32_MaxDelay_ms = 200;     
    int64_t s64_TimeBefore_us = esp_timer_get_time( ); 
    int64_t s64_TimeAfter_us  = 0; 

    ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Sleep");

    obj->b_WaitToGoToSleep = false;    

#ifdef CONFIG_APP_LIGHT_SLEEP_ESP_NOW     
    mod_pwr_save_start( );    
#else
#if defined(CONFIG_APP_ITWT_ENABLE) && defined (CONFIG_APP_ITWT_ASUS_BUG_WORKAROUND)
    //ToDo: Different sleep modes to be implemented   
    //The Asus AP always deauthenticates after 5min and 30sec regardless of accepting the TWT request
    //Sleep times more than 5min dont work. we need to wake up and make a TWT probe after 5min latest
    if(obj->u32_SleepTimeSec > CONFIG_APP_ITWT_ASUS_BUG_INTERVAL)
    {
        for(int i = 0; i< obj->u32_SleepTimeSec/CONFIG_APP_ITWT_ASUS_BUG_INTERVAL ; i++)
        {
            GoToSleep( CONFIG_APP_ITWT_ASUS_BUG_INTERVAL ); 
            ESP_LOGI(TAG_APP, "TWT probe request with 100ms timeout. err= %d", esp_wifi_sta_itwt_send_probe_req(100));
        }

        GoToSleep( obj->u32_SleepTimeSec % CONFIG_APP_ITWT_ASUS_BUG_INTERVAL ); 
    }
    else
        GoToSleep( obj->u32_SleepTimeSec ); 
#else
    GoToSleep( obj->u32_SleepTimeSec ); 
#endif
#endif   

    /*Back from sleeping so lets stop power saving and connect to the backend again.*/       
    s64_TimeAfter_us = esp_timer_get_time( );
    ESP_LOGI(TAG_APP,"MainApp awake again after %d sec.", (int) ((s64_TimeAfter_us - s64_TimeBefore_us) / 1000000));
    mod_pwr_save_stop( );    

    while(u32_MaxDelay_ms > 0 && obj->CurrentState == MAS_Sleep)
    {
        /*Lets try with a wait state before jumping to next state.*/
        /*ToDo: More clean would be to ping the backend server. To Be implemented*/
        vTaskDelay(pdMS_TO_TICKS(10)); 
        u32_MaxDelay_ms -= 10;
    }    

    //When using ESP-NOW with Auto Light Sleep we dont use an Access Point so it does not matter.
    //Assuming we still have a WiFi connection after Auto Light Sleep with iTWT.    
    //ToDo: Bug to be fixed!
    //If we've lost the WiFi connection we get a race condition when jumping to connected state before we get the WiFi disconnect state.
    //We go to the WiFi connected state and start the backend connection which then fails.
    if(obj->CurrentState == MAS_Sleep)
    {
        ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Sleep --> Assuming WiFi is connected.");
        obj->CurrentState = MAS_Handle_Transition(obj->CurrentState, MAE_WiFi_Connection_Established);
    }    
}


/// @brief     State machine - Error state handler. We are just waiting here for now.
/// @param obj MainApp object
static void MASH_Error(MAIN_APP_t * obj)
{
    ESP_LOGI(TAG_APP,"STATE_MACHINE - MAS_Error");
    vTaskDelay(pdMS_TO_TICKS(180000));    
}


/* Other local supporting functions ------------------------------------------*/
/// @brief        Print the reset reason via ESP_LOGW.
/// @param reason See esp_reset_reason_t for more details
static void Print_Reset_Reason(esp_reset_reason_t reason)
{
  switch ( reason)
  {
    case ESP_RST_UNKNOWN:    //!< Reset reason can not be determined
    ESP_LOGW(TAG_APP, "Reset Reason: UNKNOWN");
    break;
    case ESP_RST_POWERON:    //!< Reset due to power-on event
    ESP_LOGW(TAG_APP, "Reset Reason: Power-on Event");
    break;
    case ESP_RST_EXT:        //!< Reset by external pin (not applicable for ESP32)
    ESP_LOGW(TAG_APP, "Reset Reason: Ext. reset pin");
    break;
    case ESP_RST_SW:         //!< Software reset via esp_restart
    ESP_LOGW(TAG_APP, "Reset Reason: SW Reset");
    break;
    case ESP_RST_PANIC:      //!< Software reset due to exception/panic
    ESP_LOGW(TAG_APP, "Reset Reason: SW Reset due to PANIC");
    break;
    case ESP_RST_INT_WDT:    //!< Reset (software or hardware) due to interrupt watchdog
    ESP_LOGW(TAG_APP, "Reset Reason: SW or HW due to INTR. WATCHDOG");
    break;
    case ESP_RST_TASK_WDT:   //!< Reset due to task watchdog
    ESP_LOGW(TAG_APP, "Reset Reason: TASK WATCHDOG");
    break;
    case ESP_RST_WDT:        //!< Reset due to other watchdogs
    ESP_LOGW(TAG_APP, "Reset Reason: OTHER WATCHDOG");
    break;
    case ESP_RST_DEEPSLEEP:  //!< Reset after exiting deep sleep mode
    ESP_LOGW(TAG_APP, "Reset Reason: DEEP SLEEP");
    break;
    case ESP_RST_BROWNOUT:   //!< Brownout reset (software or hardware)
    ESP_LOGW(TAG_APP, "Reset Reason: BROWNOUT");
    break;
    case ESP_RST_SDIO:       //!< Reset over SDIO
    ESP_LOGW(TAG_APP, "Reset Reason: SDIO");
    break;
    default: 
    ESP_LOGE(TAG_APP, "Reset Reason: UNKNWON_DEFAULT");
  }
}


/// @brief                  Read the temp, humidity and lux values from sensors.
/// @param[out] p_TH_Values Temp and humid data will be written to struct
/// @param[out] pf_Lux      Lux data will be written to 
/// @return                 ESP_OK if no error
/// @note                   This function will block the calling task by at least 900ms due to necessary wait times
/// @note                   This function will turn the power to the sensors on and off.  
static esp_err_t GetSensorMeasurements(TEMP_HUMID_VALUES_t *p_TH_Values, float *pf_Lux)
{
    esp_err_t ret = ESP_OK;

    //Must be called before the I2C sensor init functions to enable power to the sensors
    ESP_ERROR_CHECK(mod_pwr_PeriphPWR(true));

    //Enable power to the I2C devices again and re-init the sensors if needed            
    ESP_ERROR_CHECK(i2cdev_init());

    ESP_ERROR_CHECK(mod_light_init());
    ESP_ERROR_CHECK(mod_th_meas_init());

    ESP_ERROR_CHECK(mod_th_meas_GetValues(p_TH_Values));    
    //ToDo: Optimization. The light sensor should be initialized much earlier to be ready with the data when needed.
    //ToDo: Create task to get the data in parallel to state machine getting ready with the connections.    
    vTaskDelay(pdMS_TO_TICKS(400));    
    ESP_ERROR_CHECK(mod_light_Get(pf_Lux));    
  	
    ESP_ERROR_CHECK(i2cdev_done());    
    ESP_ERROR_CHECK(mod_pwr_PeriphPWR(false));     

    ESP_ERROR_CHECK(mod_light_Deinit( ));
    ESP_ERROR_CHECK(mod_th_meas_Deinit( ));
    
    return ret;        
}


/// @brief     Sends the temp, humid and lux data to the backend.
/// @param obj MainApp object holding the data to send
/// @return    ESP_OK if no errors otherwise ESP_FAIL
static esp_err_t Backend_PublishData(MAIN_APP_t * obj)
{
    esp_err_t ret = ESP_OK;    
    BACKEND_MESSAGE_t backend_msg;

    //Get Temperature reading and send to backend        
    if( s32_ConvertTemp_f_to_str( backend_msg.str_Data, sizeof(backend_msg.str_Data), &obj->TH_Values ) > 0)
    {            
        backend_msg.topic = AmbientTempC;        
        Backend_SendMessage(&backend_msg);
        obj->BackendMsgIDs[0] = backend_msg.s32_Msg_ID;
    }
    else
        ret = ESP_FAIL;

    //Get Humidity reading and send to backend        
    if( s32_ConvertHumi_f_to_str( backend_msg.str_Data, sizeof(backend_msg.str_Data), &obj->TH_Values ) > 0)
    {            
        backend_msg.topic = Humidity;        
        Backend_SendMessage(&backend_msg);            
        obj->BackendMsgIDs[1] = backend_msg.s32_Msg_ID;
    }
    else
        ret = ESP_FAIL;

    //Get light reading and send to backend
    if( s32_ConvertLux_f_to_str( backend_msg.str_Data, sizeof(backend_msg.str_Data), &obj->f_Light_Lux ) > 0)
    {
        backend_msg.topic = Light;
        Backend_SendMessage(&backend_msg);            
        obj->BackendMsgIDs[2] = backend_msg.s32_Msg_ID;
    }
    else
        ret = ESP_FAIL;
    
    return ret;
}


/// @brief                  Function to pause calling task via vTaskDelay(..)
/// @param u32_SleepTimeSec Sleep time in seconds
static void GoToSleep(uint32_t u32_SleepTimeSec)
{
    //Putting the main task to sleep will put the device into Auto-Light-Sleep mode
    vTaskDelay(pdMS_TO_TICKS(u32_SleepTimeSec*1000));    
}


/*****************************END OF FILE**************************************/