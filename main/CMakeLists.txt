idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES MOD_WiFi MOD_Backend MOD_EventDispatcher MOD_Power MOD_TH_Meas MOD_Light MOD_ESP_NOW DRV_I2Cdev
                    REQUIRES esp_pm )
