/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_websocket_client.h"

#include "wifi.h"
#include "websocket.h"
#include "taskinfo.h"

void app_main(void)
{
    setup_wifi();
    esp_websocket_client_handle_t websocket_client_handle = websocket_start();

    while (true)
    {
        cJSON *json = cJSON_CreateObject();
        read_task_info_to_json(json);
        char *json_str = cJSON_Print(json);

        esp_websocket_client_send_text(websocket_client_handle, json_str, strlen(json_str), portMAX_DELAY);

        cJSON_Delete(json);
        cJSON_free(json_str);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
