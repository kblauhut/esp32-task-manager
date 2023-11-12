#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_websocket_client.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_log.h"

#include <string.h>

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define MAX_RETRY_COUNT 5

static char *ssid = "";
static char *password = "";

static const char *TAG = "WIFI";

static int retry_count = 0;
static EventGroupHandle_t wifi_event_group;

void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    if (base != WIFI_EVENT)
        return;

    if (id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Connecting to %s", ssid);
        esp_wifi_connect();
    }

    if (id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_count < MAX_RETRY_COUNT)
        {
            ESP_LOGI(TAG, "Retrying to connect to %s", ssid);
            esp_wifi_connect();
            retry_count++;
        }
        else
        {
            ESP_LOGI(TAG, "Failed to reconnect to %s", ssid);
            xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
        }
    }
}

void ip_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    if (base != IP_EVENT)
        return;

    if (id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *got_ip_event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Ip accquired: -" IPSTR " -", IP2STR(&got_ip_event->ip_info.ip));
        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

void setup_wifi()
{
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_event_handler_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_event_handler_instance));

    esp_event_handler_instance_t ip_event_handler_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, &ip_event_handler_instance));

    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.sta.ssid, (char *)ssid);
    strcpy((char *)wifi_config.sta.password, (char *)password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_SUCCESS | WIFI_FAILURE, false, false, portMAX_DELAY);

    if (bits & WIFI_SUCCESS)
    {
        ESP_LOGI(TAG, "Connected to %s", ssid);
    }
    else if (bits & WIFI_FAILURE)
    {
        ESP_LOGI(TAG, "Failed to connect to %s", ssid);
    }
    else
    {
        ESP_LOGI(TAG, "Unexpected event");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_handler_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler_instance));
    vEventGroupDelete(wifi_event_group);
}
