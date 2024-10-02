/**
  ******************************************************************************
  * @file    mod_esp_now.c
  * @author  The Embedded Dude
  * @brief   ESP now module to handle ESP now communication
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
#include "mod_esp_now.h"
#include "esp_err.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "mod_eventDispatcher.h"
#include "app_events.h"


/* Private typedef -----------------------------------------------------------*/


typedef struct MOD_ESPNOW_DATA_t
{    
    uint32_t u32_MsgCnt;                      //!< Total count of ESPNOW messages sent.        
    uint8_t *pu8_Buffer;                      //!< Buffer pointing to ESPNOW data.
    size_t   u32_MaxBuffSize;                 //!< Max buffer size in bytes.    
    size_t   u32_Len;                         //!< Length of data to be sent in bytes.
    uint8_t  u8_dest_mac[ESP_NOW_ETH_ALEN];   //!< MAC address of destination device.

} MOD_ESPNOW_DATA_t;


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private constants ---------------------------------------------------------*/
const char *TAG_ESPNOW = "mod_esp_now";


/* Private variables ---------------------------------------------------------*/
MOD_ESPNOW_DATA_t mod_espnow_obj;


/* Private function prototypes -----------------------------------------------*/
static esp_err_t mod_espnow_init_wifi(void);
static esp_err_t mod_espnow_init_module(void);

static void mod_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
static esp_err_t Str2Mac(const char* str_mac, uint8_t* mac);



/* Exported functions --------------------------------------------------------*/

/// @brief                    Init the ESP-NOW module
/// @param u32_MaxBufferSize  Maximum buffer size for the ESP-NOW messages. Must be < than ESP_NOW_MAX_DATA_LEN
/// @return                   ESP_OK on success
/// @note                     This module will also inti the WiFi and ESP-Now. IT would be more clean to have WiFi being initialized in WiFi Module.
/// @note                     It does not makes sense to have more than one object due to the fact that the module also controls WiFi and ESP-NOW init and deinit. 
esp_err_t mod_espnow_init( size_t u32_MaxBufferSize )
{
#if CONFIG_APP_ESPNOW_ENABLE
    if( u32_MaxBufferSize > ESP_NOW_MAX_DATA_LEN )
        return ESP_ERR_INVALID_SIZE;

    mod_espnow_obj.u32_MaxBuffSize = u32_MaxBufferSize;
    mod_espnow_obj.pu8_Buffer      = malloc(mod_espnow_obj.u32_MaxBuffSize); 
    
    if (mod_espnow_obj.pu8_Buffer == NULL) 
    {
        ESP_LOGE(TAG_ESPNOW, "Malloc data buffer failed!");
        return ESP_FAIL;
    }

    memset(mod_espnow_obj.pu8_Buffer, 0, mod_espnow_obj.u32_MaxBuffSize);

    ESP_ERROR_CHECK(mod_espnow_init_wifi( ));
    ESP_ERROR_CHECK(mod_espnow_init_module( ));
        
#else
    return ESP_ERR_INVALID_ARG;
#endif

    return ESP_OK;
}


/// @brief De-Init ESP-NOW module
/// @param void
void mod_espnow_deinit( void )
{
#if CONFIG_APP_ESPNOW_ENABLE

    free(mod_espnow_obj.pu8_Buffer);
    mod_espnow_obj.u32_MaxBuffSize = 0;    
    
    ESP_ERROR_CHECK(esp_now_unregister_send_cb( ));
    ESP_ERROR_CHECK(esp_now_deinit( ));
    ESP_ERROR_CHECK(esp_wifi_stop( ));
    ESP_ERROR_CHECK(esp_wifi_deinit( ));
#endif
}


/// @brief         Call this function to prepare the data for sending using ESP-NOW
/// @param p_Data  Pointer to the data buffer containing the data to be send
/// @param u32_Len Number of bytes to send
/// @return        ESP_OK on success
/// @note          Before calling this function, ensure mod_espnow_init(..) has been called prior
esp_err_t mod_espnow_add_send_data( void *p_Data, size_t u32_Len )
{
    if(mod_espnow_obj.u32_MaxBuffSize < mod_espnow_obj.u32_Len + u32_Len )
    {
        mod_espnow_obj.u32_Len = 0;
        return ESP_ERR_NO_MEM;
    }
    
    memcpy(mod_espnow_obj.pu8_Buffer + mod_espnow_obj.u32_Len, p_Data, u32_Len);

    mod_espnow_obj.u32_Len += u32_Len;

    return ESP_OK;
}


