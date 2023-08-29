#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "nvs_flash.h"
#include "soc/gpio_num.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_websocket_client.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project
   configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define WIFI_SSID "Tp-Link"
#define WIFI_PASS "abrar786"
#define WIFI_MAX_TRY 10

#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about
 * two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */

#define WIFI_CONNECTED_BIT BIT0 // set if we are connected
#define WIFI_FAIL_BIT BIT1      // set if we failed to connect

static const char *TAG = "wifi station";

static int s_retry_num = 0;

char *websocket_uri;
void connect_websockets(void) {
  sprintf(websocket_uri, "wss://ws-%s.pusher.com:443/app/%s", getenv("CLUSTER"),getenv("KEY"));

  const esp_websocket_client_config_t ws_cfg = {
      .uri = websocket_uri,
  };
}
// event handler function
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < WIFI_MAX_TRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG, "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
#define LED 2
void blink(void) {
  gpio_reset_pin(LED);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  while (true) {
    gpio_set_level(LED, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // non-blocking delay 1 s
    gpio_set_level(LED, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// initialize wifi client mode
void wifi_init_sta(void) {
  // created event group
  s_wifi_event_group = xEventGroupCreate();

  // initailize network interface
  ESP_ERROR_CHECK(esp_netif_init());

  // creates  an event loop that lsitens for different evets
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  // creates a wifi client with default config
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // default wifi config
  ESP_ERROR_CHECK(esp_wifi_init(
      &cfg)); // start wifi driver with default config and check for errors

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  // actual wifi configuration
  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = WIFI_SSID,
              .password = WIFI_PASS,
              .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
              .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
              .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or
   * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). The
   * bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
    blink();
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID,
             WIFI_PASS);
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
  }
}

void app_main(void) {
  // Initialize NVS (non-volatile storage) to store wifi config
  esp_err_t ret = nvs_flash_init(); // init nvs and retrieve status
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase()); // if error then erase flash and retry
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret); // double checking error handling

  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  wifi_init_sta();
}
