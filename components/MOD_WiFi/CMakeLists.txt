idf_component_register(SRCS "mod_wifi.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES MOD_EventDispatcher
                    REQUIRES nvs_flash esp_wifi)
