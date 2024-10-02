idf_component_register(SRCS "mod_th_meas.c"
                    INCLUDE_DIRS "."
                    #PRIV_REQUIRES DRV_I2Cdev DRV_sht4x MOD_Power
                    REQUIRES DRV_I2Cdev DRV_sht4x MOD_Power )
