idf_component_register( SRCS mod_backend.c
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES MOD_EventDispatcher
                        REQUIRES mqtt nvs_flash)
