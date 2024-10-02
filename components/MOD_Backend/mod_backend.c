/**
  ******************************************************************************
  * @file    mod_backend.c
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

/* Includes ------------------------------------------------------------------*/
#include "mod_backend.h"



/* Private typedef -----------------------------------------------------------*/
typedef struct MOD_BACKEND_HDL_t
{
    esp_mqtt_client_handle_t MQTT_client_hdl;
    volatile bool b_MQTT_Connected;
    volatile bool b_MQTT_Reconnect;
    volatile bool b_MQTT_ClientRunning;   

}MOD_BACKEND_HDL_t;

/* Private define ------------------------------------------------------------*/
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define CONFIG_BROKER_URL "mqtt://" CONFIG_APP_MQTT_BROKER_IP_ADR ":" STR(CONFIG_APP_MQTT_BROKER_IP_PORT)

//MQTT topic defines.
#define MQTT_TOPIC_DEVICE_ID        CONFIG_APP_MQTT_DEVICE_LOCATION "/DeviceID" 
#define MQTT_TOPIC_MAIN_LOOP_CNT    CONFIG_APP_MQTT_DEVICE_LOCATION "/" CONFIG_APP_MQTT_DEVICE_ID "/MainLoopCnt" 
#define MQTT_TOPIC_AMBIENT_TEMP_C   CONFIG_APP_MQTT_DEVICE_LOCATION "/" CONFIG_APP_MQTT_DEVICE_ID "/AmbientTempCel" 
#define MQTT_TOPIC_HUMIDITY         CONFIG_APP_MQTT_DEVICE_LOCATION "/" CONFIG_APP_MQTT_DEVICE_ID "/Humidity" 
#define MQTT_TOPIC_LIGHT            CONFIG_APP_MQTT_DEVICE_LOCATION "/" CONFIG_APP_MQTT_DEVICE_ID "/Light" 


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/
static const char *TAG_BAC = "mod_backend";


/* Private variables ---------------------------------------------------------*/
MOD_BACKEND_HDL_t mod_backend;

esp_mqtt_client_config_t mqtt_cfg =
{
    .broker.address.uri                  = CONFIG_BROKER_URL,
    .credentials.username                = CONFIG_APP_MQTT_BROKER_USER_NAME,
    .credentials.client_id               = CONFIG_APP_MQTT_DEVICE_ID,        
    .credentials.authentication.password = CONFIG_APP_MQTT_BROKER_USER_PW,
};

/* Private function prototypes -----------------------------------------------*/
static void log_error_if_nonzero(const char *message, int error_code);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);


/* Exported functions --------------------------------------------------------*/

/// @brief  Init the backend module
/// @param  void
/// @return ESP_OK on success
/// @note   Call this function before calling any other function from this module
esp_err_t Backend_Init(void)
{
    //Ensure flash is initialized prior to starting the mqtt client.
    ESP_ERROR_CHECK(nvs_flash_init());    

    mod_backend.MQTT_client_hdl = esp_mqtt_client_init(&mqtt_cfg);

    mod_backend.b_MQTT_Connected = false;
    mod_backend.b_MQTT_Reconnect = true;    
    mod_backend.b_MQTT_ClientRunning = false;

    //EventDispatcher_RegisterEventHandler(MOD_WIFI_EVENTS, WIFI_CONNECTED_EVENT,    WiFi_events_handler, NULL);
    //EventDispatcher_RegisterEventHandler(MOD_WIFI_EVENTS, WIFI_DISCONNECTED_EVENT, WiFi_events_handler, NULL);  

    if( mod_backend.MQTT_client_hdl == NULL )      
        return ESP_FAIL;
    
    return ESP_OK;
}


/// @brief  De-Init the backend module
/// @param  void
/// @return ESP_OK on success
esp_err_t Backend_DeInit(void)
{
    esp_err_t ret = ESP_OK;

    ret = esp_mqtt_client_destroy(mod_backend.MQTT_client_hdl);
    log_error_if_nonzero("Backend_DeInit: DeInit client failed", ret);

    return ret;
}


/// @brief  Call this function to connect to the backend
/// @param  void
/// @return ESP_OK on success
/// @note   Ensure to call Backend_Init(..) before calling this function
/// @todo:  If this function is called multiple times we get a -1 (FAIL) back. We need to be able to handle mutliple calls without returning an error
esp_err_t Backend_Connect(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG_BAC, "Backend_Connect to Broker URL:%s", CONFIG_BROKER_URL);    

    if( mod_backend.b_MQTT_ClientRunning == false)
    {
        /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
        ret = esp_mqtt_client_register_event(mod_backend.MQTT_client_hdl, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
        if( ret != ESP_OK)
            return ret;
    
        ret = esp_mqtt_client_start(mod_backend.MQTT_client_hdl);
        if( ret != ESP_OK)
            return ret;
    
        mod_backend.b_MQTT_ClientRunning = true;
    }

    return ret;
}


