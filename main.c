#include "nvs_flash.h"
#include "esp_websocket_client.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define TAG "main"


char *websocket_uri;
void connect_websockets(void) {
  sprintf(websocket_uri, "wss://ws-%s.pusher.com:443/app/%s", getenv("CLUSTER"),getenv("KEY"));

  const esp_websocket_client_config_t ws_cfg = {
      .uri = websocket_uri,
  };
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
}