/// @brief  Send data using ESP-NOW
/// @param  void
/// @return ESP_OK on success
/// @note   Ensure to call mod_espnow_add_send_data(..) prior to prepare the data for sending
esp_err_t mod_espnow_send_data( void )
{    
    esp_err_t ret =  ESP_OK;
    
    ESP_ERROR_CHECK(esp_wifi_start( ));    
    ret = esp_now_send( mod_espnow_obj.u8_dest_mac, mod_espnow_obj.pu8_Buffer, mod_espnow_obj.u32_Len );
    
    mod_espnow_obj.u32_Len = 0;

    return ret;
}


/* Private functions ---------------------------------------------------------*/

/// @brief  Init WiFi 
/// @param  void 
/// @return ESP_OK on success
static esp_err_t mod_espnow_init_wifi( void )
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT( );
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ));
    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ));
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ));
    ESP_ERROR_CHECK( esp_wifi_start( ));
    ESP_ERROR_CHECK( esp_wifi_set_channel( CONFIG_APP_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE ));

#if CONFIG_APP_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
    
    return ESP_OK;
}


/// @brief  Init ESP-NOW and register the send callback function
/// @param  void
/// @return ESP_OK on success
static esp_err_t mod_espnow_init_module(void)
{ 
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(mod_espnow_send_cb) ); 

#if CONFIG_APP_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK( esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW) );
    ESP_ERROR_CHECK( esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL) );
#endif
    
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_APP_ESPNOW_PMK) );

    /*Get peer mac address from sdkconfig*/
    ESP_ERROR_CHECK( Str2Mac(CONFIG_APP_ESPNOW_PEER_MAC, mod_espnow_obj.u8_dest_mac ));

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) 
    {
        ESP_LOGE(TAG_ESPNOW, "Malloc peer information failed");        
        esp_now_deinit();
        return ESP_FAIL;
    }

    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_APP_ESPNOW_CHANNEL;
    peer->ifidx   = ESP_IF_WIFI_STA;
    peer->encrypt = false;
    memcpy(peer->peer_addr, mod_espnow_obj.u8_dest_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);
    
    return ESP_OK;
}


/// @brief          Callback function that gets called when ESP-NOW has been sent
/// @param mac_addr Mac address
/// @param status   Send status see esp_now_send_status_t
/// @note           ESPNOW sending or receiving callback function is called in WiFi task. Dont block the calling thread!
static void mod_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    MOD_ESPNOW_EVENTS_ENUM_t espnow_event = ESPNOW_DATA_SENT_FAILED;

    if (mac_addr == NULL)     
        ESP_LOGE(TAG_ESPNOW, "ESP-NOW: Send cb arg error");            
    else if(status == ESP_NOW_SEND_FAIL)    
        ESP_LOGE(TAG_ESPNOW, "ESP_NOW: Send failed!");           
    else    
        espnow_event = ESPNOW_DATA_SENT;
        
    EventDispatcher_PostEvent(MOD_ESPNOW_EVENTS, espnow_event, NULL, 0, portMAX_DELAY);        
}


/// @brief         Converts a string containing a mac address byte values
/// @param str_mac String that contains the mac address in the format ff:ff:ff:ff:ff:ff
/// @param mac     Destination array the converted bytes will be written to
/// @return        ESP_OK if conversation was ok, otherwise ESP_FAIL
/// @note          Ensure that mac is at least 6 bytes big
static esp_err_t Str2Mac(const char* str_mac, uint8_t* mac)
{
    esp_err_t ret = ESP_OK;

    if( ESP_NOW_ETH_ALEN != sscanf( str_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",&mac[0], &mac[1], &mac[2],&mac[3], &mac[4], &mac[5] ))
        ret = ESP_FAIL;
    
    return ret;
}

/*****************************END OF FILE**************************************/

