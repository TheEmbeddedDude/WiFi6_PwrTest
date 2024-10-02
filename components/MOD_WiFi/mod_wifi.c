 /**
  ******************************************************************************
  * @file    mod_wifi.c
  * @author  The Embedded Dude
  * @brief   Handles WiFi init, config, de-init etc.  
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
#include "mod_wifi.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define APP_NETIF_DESC_STA "mod_wifi_netif_sta"

#if CONFIG_APP_WIFI_SCAN_METHOD_FAST
#define CONFIG_APP_WIFI_SCAN_METHOD WIFI_FAST_SCAN
#elif CONFIG_APP_WIFI_SCAN_METHOD_ALL_CHANNEL
#define CONFIG_APP_WIFI_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#endif

#if CONFIG_APP_WIFI_CONNECT_AP_BY_SIGNAL
#define CONFIG_APP_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_APP_WIFI_CONNECT_AP_BY_SECURITY
#define CONFIG_APP_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY
#endif

#if CONFIG_APP_WIFI_AUTH_OPEN
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_APP_WIFI_AUTH_WEP
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_APP_WIFI_AUTH_WPA_PSK
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_APP_WIFI_AUTH_WPA2_PSK
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_APP_WIFI_AUTH_WPA_WPA2_PSK
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_APP_WIFI_AUTH_WPA2_ENTERPRISE
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_ENTERPRISE
#elif CONFIG_APP_WIFI_AUTH_WPA3_PSK
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_APP_WIFI_AUTH_WPA2_WPA3_PSK
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_APP_WIFI_AUTH_WAPI_PSK
#define CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/*set the following settings via "idf.py menuconfig"*/
#define DEFAULT_SSID                           CONFIG_APP_WIFI_SSID
#define DEFAULT_PWD                            CONFIG_APP_WIFI_PASSWORD
#define DEFAULT_WIFI_SCAN_METHOD               CONFIG_APP_WIFI_SCAN_METHOD 
#define DEFAULT_WIFI_CONNECT_AP_SORT_METHOD    CONFIG_APP_WIFI_CONNECT_AP_SORT_METHOD                                               
#define DEFAULT_WIFI_SCAN_RSSI_THRESHOLD       CONFIG_APP_WIFI_SCAN_RSSI_THRESHOLD
#define DEFAULT_WIFI_SCAN_AUTH_MODE_THRESHOLD  CONFIG_APP_WIFI_SCAN_AUTH_MODE_THRESHOLD


#if CONFIG_APP_CONNECT_IPV6
#define MAX_IP6_ADDRS_PER_NETIF (5)

#if defined(CONFIG_APP_CONNECT_IPV6_PREF_LOCAL_LINK)
#define CONFIG_APP_CONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_LINK_LOCAL
#elif defined(CONFIG_APP_CONNECT_IPV6_PREF_GLOBAL)
#define CONFIG_APPCONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_GLOBAL
#elif defined(CONFIG_APP_CONNECT_IPV6_PREF_SITE_LOCAL)
#define CONFIG_APPCONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_SITE_LOCAL
#elif defined(CONFIG_APP_CONNECT_IPV6_PREF_UNIQUE_LOCAL)
#define CONFIG_APPCONNECT_PREFERRED_IPV6_TYPE ESP_IP6_ADDR_IS_UNIQUE_LOCAL
#endif // if-elif CONFIG_EXAMPLE_CONNECT_IPV6_PREF_...
#endif




/* Private macro -------------------------------------------------------------*/



/* Private constants ---------------------------------------------------------*/
const char *TAG = "mod_wifi";

#if CONFIG_APP_CONNECT_IPV6
/* types of ipv6 addresses to be displayed on ipv6 events */
const char *ipv6_addr_types_to_str[6] = 
{
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
};
#endif


/* Private variables ---------------------------------------------------------*/
static esp_netif_t       *s_app_sta_netif = NULL;
static SemaphoreHandle_t  s_semph_get_ip_addrs = NULL;
#if CONFIG_APP_CONNECT_IPV6
static SemaphoreHandle_t s_semph_get_ip6_addrs = NULL;
#endif

