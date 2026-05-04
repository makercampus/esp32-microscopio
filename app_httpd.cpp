// Original Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <esp_http_server.h>
#include <esp_timer.h>
#include <esp_camera.h>

#include <Arduino.h>
#include <WiFi.h>

#include <cmath>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "index_ov2640.h"
#include "index_other.h"
#include "maker_config.h"

// Functions from main .ino
extern void flashLED(int flashtime);
extern void setLamp(int newVal);
extern void printLocalTime(bool extraData);
extern bool applyProfileByIndex(int index);
extern int profileIndexFromKey(const char *key);
extern const char *getProfileKeyByIndex(int index);
extern const char *getProfileLabelByIndex(int index);
extern int getActiveProfileIndex();
extern int clampLampPercent(int value);

// External state from main .ino
extern char myName[];
extern char myVer[];
extern char baseVersion[];
extern IPAddress ip;
extern IPAddress net;
extern IPAddress gw;
extern bool accesspoint;
extern char apName[];
extern bool captivePortal;
extern int httpPort;
extern int streamPort;
extern char httpURL[];
extern char streamURL[];
extern char default_index[];
extern int8_t streamCount;
extern unsigned long streamsServed;
extern unsigned long imagesServed;
extern int lampVal;
extern String critERR;
extern bool debugData;
extern bool haveTime;
extern int sketchSize;
extern int sketchSpace;
extern String sketchMD5;
extern bool otaEnabled;
extern char otaPassword[];
extern unsigned long xclk;
extern int sensorPID;

#define PART_BOUNDARY "123456789000000000000987654321"

static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = nullptr;
httpd_handle_t camera_httpd = nullptr;

static const char *framesizeName(framesize_t size) {
    switch (size) {
#ifdef FRAMESIZE_96X96
        case FRAMESIZE_96X96: return "THUMB (96x96)";
#endif
        case FRAMESIZE_QQVGA: return "QQVGA (160x120)";
        case FRAMESIZE_HQVGA: return "HQVGA (240x176)";
#ifdef FRAMESIZE_240X240
        case FRAMESIZE_240X240: return "240x240";
#endif
        case FRAMESIZE_QVGA: return "QVGA (320x240)";
        case FRAMESIZE_CIF: return "CIF (400x296)";
        case FRAMESIZE_HVGA: return "HVGA (480x320)";
        case FRAMESIZE_VGA: return "VGA (640x480)";
        case FRAMESIZE_SVGA: return "SVGA (800x600)";
        case FRAMESIZE_XGA: return "XGA (1024x768)";
        case FRAMESIZE_HD: return "HD (1280x720)";
        case FRAMESIZE_SXGA: return "SXGA (1280x1024)";
        case FRAMESIZE_UXGA: return "UXGA (1600x1200)";
#ifdef FRAMESIZE_FHD
        case FRAMESIZE_FHD: return "FHD (1920x1080)";
#endif
#ifdef FRAMESIZE_QXGA
        case FRAMESIZE_QXGA: return "QXGA (2048x1536)";
#endif
        default: return "Desconocida";
    }
}

static int clampBrightness(int value) {
    const int minVal = maker::isOv3660Sensor(sensorPID) ? -3 : -2;
    const int maxVal = maker::isOv3660Sensor(sensorPID) ? 3 : 2;
    return constrain(value, minVal, maxVal);
}

static int clampContrast(int value) {
    const int minVal = maker::isOv3660Sensor(sensorPID) ? -3 : -2;
    const int maxVal = maker::isOv3660Sensor(sensorPID) ? 3 : 2;
    return constrain(value, minVal, maxVal);
}

static int clampSaturation(int value) {
    const int minVal = maker::isOv3660Sensor(sensorPID) ? -4 : -2;
    const int maxVal = maker::isOv3660Sensor(sensorPID) ? 4 : 2;
    return constrain(value, minVal, maxVal);
}

