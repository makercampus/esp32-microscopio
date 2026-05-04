# API HTTP - Maker Campus Microscopio UDD

API mínima para control de cámara y UI web.

## Rutas

### Puerto HTTP (`80` por defecto)
- `/` - UI principal (usa `default_index`)
- `/?view=full|simple|portal` - navegación directa de vistas
- `/status` - estado JSON simplificado
- `/control?var=<key>&val=<value>` - aplica un ajuste
- `/capture` - captura JPEG

### Puerto Stream (`81` por defecto)
- `/` - stream MJPEG

## Claves válidas en `/control`
- `lamp` (0..80)
- `brightness`
- `contrast`
- `saturation`
- `special_effect` (0..6)
- `profile` (`low|medium|high` o `0|1|2`)

## Campos relevantes en `/status`
- `lamp`
- `lamp_simple_default`
- `lamp_max`
- `profile_index`
- `profile_key`
- `profile_label`
- `framesize`
- `framesize_name`
- `quality_raw`
- `quality_pct`
- `xclk`
- `brightness`
- `contrast`
- `saturation`
- `special_effect`
- `cam_name`
- `code_ver`
- `stream_url`