#if CONFIG_EXAMPLE_ITWT_TRIGGER_ENABLE
uint8_t trigger_enabled = 1;
#else
uint8_t trigger_enabled = 0;
#endif

#if CONFIG_EXAMPLE_ITWT_ANNOUNCED
uint8_t flow_type_announced = 1;
#else
uint8_t flow_type_announced = 0;
#endif

#if CONFIG_APP_WIFI_POWER_SAVE_NONE
wifi_ps_type_t Wifi_PWR_SaveType = WIFI_PS_NONE;
#elif CONFIG_APP_WIFI_POWER_SAVE_MIN
wifi_ps_type_t Wifi_PWR_SaveType = WIFI_PS_MIN_MODEM;
#elif CONFIG_APP_WIFI_POWER_SAVE_MAX
wifi_ps_type_t Wifi_PWR_SaveType = WIFI_PS_MAX_MODEM;
#elif
wifi_ps_type_t Wifi_PWR_SaveType = WIFI_PS_NONE;
#endif


esp_netif_t *netif_sta = NULL;
const int CONNECTED_BIT = BIT0;
const int DISCONNECTED_BIT = BIT1;
volatile bool b_WiFi_Connected = false;
static int s_retry_num = 0;
static bool b_WiFi_Reconnect = true; //Determines if WiFi reconnect should be tried after disconnect


/* Private function prototypes -----------------------------------------------*/
static esp_err_t mod_wifi_init(void);
static void mod_wifi_shutdown(void);

static void mod_wifi_start(void);
static void mod_wifi_stop(void);

static esp_err_t mod_wifi_sta_do_connect(wifi_config_t wifi_config, bool wait);
static esp_err_t mod_wifi_sta_do_disconnect(void);

static void mod_wifi_handler_on_wifi_connect(void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_on_sta_got_ipv6(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_itwt_setup(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_itwt_probe(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_itwt_suspend(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void mod_wifi_handler_itwt_teardown(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static bool mod_wifi_is_our_netif(const char *prefix, esp_netif_t *netif);
static esp_netif_t *mod_wifi_get_netif_from_desc(const char *desc);
static void mod_wifi_print_all_netif_ips(const char *prefix);

static void mod_wifi_set_static_ip(esp_netif_t *netif);
static const char *mod_wifi_itwt_probe_status_to_str(wifi_itwt_probe_status_t status);
static const char *mod_wifi_phy_mode_to_str(wifi_phy_mode_t wifi_phy_mode);



/* Exported functions --------------------------------------------------------*/

/// @brief  Init WiFi as STA and try to connect to AP
/// @param  void
/// @return ESP_OK on success
esp_err_t mod_wifi_connect(void)
{
    if (mod_wifi_init() != ESP_OK) 
    {
        return ESP_FAIL;
    }

    //If we do a reset via  esp_restart() this handler is called prior to the restart. 
    //Deep sleep is not a restart (I assume)
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&mod_wifi_shutdown));
    
    mod_wifi_print_all_netif_ips(APP_NETIF_DESC_STA);

    b_WiFi_Reconnect = true;

    return ESP_OK;
}


/// @brief               Disconnect from AP and shutwdown WiFi
/// @param b_CreateEvent If true a WIFI_DISCONNECTED_EVENT will be send otherwise no event will be generated
/// @return              ESP_OK on success
esp_err_t mod_wifi_disconnect(bool b_CreateEvent)
{
    //Make sure we dont try a reconnect
    b_WiFi_Reconnect = false;

    if( b_CreateEvent == false )
    {
        //Unregister the Disconnect handler before shutting down otherwise we will get an WIFI_DISCONNECT Event
        ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &mod_wifi_handler_on_wifi_disconnect));    
    }

    mod_wifi_shutdown();

    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&mod_wifi_shutdown));

    return ESP_OK;
}