void serialDump() {
    Serial.println();
    Serial.printf("Name: %s\r\n", myName);
    if (haveTime) {
        Serial.print("Time: ");
        printLocalTime(true);
    }
    Serial.printf("Firmware: %s (base: %s)\r\n", myVer, baseVersion);
    const float sketchPct = 100.0f * sketchSize / sketchSpace;
    Serial.printf("Sketch Size: %i (total: %i, %.1f%% used)\r\n", sketchSize, sketchSpace, sketchPct);
    Serial.printf("MD5: %s\r\n", sketchMD5.c_str());
    Serial.printf("ESP sdk: %s\r\n", ESP.getSdkVersion());
    if (otaEnabled) {
        if (strlen(otaPassword) > 0) {
            Serial.printf("OTA: Enabled, Password: %s\r\n", otaPassword);
        } else {
            Serial.printf("OTA: Enabled, No Password\r\n");
        }
    } else {
        Serial.printf("OTA: Disabled\r\n");
    }

    if (accesspoint) {
        Serial.printf("WiFi Mode: AccessPoint%s\r\n", captivePortal ? " with captive portal" : "");
        Serial.printf("WiFi SSID: %s\r\n", apName);
    } else {
        Serial.printf("WiFi Mode: Client\r\n");
        Serial.printf("WiFi SSID: %s\r\n", WiFi.SSID().c_str());
        Serial.printf("WiFi RSSI: %i\r\n", WiFi.RSSI());
    }
    Serial.printf("WiFi IP address: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    if (!accesspoint) {
        Serial.printf("WiFi Netmask: %d.%d.%d.%d\r\n", net[0], net[1], net[2], net[3]);
        Serial.printf("WiFi Gateway: %d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
    }
    Serial.printf("WiFi Http port: %i, Stream port: %i\r\n", httpPort, streamPort);

    const int64_t sec = esp_timer_get_time() / 1000000;
    const int64_t upDays = static_cast<int64_t>(floor(sec / 86400));
    const int upHours = static_cast<int64_t>(floor(sec / 3600)) % 24;
    const int upMin = static_cast<int64_t>(floor(sec / 60)) % 60;
    const int upSec = sec % 60;

    Serial.printf("System up: %" PRId64 ":%02i:%02i:%02i (d:h:m:s)\r\n", upDays, upHours, upMin, upSec);
    Serial.printf("Active streams: %i, Previous streams: %lu, Images captured: %lu\r\n", streamCount, streamsServed, imagesServed);
    Serial.printf("Xclk Freq: %lu MHz, Lamp: %d%%\r\n", xclk, lampVal);
    Serial.printf("Perfil activo: %s\r\n", getProfileLabelByIndex(getActiveProfileIndex()));

    if (critERR.length() > 0) {
        Serial.printf("\r\nError crítico de cámara: %s\r\n", critERR.c_str());
    }
    Serial.println();
}

static esp_err_t capture_handler(httpd_req_t *req) {
    flashLED(50);

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("CAPTURE: failed to acquire frame");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    esp_err_t res = ESP_OK;
    if (fb->format == PIXFORMAT_JPEG) {
        res = httpd_resp_send(req, reinterpret_cast<const char *>(fb->buf), fb->len);
    } else {
        Serial.println("Capture Error: Non-JPEG image returned by camera module");
        res = ESP_FAIL;
    }

    esp_camera_fb_return(fb);
    imagesServed++;
    return res;
}

static esp_err_t stream_handler(httpd_req_t *req) {
    flashLED(75);
    Serial.println("Stream requested");
    streamCount = 1;

    esp_err_t res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        streamCount = 0;
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));

    while (res == ESP_OK) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("STREAM: failed to acquire frame");
            res = ESP_FAIL;
            break;
        }

        if (fb->format != PIXFORMAT_JPEG) {
            Serial.println("STREAM: Non-JPEG frame returned by camera module");
            esp_camera_fb_return(fb);
            res = ESP_FAIL;
            break;
        }

        char partBuf[64];
        const size_t hlen = snprintf(partBuf, sizeof(partBuf), STREAM_PART, fb->len);
        res = httpd_resp_send_chunk(req, partBuf, hlen);
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, reinterpret_cast<const char *>(fb->buf), fb->len);
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
        }

        if (debugData) {
            Serial.printf("MJPG: %uB\r\n", static_cast<uint32_t>(fb->len));
        }

        esp_camera_fb_return(fb);
    }

    streamCount = 0;
    streamsServed++;
    Serial.println("Stream ended");
    return res;
}

