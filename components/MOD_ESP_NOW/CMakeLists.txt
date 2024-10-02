idf_component_register(
    SRCS "mod_esp_now.c"
    INCLUDE_DIRS .
    PRIV_REQUIRES MOD_EventDispatcher MOD_TH_Meas
	REQUIRES esp_wifi
)