/// @brief Stops the backend connection
/// @note  This will force a stop. To connect again Backend_Start() has to be called.
void Backend_Disconnect(void)
{
    if( mod_backend.b_MQTT_Connected == true )
    {
        /* Unregister event handler first in case we are still connected.
           Otherwise the event handler will be invoked on the MQTT disconnect event and at that time we might have 
           managed to destroy the client which will cause an error.
           */
        log_error_if_nonzero("Backend_Stop: Unregister event handler", esp_mqtt_client_unregister_event(mod_backend.MQTT_client_hdl, ESP_EVENT_ANY_ID, mqtt_event_handler));    
    
        //Try to gracefully shutdown of MQTT connection
        if(mod_backend.b_MQTT_Connected == true)
            log_error_if_nonzero("Backend_Stop: Disconnect failure", esp_mqtt_client_disconnect(mod_backend.MQTT_client_hdl));
        
        if(mod_backend.b_MQTT_ClientRunning == true)
        {
            log_error_if_nonzero("Backend_Stop: Stop client failure", esp_mqtt_client_stop(mod_backend.MQTT_client_hdl));        
            mod_backend.b_MQTT_ClientRunning = false;
        }   
    
        mod_backend.b_MQTT_Connected = false;
    }
}


/// @brief Send a message to the backend
/// @param Message See BACKEND_MESSAGE_t for details.
void Backend_SendMessage(BACKEND_MESSAGE_t* Message)
{
    int s32_msg_id = 0; 
      
    switch(Message->topic)
    {
        case AmbientTempC:                         
            s32_msg_id = esp_mqtt_client_publish(mod_backend.MQTT_client_hdl, MQTT_TOPIC_AMBIENT_TEMP_C, Message->str_Data, 0, CONFIG_APP_MQTT_QoS, 0);        
        break;

        case Humidity:
            s32_msg_id = esp_mqtt_client_publish(mod_backend.MQTT_client_hdl, MQTT_TOPIC_HUMIDITY, Message->str_Data , 0, CONFIG_APP_MQTT_QoS, 0);        
        break;

        case Light:
            s32_msg_id = esp_mqtt_client_publish(mod_backend.MQTT_client_hdl, MQTT_TOPIC_LIGHT, Message->str_Data , 0, CONFIG_APP_MQTT_QoS, 0);                                
        break;

        default:
        {
            ESP_LOGE(TAG_BAC, "Backend_SendMessage error. Topic: %d, Err: Unknown topic", Message->topic);                
            return; //Unknown topic            
        }
    }     
    
    if(s32_msg_id == -2)
        ESP_LOGE(TAG_BAC, "Backend_SendMessage error. Outbox full."); 
    else if(s32_msg_id == -1)
        ESP_LOGE(TAG_BAC, "Backend_SendMessage error. Failure"); 
    else
        Message->s32_Msg_ID = s32_msg_id;    
}


/* Private functions ---------------------------------------------------------*/

/// @brief            If the error is not ESP_OK the message will be logged
/// @param message    message to write in output log
/// @param error_code Errro code
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) 
    {
        ESP_LOGE(TAG_BAC, "Last error %s: 0x%x", message, error_code);
    }
}


/// @brief              Event handler registered to receive MQTT events
/// @param handler_args Additional user data registered to the event
/// @param base         Event base from subsystem (always MQTT Base in this example).
/// @param event_id     The id for the received event.
/// @param event_data   The data for the event, esp_mqtt_event_handle_t.
/// @note               This function is called by the MQTT client event loop.
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_err_t ESP_err = ESP_OK;

    ESP_LOGD(TAG_BAC, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
        
    switch ((esp_mqtt_event_id_t)event_id) 
    {
        case MQTT_EVENT_CONNECTED:
            mod_backend.b_MQTT_Connected = true;
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_CONNECTED");
            EventDispatcher_PostEvent(MOD_BACKEND_EVENTS, BACKEND_CONNECTED_EVENT, NULL, 0, portMAX_DELAY);            
            break;

        case MQTT_EVENT_DISCONNECTED:
            mod_backend.b_MQTT_Connected = false;
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_DISCONNECTED");

            EventDispatcher_PostEvent(MOD_BACKEND_EVENTS, BACKEND_DISCONNECTED_EVENT, NULL, 0, portMAX_DELAY);              

            /*If we lost the connection(not on purpose) try to reconnect*/
            if(mod_backend.b_MQTT_Reconnect == true)
            {                   
                //ToDo: Max counter for reconnection tries plus a delay before trying a reconnect
                ESP_err = esp_mqtt_client_reconnect(client);
                log_error_if_nonzero("MQTT reconnect failed", ESP_err);            
                /* Note: esp_mqtt_client_stop(client)) will not work here because the mqtt client cannot be stopped from the the MQTT task itself!
                         Call Backend_Stop() to stop the client connection. */
                if( ESP_err != ESP_OK )
                    EventDispatcher_PostEvent(MOD_BACKEND_EVENTS, BACKEND_CONNECT_FAILED_EVENT, NULL, 0, portMAX_DELAY);              
            }   
            break;
    
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_PUBLISHED, msg_id=%d, size:%u", event->msg_id, sizeof(event->msg_id));
            EventDispatcher_PostEvent(MOD_BACKEND_EVENTS, BACKEND_SEND_MESSAGE_DONE, (const void*)(&event->msg_id), sizeof(event->msg_id), portMAX_DELAY);  
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_BAC, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG_BAC, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    
            }
            break;
        default:
            ESP_LOGI(TAG_BAC, "Other event id:%d", event->event_id);
            break;
    }
}


/*****************************END OF FILE**************************************/


