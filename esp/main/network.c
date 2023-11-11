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

static char *ssid = "";
static char *password = "";

static const char *TAG = "websocket";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
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

static void websocket_app_start(void)
{
    esp_websocket_client_config_t websocket_cfg = {
        uri : "ws://192.168.2.125",
        port : 8888,
    };

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    printf("client ptr %p \n", &client);
    printf("websocket_app_start \n");
    // This call demonstrates adding another header; it's called to increase code coverage
    // esp_websocket_client_append_header(client, "HeaderNewKey", "value");

    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    printf("register events \n");

    esp_websocket_client_start(client);
    printf("start ws \n");
    char data[32];
    int i = 0;

    printf("websocket_app_start \n");

    while (i < 1)
    {
        printf("websocket_app_start while \n");
        if (esp_websocket_client_is_connected(client))
        {
            printf("websocket connected \n");
            int len = sprintf(data, "hello %04d", i++);
            ESP_LOGI(TAG, "Sending %s", data);
            esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Sending fragmented message");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    memset(data, 'a', sizeof(data));
    esp_websocket_client_send_text(client, data, sizeof(data), portMAX_DELAY);
    memset(data, 'b', sizeof(data));
    esp_websocket_client_send_bin(client, data, sizeof(data), portMAX_DELAY);
    esp_websocket_client_close(client, portMAX_DELAY);

    esp_websocket_client_destroy(client);
}

void wifi_connect()
{
    esp_log_level_set("*", ESP_LOG_DEBUG);
    nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    printf("wifi_init done \n");

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    printf("wifi_set_mode done \n");

    wifi_config_t wifi_config = {0};
    wifi_config.sta.bssid_set = false;
    strcpy((char *)wifi_config.sta.ssid, (char *)ssid);
    strcpy((char *)wifi_config.sta.password, (char *)password);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    printf("wifi_set_config done \n");

    ESP_ERROR_CHECK(esp_wifi_start());
    printf("wifi_start done \n");

    ESP_ERROR_CHECK(esp_wifi_connect());
    printf("wifi_connect done \n");

    ESP_ERROR_CHECK(esp_netif_init());

    websocket_app_start();
}