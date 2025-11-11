#pragma once
#include "esp_camera.h"

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 12;
  config.pin_d1 = 13;
  config.pin_d2 = 15;
  config.pin_d3 = 14;
  config.pin_d4 = 27;
  config.pin_d5 = 25;
  config.pin_d6 = 26;
  config.pin_d7 = 17;
  config.pin_xclk = 21;
  config.pin_pclk = 22;
  config.pin_vsync = 23;
  config.pin_href = 18;
  config.pin_sscb_sda = 32;
  config.pin_sscb_scl = 33;
  config.pin_pwdn = -1;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
  } else {
    Serial.println("Camera ready");
  }
}
