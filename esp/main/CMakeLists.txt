idf_component_register(SRCS "main.c" "network.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_websocket_client esp_wifi nvs_flash
                    )