/// @brief Init iTWT
/// @param void
/// @note Ensure we are connected to WiFi and got an IP address before calling this function 
/// @note If we are not in HE20 mode iTWT init will fail.
void mod_wifi_init_iTWT(void)
{
    esp_err_t err = ESP_OK;

    /* setup a trigger-based announce individual TWT agreement. */
    wifi_phy_mode_t phymode;
    wifi_config_t sta_cfg = { 0, };
    esp_wifi_get_config(WIFI_IF_STA, &sta_cfg);
    esp_wifi_sta_get_negotiated_phymode(&phymode);

    ESP_LOGI(TAG, "Wi-Fi Phy mode: %s", mod_wifi_phy_mode_to_str(phymode));

    if (phymode == WIFI_PHY_MODE_HE20) 
    {
#if CONFIG_APP_ITWT_ENABLE        
        
        wifi_twt_setup_config_t setup_config = 
        {
            .setup_cmd       = TWT_REQUEST,
            .flow_id         = 0,
            .twt_id          = CONFIG_APP_ITWT_ID,
            .flow_type       = flow_type_announced ? 0 : 1,
            .min_wake_dura   = CONFIG_APP_ITWT_MIN_WAKE_DURA,
            .wake_invl_expn  = CONFIG_APP_ITWT_WAKE_INVL_EXPN,
            .wake_invl_mant  = CONFIG_APP_ITWT_WAKE_INVL_MANT,
            .trigger         = trigger_enabled,
            .timeout_time_ms = CONFIG_APP_ITWT_SETUP_TIMEOUT_TIME_MS,
        };

        err = esp_wifi_sta_itwt_setup(&setup_config);

        if (err != ESP_OK) 
            ESP_LOGE(TAG, "itwt setup failed, err:0x%x", err);        
#else   
        ESP_LOGI(TAG, "iTWT is disabled. To enable set CONFIG_APP_ITWT_ENABLE");
#endif
    } 
    else 
    {
#if CONFIG_APP_ITWT_ENABLE   
        ESP_LOGE(TAG, "iTWT setup not possible. Must be in 11ax mode to support iTWT.");
#endif
    }
}


/// @brief Stop iTWT - Teardown of session
/// @param  void
void mod_wifi_stop_iTWT(void)
{
    esp_err_t err = ESP_OK;

    err = esp_wifi_sta_itwt_teardown(FLOW_ID_ALL);

    if (err != ESP_OK) 
        ESP_LOGE(TAG, "itwt stop failed, err:0x%x", err);    
}


/* Private functions ---------------------------------------------------------*/


/// @brief  Try to disconnect STA from AP
/// @param  void
static void mod_wifi_shutdown(void)
{
    mod_wifi_sta_do_disconnect();
}


/// @brief  Init WiFi as STA using 20Mhz channels. Also sets the Power mode
/// @param  void 
/// @return ESP_OK on success
static esp_err_t mod_wifi_init(void)
{
    ESP_LOGI(TAG, "Wi-Fi connecting..");
    
    mod_wifi_start();
    
    wifi_config_t wifi_config = 
    {
        .sta = 
        {
            .ssid               = DEFAULT_SSID,
            .password           = DEFAULT_PWD,
            .scan_method        = DEFAULT_WIFI_SCAN_METHOD,
            .sort_method        = DEFAULT_WIFI_CONNECT_AP_SORT_METHOD,
            .threshold.rssi     = DEFAULT_WIFI_SCAN_RSSI_THRESHOLD,
            .threshold.authmode = DEFAULT_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };

    /*We only need 20Mhz channels not 40Mhz*/
    esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);
    
    //Note: Not needed if CONFIG_SOC_WIFI_HE_SUPPORT is enabled in menuconfig
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX);
    esp_wifi_set_ps(Wifi_PWR_SaveType);

#if CONFIG_APP_IP_ENABLE_STATIC_IP
    mod_wifi_set_static_ip(s_app_sta_netif);
#endif
   
    return mod_wifi_sta_do_connect(wifi_config, true);
}