static esp_err_t cmd_handler(httpd_req_t *req) {
    char variable[32] = {0};
    char value[32] = {0};

    const size_t bufLen = httpd_req_get_url_query_len(req) + 1;
    if (bufLen <= 1) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char *buf = static_cast<char *>(malloc(bufLen));
    if (!buf) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    if (httpd_req_get_url_query_str(req, buf, bufLen) != ESP_OK ||
        httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK ||
        httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    if (critERR.length() > 0) {
        return httpd_resp_send_500(req);
    }

    sensor_t *s = esp_camera_sensor_get();
    int res = 0;

    if (!strcmp(variable, "lamp")) {
        lampVal = clampLampPercent(atoi(value));
        setLamp(lampVal);
    } else if (!strcmp(variable, "brightness")) {
        res = s->set_brightness(s, clampBrightness(atoi(value)));
    } else if (!strcmp(variable, "contrast")) {
        res = s->set_contrast(s, clampContrast(atoi(value)));
    } else if (!strcmp(variable, "saturation")) {
        res = s->set_saturation(s, clampSaturation(atoi(value)));
    } else if (!strcmp(variable, "special_effect")) {
        res = s->set_special_effect(s, constrain(atoi(value), 0, 6));
    } else if (!strcmp(variable, "profile")) {
        int index = profileIndexFromKey(value);
        if (index < 0) {
            bool numericValue = strlen(value) > 0;
            for (size_t i = 0; i < strlen(value); i++) {
                if ((value[i] < '0' || value[i] > '9') && !(i == 0 && value[i] == '-')) {
                    numericValue = false;
                    break;
                }
            }
            if (numericValue) index = atoi(value);
        }
        if (index < 0 || !applyProfileByIndex(index)) {
            res = -1;
        }
    } else {
        res = -1;
    }

    if (res != 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, nullptr, 0);
}

static esp_err_t status_handler(httpd_req_t *req) {
    static char jsonResponse[1024];
    char *p = jsonResponse;

    *p++ = '{';

    if (critERR.length() == 0) {
        sensor_t *s = esp_camera_sensor_get();
        const int profileIndex = getActiveProfileIndex();
        const int qualityPct = maker::sensorRawToQualityPercent(s->status.quality, sensorPID);

        p += sprintf(p, "\"lamp\":%d,", lampVal);
        p += sprintf(p, "\"lamp_simple_default\":%d,", maker::kLampSimpleDefaultPercent);
        p += sprintf(p, "\"lamp_max\":%d,", maker::kLampMaxPercent);
        p += sprintf(p, "\"profile_index\":%d,", profileIndex);
        p += sprintf(p, "\"profile_key\":\"%s\",", getProfileKeyByIndex(profileIndex));
        p += sprintf(p, "\"profile_label\":\"%s\",", getProfileLabelByIndex(profileIndex));
        p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
        p += sprintf(p, "\"framesize_name\":\"%s\",", framesizeName(static_cast<framesize_t>(s->status.framesize)));
        p += sprintf(p, "\"quality_raw\":%u,", s->status.quality);
        p += sprintf(p, "\"quality_pct\":%d,", qualityPct);
        p += sprintf(p, "\"xclk\":%u,", static_cast<unsigned int>(xclk));
        p += sprintf(p, "\"brightness\":%d,", s->status.brightness);
        p += sprintf(p, "\"contrast\":%d,", s->status.contrast);
        p += sprintf(p, "\"saturation\":%d,", s->status.saturation);
        p += sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
        p += sprintf(p, "\"cam_name\":\"%s\",", myName);
        p += sprintf(p, "\"code_ver\":\"%s\",", myVer);
        p += sprintf(p, "\"stream_url\":\"%s\"", streamURL);
    } else {
        p += sprintf(p, "\"error\":\"camera_init\"");
    }

    *p++ = '}';
    *p++ = 0;

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, jsonResponse, strlen(jsonResponse));
}

static esp_err_t error_handler(httpd_req_t *req) {
    std::string html(error_html);
    size_t index = 0;

    while ((index = html.find("<CAMNAME>")) != std::string::npos) {
        html.replace(index, strlen("<CAMNAME>"), myName);
    }
    while ((index = html.find("<ERRORTEXT>")) != std::string::npos) {
        html.replace(index, strlen("<ERRORTEXT>"), critERR.c_str());
    }

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    return httpd_resp_send(req, html.c_str(), html.length());
}

static esp_err_t index_handler(httpd_req_t *req) {
    char view[32] = {0};

    const size_t bufLen = httpd_req_get_url_query_len(req) + 1;
    if (bufLen > 1) {
        char *buf = static_cast<char *>(malloc(bufLen));
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        const bool ok =
            httpd_req_get_url_query_str(req, buf, bufLen) == ESP_OK &&
            httpd_query_key_value(buf, "view", view, sizeof(view)) == ESP_OK;
        free(buf);

        if (!ok) {
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
    } else {
        strcpy(view, default_index);
        if (captivePortal) {
            strcpy(view, "portal");
        }
    }

    if (!strncmp(view, "simple", sizeof(view))) {
        if (critERR.length() > 0) {
            return error_handler(req);
        }
        httpd_resp_set_type(req, "text/html");
        return httpd_resp_send(req, reinterpret_cast<const char *>(index_simple_html), index_simple_html_len);
    }

    if (!strncmp(view, "full", sizeof(view))) {
        if (critERR.length() > 0) {
            return error_handler(req);
        }
        httpd_resp_set_type(req, "text/html");
        return httpd_resp_send(req, reinterpret_cast<const char *>(index_ov2640_html), index_ov2640_html_len);
    }

    if (!strncmp(view, "portal", sizeof(view))) {
        std::string html(portal_html);
        size_t index = 0;

        while ((index = html.find("<APPURL>")) != std::string::npos) {
            html.replace(index, strlen("<APPURL>"), httpURL);
        }
        while ((index = html.find("<STREAMURL>")) != std::string::npos) {
            html.replace(index, strlen("<STREAMURL>"), streamURL);
        }
        while ((index = html.find("<CAMNAME>")) != std::string::npos) {
            html.replace(index, strlen("<CAMNAME>"), myName);
        }

        httpd_resp_set_type(req, "text/html");
        return httpd_resp_send(req, html.c_str(), html.length());
    }

    httpd_resp_send_404(req);
    return ESP_FAIL;
}

void startCameraServer(int hPort, int sPort) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = nullptr,
    };

    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = nullptr,
    };

    httpd_uri_t cmd_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler,
        .user_ctx = nullptr,
    };

    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = capture_handler,
        .user_ctx = nullptr,
    };

    httpd_uri_t stream_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = nullptr,
    };

    config.server_port = hPort;
    config.ctrl_port = hPort;

    Serial.printf("Starting web server on port: '%d'\r\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }

    config.server_port = sPort;
    config.ctrl_port = sPort;

    Serial.printf("Starting stream server on port: '%d'\r\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
