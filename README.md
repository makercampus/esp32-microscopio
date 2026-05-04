# Maker Campus Microscopio UDD


#### Créditos: Mattias Morales
---

## Cómo usar

### 0) Versiones
additional board manager: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Luego instalar esp32 by Espressif Systems versión:
> 3.3.8


### 1) Configurar nombre y contraseña del WiFi
Edita **solo** este archivo:
- `maker_config.h`

Arriba del archivo cambia estas dos líneas:

```cpp
#define MAKER_DEFAULT_AP_SSID "TEST-MAKER-CAMPUS-CAM"
#define MAKER_DEFAULT_AP_PASSWORD "1234567890"
```

### 2) Cargar firmware
Compila y sube el firmware a la ESP32-CAM.

### 3) Conectarte
- Conéctate al WiFi que configuraste.
- Abre en el navegador:
- `http://192.168.4.1/`