/// @brief             Try to connect to an AP
/// @param wifi_config WiFi config 
/// @param wait        false = not waiting until IP addr. assigned, true = waiting for IP addr.
/// @return            ESP_OK on success
static esp_err_t mod_wifi_sta_do_connect(wifi_config_t wifi_config, bool wait)
{
    if (wait)
    {
        s_semph_get_ip_addrs = xSemaphoreCreateBinary();
        
        if (s_semph_get_ip_addrs == NULL)         
            return ESP_ERR_NO_MEM;
        
#if CONFIG_APP_CONNECT_IPV6
        s_semph_get_ip6_addrs = xSemaphoreCreateBinary();
        
        if (s_semph_get_ip6_addrs == NULL) 
        {
            vSemaphoreDelete(s_semph_get_ip_addrs);
            return ESP_ERR_NO_MEM;
        }
#endif
    }
    
    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &mod_wifi_handler_on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP,         &mod_wifi_handler_on_sta_got_ip,      NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,    &mod_wifi_handler_on_wifi_connect,    s_app_sta_netif));

#if CONFIG_APP_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &mod_wifi_handler_on_sta_got_ipv6, NULL));
#endif

#if CONFIG_APP_ITWT_ENABLE    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_ITWT_SETUP,    &mod_wifi_handler_itwt_setup,    NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_ITWT_TEARDOWN, &mod_wifi_handler_itwt_teardown, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_ITWT_SUSPEND,  &mod_wifi_handler_itwt_suspend,  NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_ITWT_PROBE,    &mod_wifi_handler_itwt_probe,    NULL, NULL));
#endif

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));    
    esp_err_t ret = esp_wifi_connect();

    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    
    if (wait) 
    {
        ESP_LOGI(TAG, "Waiting for IP(s)");
#if CONFIG_APP_CONNECT_IPV4
        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
#endif
#if CONFIG_APP_CONNECT_IPV6
        xSemaphoreTake(s_semph_get_ip6_addrs, portMAX_DELAY);
#endif
        if (s_retry_num > CONFIG_APP_WIFI_CONN_MAX_RETRY) 
            return ESP_FAIL;        
    }

    return ESP_OK;
}


/// @brief  Disconnect STA from AP
/// @param  void
/// @return ESP_OK on success
static esp_err_t mod_wifi_sta_do_disconnect(void)
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &mod_wifi_handler_on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &mod_wifi_handler_on_sta_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &mod_wifi_handler_on_wifi_connect));

#if CONFIG_APP_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &mod_wifi_handler_on_sta_got_ipv6));
#endif
    
    if (s_semph_get_ip_addrs) 
        vSemaphoreDelete(s_semph_get_ip_addrs);
    
#if CONFIG_APP_CONNECT_IPV6
    if (s_semph_get_ip6_addrs) 
        vSemaphoreDelete(s_semph_get_ip6_addrs);    
#endif
    return esp_wifi_disconnect();
}


/// @brief  Start WiFi module
/// @param  void
static void mod_wifi_start(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = APP_NETIF_DESC_STA;
    esp_netif_config.route_prio = 128;
    
    s_app_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}


/// @brief  Stop WiFi module
/// @param  void
static void mod_wifi_stop(void)
{
    esp_err_t err = esp_wifi_stop();

    if (err == ESP_ERR_WIFI_NOT_INIT) 
        return;
    
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(s_app_sta_netif));
    
    esp_netif_destroy(s_app_sta_netif);
    s_app_sta_netif = NULL;
}


/* Private handler functions ---------------------------------------------------------*/

/// @brief            WiFi event handler when disconnected
/// @param arg        Additional data registered to the event
/// @param event_base Event base from subsystem
/// @param event_id   The received event ID
/// @param event_data The data for the event
static void mod_wifi_handler_on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    s_retry_num++;

    if (s_retry_num > CONFIG_APP_WIFI_CONN_MAX_RETRY) 
    {
        ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);
        /* let mod_wifi_sta_do_connect() return */
        if (s_semph_get_ip_addrs) 
            xSemaphoreGive(s_semph_get_ip_addrs);
        
#if CONFIG_APP_CONNECT_IPV6
        if (s_semph_get_ip6_addrs)
            xSemaphoreGive(s_semph_get_ip6_addrs);        
