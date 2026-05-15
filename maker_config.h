#pragma once

#include <Arduino.h>
#include <esp_camera.h>
#include <cmath>

// cambiar aquí nombre wi-fi y contraseña
#define MAKER_DEFAULT_AP_SSID "12-MAKER-CAMPUS-CAM"
#define MAKER_DEFAULT_AP_PASSWORD "1234567890"

#define MAKER_BRAND_NAME "Maker Campus Microscopio UDD"
#define MAKER_PORTAL_TITLE "Maker Campus - Universidad del Desarrollo"
#define MAKER_DEFAULT_MDNS_NAME "maker-campus-cam"

namespace maker {

struct CameraProfileSetting {
    const char *key;
    const char *label;
    framesize_t frameSize;
    uint8_t qualityPercent;
    uint8_t xclkMHz;
};

constexpr char kBrandName[] = MAKER_BRAND_NAME;
constexpr char kPortalTitle[] = MAKER_PORTAL_TITLE;
constexpr char kDefaultApSsid[] = MAKER_DEFAULT_AP_SSID;
constexpr char kDefaultApPassword[] = MAKER_DEFAULT_AP_PASSWORD;
constexpr char kDefaultMdnsName[] = MAKER_DEFAULT_MDNS_NAME;

constexpr int kLampSimpleDefaultPercent = 25;
constexpr int kLampMaxPercent = 80;

constexpr CameraProfileSetting kProfiles[] = {
    {"low", "Bajo", FRAMESIZE_HQVGA, 60, 5},
    {"medium", "Mediano", FRAMESIZE_VGA, 60, 6},
    {"high", "Alto", FRAMESIZE_XGA, 65, 7},
};

constexpr size_t kProfileCount = sizeof(kProfiles) / sizeof(kProfiles[0]);
constexpr uint8_t kDefaultProfileIndex = 1;  

inline bool isOv3660Sensor(int sensorPid) {
    return sensorPid == OV3660_PID;
}

inline int qualityPercentToSensorRaw(int qualityPercent, int sensorPid) {
    const int minRaw = isOv3660Sensor(sensorPid) ? 4 : 6;
    const int maxRaw = 63;
    const int pct = constrain(qualityPercent, 0, 100);
    const float ratio = static_cast<float>(pct) / 100.0f;
    const int raw = static_cast<int>(roundf(maxRaw - ratio * (maxRaw - minRaw)));
    return constrain(raw, minRaw, maxRaw);
}

inline int sensorRawToQualityPercent(int rawQuality, int sensorPid) {
    const int minRaw = isOv3660Sensor(sensorPid) ? 4 : 6;
    const int maxRaw = 63;
    const int raw = constrain(rawQuality, minRaw, maxRaw);
    const float ratio = static_cast<float>(maxRaw - raw) / static_cast<float>(maxRaw - minRaw);
    const int pct = static_cast<int>(roundf(ratio * 100.0f));
    return constrain(pct, 0, 100);
}

}  
