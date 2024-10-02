idf_component_register(SRCS "mod_light.c"
                    INCLUDE_DIRS "."                    
                    REQUIRES DRV_I2Cdev DRV_TSL2591 MOD_Power )
