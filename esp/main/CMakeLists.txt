idf_component_register(SRCS "main.c" "wifi.c" "websocket.c" "taskinfo.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_websocket_client esp_wifi nvs_flash json
                    )