#endif
        EventDispatcher_PostEvent(MOD_WIFI_EVENTS, WIFI_CONNECT_FAILED_EVENT, NULL, 0, portMAX_DELAY); 
        return;
    }

    EventDispatcher_PostEvent(MOD_WIFI_EVENTS, WIFI_DISCONNECTED_EVENT, NULL, 0, portMAX_DELAY); 

    if(b_WiFi_Reconnect == true)
    {
        ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");    
        esp_err_t err = esp_wifi_connect();    

        if (err == ESP_ERR_WIFI_NOT_STARTED)
            return;
    
        ESP_ERROR_CHECK(err);
    }
}


/// @brief            WiFi event handler when connected
/// @param esp_netif  Pointer to netif object for IPV6
/// @param event_base Event base from subsystem
/// @param event_id   The received event ID
/// @param event_data The data for the event
static void mod_wifi_handler_on_wifi_connect(void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    wifi_phy_mode_t phymode;
    esp_wifi_sta_get_negotiated_phymode(&phymode);
    ESP_LOGI(TAG, "Wi-Fi Phy mode: %s", mod_wifi_phy_mode_to_str(phymode));

#if CONFIG_APP_CONNECT_IPV6
    esp_netif_create_ip6_linklocal(esp_netif);
#endif // CONFIG_EXAMPLE_CONNECT_IPV6
}


/// @brief            WiFi event handler when STA got an IP (IPV4)
/// @param arg        Additional data registered to the event
/// @param event_base Event base from subsystem
/// @param event_id   The received event ID
/// @param event_data The data for the event
static void mod_wifi_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    s_retry_num = 0;

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

    if (!mod_wifi_is_our_netif(APP_NETIF_DESC_STA, event->esp_netif)) 
        return;
    
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    
    if (s_semph_get_ip_addrs) 
        xSemaphoreGive(s_semph_get_ip_addrs);
    else 
        ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));

    EventDispatcher_PostEvent(MOD_WIFI_EVENTS, WIFI_CONNECTED_EVENT, NULL, 0, portMAX_DELAY);   

//ToDo: Test if iTWT can be enabled after data has been send and then going to sleep
#if CONFIG_APP_ITWT_ENABLE
    //mod_wifi_init_iTWT();
#endif
}


/// @brief             WiFi event handler when STA got an IP (IPV6)
/// @param arg         Additional data registered to the event
/// @param event_base  Event base from subsystem
/// @param event_id    The received event ID
/// @param event_data  The data for the event
static void mod_wifi_handler_on_sta_got_ipv6(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
#if CONFIG_APP_CONNECT_IPV6
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;

    if (!mod_wifi_is_our_netif(APP_NETIF_DESC_STA, event->esp_netif)) 
        return;
    
    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);

    ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip), ipv6_addr_types_to_str[ipv6_type]);

    if (ipv6_type == CONFIG_APP_CONNECT_PREFERRED_IPV6_TYPE) 
    {
        if (s_semph_get_ip6_addrs) 
            xSemaphoreGive(s_semph_get_ip6_addrs);
        else 
            ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(event->ip6_info.ip), ipv6_addr_types_to_str[ipv6_type]);
        
    }
#endif // CONFIG_APP_CONNECT_IPV6
}


