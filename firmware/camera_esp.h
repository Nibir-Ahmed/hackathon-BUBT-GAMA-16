// camera_esp.h — ESP32-S3-CAM OV2640 Initialization & Image Capture Helper

#ifndef CAMERA_ESP_H
#define CAMERA_ESP_H

#include <Arduino.h>
#include "esp_camera.h"
#include "esp_log.h"

// ----------------------------------------------------------------------------
// Pin Map for Standard ESP32-S3-CAM Boards (Freenove / Ai-Thinker S3)
// ----------------------------------------------------------------------------
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

class CameraController {
private:
    bool initialized;

public:
    CameraController() : initialized(false) {}

    bool begin() {
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;
        config.pin_d0 = CAM_PIN_D0;
        config.pin_d1 = CAM_PIN_D1;
        config.pin_d2 = CAM_PIN_D2;
        config.pin_d3 = CAM_PIN_D3;
        config.pin_d4 = CAM_PIN_D4;
        config.pin_d5 = CAM_PIN_D5;
        config.pin_d6 = CAM_PIN_D6;
        config.pin_d7 = CAM_PIN_D7;
        config.pin_xclk = CAM_PIN_XCLK;
        config.pin_pclk = CAM_PIN_PCLK;
        config.pin_vsync = CAM_PIN_VSYNC;
        config.pin_href = CAM_PIN_HREF;
        config.pin_sccb_sda = CAM_PIN_SIOD;
        config.pin_sccb_scl = CAM_PIN_SIOC;
        config.pin_pwdn = CAM_PIN_PWDN;
        config.pin_reset = CAM_PIN_RESET;
        config.xclk_freq_hz = 20000000;
        config.pixel_format = PIXFORMAT_JPEG;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

        // Use appropriate frame size and buffer based on PSRAM
        if (psramFound()) {
            config.frame_size = FRAMESIZE_VGA; // 640x480
            config.jpeg_quality = 12;
            config.fb_count = 2;
            config.fb_location = CAMERA_FB_IN_PSRAM;
            Serial.println("[CAMERA] PSRAM detected — using VGA frame size.");
        } else {
            config.frame_size = FRAMESIZE_QVGA; // 320x240
            config.jpeg_quality = 14;
            config.fb_count = 1;
            config.fb_location = CAMERA_FB_IN_DRAM;
            Serial.println("[CAMERA] No PSRAM — using QVGA frame size.");
        }

        esp_err_t err = esp_camera_init(&config);
        if (err != ESP_OK) {
            Serial.printf("[CAMERA] Camera init failed with error 0x%x\n", err);
            initialized = false;
            return false;
        }

        // Silence frame buffer overflow warnings during idle periods
        esp_log_level_set("cam_hal", ESP_LOG_NONE);

        // Adjust sensor parameters for classroom lighting
        sensor_t* s = esp_camera_sensor_get();
        if (s != nullptr) {
            s->set_brightness(s, 1);
            s->set_contrast(s, 1);
            s->set_saturation(s, 0);
            s->set_vflip(s, 1); // Flip if inverted
            s->set_hmirror(s, 0);
        }

        initialized = true;
        Serial.println("[CAMERA] OV2640 camera successfully initialized.");
        return true;
    }

    bool isReady() const {
        return initialized;
    }

    camera_fb_t* capture() {
        if (!initialized) {
            if (!begin()) return nullptr;
        }
        return esp_camera_fb_get();
    }

    void release(camera_fb_t* fb) {
        if (fb != nullptr) {
            esp_camera_fb_return(fb);
        }
    }
};

extern CameraController camera;

#endif // CAMERA_ESP_H

