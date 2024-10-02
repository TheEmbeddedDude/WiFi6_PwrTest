idf_component_register(
    SRCS app_events.c mod_eventDispatcher.c
    INCLUDE_DIRS .
    #PRIV_REQUIRES 
	REQUIRES esp_event
)