#if CONFIG_APP_ITWT_ENABLE
/// @brief            WiFi iTWT event handler gets called when iTWT request got accepted by AP
/// @param arg        Additional data registered to the event
/// @param event_base Event base from subsystem
/// @param event_id   The received event ID
/// @param event_data The data for the event --> wifi_event_sta_itwt_setup_t data
static void mod_wifi_handler_itwt_setup(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_setup_t *setup = (wifi_event_sta_itwt_setup_t *) event_data;
    
    if (setup->config.setup_cmd == TWT_ACCEPT) 
    {
        /* TWT Wake Interval = TWT Wake Interval Mantissa * (2 ^ TWT Wake Interval Exponent) */
        ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, flow_id:%d, %s, %s, wake_dura:%d, wake_invl_e:%d, wake_invl_m:%d", setup->config.twt_id,
                setup->config.flow_id, setup->config.trigger ? "trigger-enabled" : "non-trigger-enabled", setup->config.flow_type ? "unannounced" : "announced",
                setup->config.min_wake_dura, setup->config.wake_invl_expn, setup->config.wake_invl_mant);
        ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SETUP>wake duration:%d us, service period:%d us", setup->config.min_wake_dura << 8, setup->config.wake_invl_mant << setup->config.wake_invl_expn);
    
        EventDispatcher_PostEvent(MOD_WIFI_EVENTS, WIFI_ITWT_ESTABLISHED, NULL, 0, portMAX_DELAY);   
    } 
    else     
    {
        //ToDo: Other setup.cmd replies must be handled When setting the iTWT to 10min we get a REJECTED (ID7) as response. 
        ESP_LOGE(TAG, "<WIFI_EVENT_ITWT_SETUP>twt_id:%d, unexpected setup command:%d", setup->config.twt_id, setup->config.setup_cmd);
    }
}


/// @brief             WiFi iTWT event handler gets called when iTWT teardown happened
/// @param arg         Additional data registered to the event
/// @param event_base  Event base from subsystem
/// @param event_id    The received event ID
/// @param event_data  The data for the event --> wifi_event_sta_itwt_teardown_t data
static void mod_wifi_handler_itwt_teardown(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_teardown_t *teardown = (wifi_event_sta_itwt_teardown_t *) event_data;
    ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_TEARDOWN>flow_id %d%s", teardown->flow_id, (teardown->flow_id == 8) ? "(all twt)" : "");

    EventDispatcher_PostEvent(MOD_WIFI_EVENTS, WIFI_ITWT_CLOSED, NULL, 0, portMAX_DELAY);   
}


/// @brief            WiFi iTWT event handler gets called when iTWT suspend happened
/// @param arg        Additional data registered to the event
/// @param event_base Event base from subsystem
/// @param event_id   The received event ID
/// @param event_data The data for the event --> wifi_event_sta_itwt_suspend_t data
static void mod_wifi_handler_itwt_suspend(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_suspend_t *suspend = (wifi_event_sta_itwt_suspend_t *) event_data;
    ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_SUSPEND>status:%d, flow_id_bitmap:0x%x, actual_suspend_time_ms:[%lu %lu %lu %lu %lu %lu %lu %lu]",
             suspend->status, suspend->flow_id_bitmap,
             suspend->actual_suspend_time_ms[0], suspend->actual_suspend_time_ms[1], suspend->actual_suspend_time_ms[2], suspend->actual_suspend_time_ms[3],
             suspend->actual_suspend_time_ms[4], suspend->actual_suspend_time_ms[5], suspend->actual_suspend_time_ms[6], suspend->actual_suspend_time_ms[7]);
}


/// @brief             WiFi iTWT event handler gets called when iTWT probe happened
/// @param arg         Additional data registered to the event
/// @param event_base  Event base from subsystem
/// @param event_id    The received event ID
/// @param event_data  The data for the event --> wifi_event_sta_itwt_probe_t data
static void mod_wifi_handler_itwt_probe(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    wifi_event_sta_itwt_probe_t *probe = (wifi_event_sta_itwt_probe_t *) event_data;
    ESP_LOGI(TAG, "<WIFI_EVENT_ITWT_PROBE>status:%s, reason:0x%x", mod_wifi_itwt_probe_status_to_str(probe->status), probe->reason);
}
#endif //CONFIG_APP_ITWT_ENABLE



/* Private helper functions ---------------------------------------------------------*/

