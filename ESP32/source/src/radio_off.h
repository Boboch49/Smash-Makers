#pragma once
#include <esp_wifi.h>
#include <esp_bt.h>
#if defined(CONFIG_FREERTOS_UNICORE) && CONFIG_FREERTOS_UNICORE
#error This firmware expects the dual-core ESP32-S3
#endif
#if ARDUINO_RUNNING_CORE != 1
#error Display and UART loop must run on core 1
#endif
// No WiFi/Bluetooth start call and no network library initialization.
// WIFI_NOT_INIT is the normal cold-boot state, not a reason to start the radio.
static bool disableRadios(){
 const esp_err_t stopped=esp_wifi_stop();
 bool wifiOff=stopped==ESP_OK||stopped==ESP_ERR_WIFI_NOT_INIT||stopped==ESP_ERR_WIFI_NOT_STARTED;
 if(stopped!=ESP_ERR_WIFI_NOT_INIT && wifiOff){
  const esp_err_t deinit=esp_wifi_deinit();
  wifiOff=deinit==ESP_OK||deinit==ESP_ERR_WIFI_NOT_INIT;
 }
 esp_bt_controller_status_t status=esp_bt_controller_get_status();
 if(status==ESP_BT_CONTROLLER_STATUS_ENABLED){
  if(esp_bt_controller_disable()!=ESP_OK)return false;
  status=esp_bt_controller_get_status();
 }
 if(status==ESP_BT_CONTROLLER_STATUS_INITED){
  if(esp_bt_controller_deinit()!=ESP_OK)return false;
 }
 const bool btOff=esp_bt_controller_get_status()==ESP_BT_CONTROLLER_STATUS_IDLE;
 if(btOff){
  // Arduino may have released this memory already. Both states are acceptable.
  // Memory becomes available to the HUB75 DMA buffers until the next reset.
  (void)esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
 }
 return wifiOff&&btOff;
}
