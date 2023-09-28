#include "driver/gpio.h"
#include "esp_log.h"
// #include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "wifi_connection.h"
#include <stdio.h>

static const char *TAG = "main";
#define LED 2
char websocket_uri[100];

void connect_websockets(void) {
  sprintf(websocket_uri, "wss://ws-%s.pusher.com:443/app/%s", getenv("CLUSTER"),
          getenv("KEY"));
  ESP_LOGW(TAG, "%s", websocket_uri);
}

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

void app_main(void) {
  esp_log_level_set(TAG, ESP_LOG_INFO);
  // Initialize NVS (non-volatile storage) to store wifi config
  esp_err_t ret = nvs_flash_init(); // init nvs and retrieve status
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase()); // if error then erase flash and retry
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret); // double checking error handling

  wifi_init_sta();
  ESP_LOGI(TAG, "connected succesfully");
  ESP_LOGI(TAG, "the websocket uri is :%s", "haseeb khalid");
  connect_websockets();
  blink();
}
