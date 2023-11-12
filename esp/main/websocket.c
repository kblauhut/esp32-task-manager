
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_websocket_client.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "websocket";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void websocket_test_task()
{
    while (true)
    {
        volatile int o = 0;
        for (volatile int i = 0; i < 1000000; i++)
        {
            o = i; // Just so the compiler doesn't optimize the loop away
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
        }
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        if (data->op_code == 0x08 && data->data_len == 2)
        {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        }
        else
        {
            ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);

            if ((data->data_len > 0) && strncmp("add_thread", (char *)data->data_ptr, data->data_len) == 0)
            {
                ESP_LOGI(TAG, "Adding thread");
                char thread_name[64];
                sprintf(thread_name, "thread_%ld", xTaskGetTickCount());
                xTaskCreate(&websocket_test_task, thread_name, 2048, NULL, 5, NULL);
            }
        }

        // If received data contains json structure it succeed to parse
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        log_error_if_nonzero("HTTP status code", data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", data->error_handle.esp_transport_sock_errno);
        }
        break;
    }
}

esp_websocket_client_handle_t websocket_start()
{
    esp_websocket_client_config_t websocket_cfg = {
        uri : "ws://192.168.2.125",
        port : 3010,
    };

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t websocket_client_handle = esp_websocket_client_init(&websocket_cfg);
    ESP_ERROR_CHECK(esp_websocket_register_events(websocket_client_handle, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)websocket_client_handle));
    ESP_ERROR_CHECK(esp_websocket_client_start(websocket_client_handle));

    int i = 0;
    while (!esp_websocket_client_is_connected(websocket_client_handle))
    {
        ESP_LOGI(TAG, "Trying websocket connection... attempt: %d", i++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Websocket connected!");

    // ESP_ERROR_CHECK(esp_websocket_client_close(websocket_client_handle, portMAX_DELAY));
    // ESP_ERROR_CHECK(esp_websocket_client_destroy(websocket_client_handle));
    return websocket_client_handle;
}