/// @brief       Set a static IP address
/// @param netif Pointer to netif object
static void mod_wifi_set_static_ip(esp_netif_t *netif)
{
#if CONFIG_APP_IP_ENABLE_STATIC_IP
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    
    ip.ip.addr      = ipaddr_addr(CONFIG_APP_IP_STATIC_IP_ADDR);
    ip.gw.addr      = ipaddr_addr(CONFIG_APP_IP_STATIC_GW_ADDR);
    ip.netmask.addr = ipaddr_addr(CONFIG_APP_IP_STATIC_NETMASK_ADDR);    
    
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    
    ESP_LOGI(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", 
             CONFIG_APP_IP_STATIC_IP_ADDR, CONFIG_APP_IP_STATIC_NETMASK_ADDR, CONFIG_APP_IP_STATIC_GW_ADDR);
             
#endif
}


/// @brief        Checks the netif description if it contains specified prefix.
/// @details      All netifs created withing common connect component are prefixed with the module TAG,
/// @details      so it returns true if the specified netif is owned by this module
/// @param prefix Prefix to check against module TAG 
/// @param netif  netif to check
/// @return       ESP_OK on success
static bool mod_wifi_is_our_netif(const char *prefix, esp_netif_t *netif)
{
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}


/// @brief      Get netif from descriptor
/// @param desc Descriptor string
/// @return     ESP_OK on success
static esp_netif_t *mod_wifi_get_netif_from_desc(const char *desc)
{
    esp_netif_t *netif = NULL;

    while ((netif = esp_netif_next_unsafe(netif)) != NULL) 
    {
        if (strcmp(esp_netif_get_desc(netif), desc) == 0) 
            return netif;        
    }
    return netif;
}


/// @brief         Prints all IPs for all netifs
/// @param prefix  Module prefix
static void mod_wifi_print_all_netif_ips(const char *prefix)
{
    // iterate over active interfaces, and print out IPs of "our" netifs
    esp_netif_t *netif = NULL;

    for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) 
    {
        netif = esp_netif_next_unsafe(netif);

        if (mod_wifi_is_our_netif(prefix, netif)) 
        {
            ESP_LOGI(TAG, "Connected to %s", esp_netif_get_desc(netif));
#if CONFIG_LWIP_IPV4
            esp_netif_ip_info_t ip;
            ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip));
            ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&ip.ip));
#endif
#if CONFIG_EXAMPLE_CONNECT_IPV6
            esp_ip6_addr_t ip6[MAX_IP6_ADDRS_PER_NETIF];
            int ip6_addrs = esp_netif_get_all_ip6(netif, ip6);
            
            for (int j = 0; j < ip6_addrs; ++j) 
            {
                esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&(ip6[j]));
                ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(ip6[j]), ipv6_addr_types_to_str[ipv6_type]);
            }
#endif
        }
    }
}


/// @brief        "Translates" wifi_itwt_probe_status_t into string
/// @param status See wifi_itwt_probe_status_t 
/// @return       translated string
static const char *mod_wifi_itwt_probe_status_to_str(wifi_itwt_probe_status_t status)
{
    switch (status) {
    case ITWT_PROBE_FAIL:                 return "itwt probe fail";
    case ITWT_PROBE_SUCCESS:              return "itwt probe success";
    case ITWT_PROBE_TIMEOUT:              return "itwt probe timeout";
    case ITWT_PROBE_STA_DISCONNECTED:     return "Sta disconnected";
    default:                              return "Unknown status";
    }
}

 
/// @brief               "Translates" wifi_phy_mode_t into string
/// @param wifi_phy_mode See wifi_phy_mode_t 
/// @return              translated string
static const char *mod_wifi_phy_mode_to_str(wifi_phy_mode_t wifi_phy_mode)
{
    switch (wifi_phy_mode) 
    {        
        case WIFI_PHY_MODE_LR:   return "PHY mode for Low Rate"; 
        case WIFI_PHY_MODE_11B:  return "PHY mode for 11b";
        case WIFI_PHY_MODE_11G:  return "PHY mode for 11g";
        case WIFI_PHY_MODE_HT20: return "PHY mode for Bandwidth HT20";
        case WIFI_PHY_MODE_HT40: return "PHY mode for Bandwidth HT40";
        case WIFI_PHY_MODE_HE20: return "PHY mode for Bandwidth HE20";    
        default:                 return "Unknown wifi phy mode";
    }
}

/*****************************END OF FILE**************************************/