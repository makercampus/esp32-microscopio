/*
 * Vista full (usada para cualquier sensor soportado)
 */

const uint8_t index_ov2640_html[] = R"=====(<!doctype html>
<html lang="es">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Maker Campus | Vista Full</title>
    <style>
      :root {
        --bg: #111827;
        --panel: #1f2937;
        --panel-soft: #243143;
        --text: #f8fafc;
        --muted: #cbd5e1;
        --accent: #0ea5a8;
        --accent-2: #22d3ee;
        --danger: #ef4444;
      }

      * { box-sizing: border-box; }

      body {
        margin: 0;
        font-family: "Segoe UI", Tahoma, sans-serif;
        background: radial-gradient(circle at top, #1b2d45 0%, var(--bg) 55%);
        color: var(--text);
      }

      .layout {
        max-width: 1240px;
        margin: 0 auto;
        padding: 16px;
        display: grid;
        grid-template-columns: 1fr;
        gap: 16px;
      }

      .card {
        border-radius: 12px;
        background: linear-gradient(180deg, var(--panel) 0%, #17212e 100%);
        border: 1px solid #2f3f52;
        box-shadow: 0 12px 30px rgba(0,0,0,0.25);
      }

      .header {
        padding: 14px 16px;
        display: flex;
        flex-wrap: wrap;
        gap: 8px;
        align-items: center;
        justify-content: space-between;
      }

      .header h1 {
        margin: 0;
        font-size: 1rem;
        letter-spacing: 0.2px;
      }

      .meta {
        font-size: 0.84rem;
        color: var(--muted);
      }

      .viewer {
        padding: 0 16px 16px;
      }

      .viewer img {
        width: 100%;
        max-height: 70vh;
        object-fit: contain;
        background: #0b111a;
        border: 1px solid #314255;
        border-radius: 12px;
      }

      .toolbar {
        margin-top: 10px;
        display: flex;
        flex-wrap: wrap;
        gap: 8px;
      }

      .controls {
        padding: 16px;
        display: grid;
        grid-template-columns: 1fr;
        gap: 12px;
      }

      .group {
        background: var(--panel-soft);
        border: 1px solid #32465f;
        border-radius: 10px;
        padding: 12px;
      }

      .group h2 {
        margin: 0 0 10px;
        font-size: 0.95rem;
      }

      .row {
        display: grid;
        grid-template-columns: 110px 1fr auto;
        align-items: center;
        gap: 8px;
        margin: 8px 0;
      }

      .row label {
        font-size: 0.9rem;
      }

      .badge {
        font-size: 0.8rem;
        color: var(--muted);
      }

      button {
        border: 0;
        border-radius: 9px;
        color: #fff;
        background: #2f4d68;
        padding: 9px 12px;
        cursor: pointer;
        font-weight: 600;
      }

      button:hover { filter: brightness(1.08); }
      button:active { transform: translateY(1px); }

      .accent { background: linear-gradient(90deg, var(--accent), var(--accent-2)); color: #06252e; }
      .danger { background: var(--danger); }
      .ghost { background: #314155; }
      .active { outline: 2px solid #67e8f9; }

      input[type="range"], select {
        width: 100%;
      }

      @media (min-width: 980px) {
        .layout {
          grid-template-columns: minmax(520px, 1fr) 360px;
          align-items: start;
        }
      }
    </style>
  </head>

  <body>
    <main class="layout">
      <section class="card">
        <div class="header">
          <div>
            <h1 id="cam-name">Maker Campus Microscopio UDD</h1>
            <div class="meta" id="fw"></div>
          </div>
          <div class="toolbar">
            <button id="btn-simple" class="ghost">Vista Simple</button>
            <button id="btn-stream" class="accent">Iniciar Stream</button>
            <button id="btn-still">Captura Still</button>
          </div>
        </div>
        <div class="viewer">
          <img id="stream" alt="Video de cámara">
        </div>
      </section>

      <aside class="card controls">
        <section class="group">
          <h2>Perfiles</h2>
          <div class="toolbar">
            <button id="profile-low">Bajo</button>
            <button id="profile-medium">Mediano</button>
            <button id="profile-high">Alto</button>
          </div>
        </section>

        <section class="group">
          <h2>Iluminación</h2>
          <div class="row">
            <label for="lamp">Flashlight</label>
            <input id="lamp" type="range" min="0" max="80" step="1" value="0">
            <span class="badge" id="lamp-value">0%</span>
          </div>
        </section>

        <section class="group">
          <h2>Imagen</h2>
          <div class="row">
            <label for="brightness">Brillo</label>
            <input id="brightness" type="range" min="-3" max="3" step="1" value="0">
            <span class="badge" id="brightness-value">0</span>
          </div>
          <div class="row">
            <label for="contrast">Contraste</label>
            <input id="contrast" type="range" min="-3" max="3" step="1" value="0">
            <span class="badge" id="contrast-value">0</span>
          </div>
          <div class="row">
            <label for="saturation">Saturación</label>
            <input id="saturation" type="range" min="-4" max="4" step="1" value="0">
            <span class="badge" id="saturation-value">0</span>
          </div>
          <div class="row">
            <label for="special_effect">Filtro</label>
            <select id="special_effect">
              <option value="0">Sin filtro</option>
              <option value="1">Negativo</option>
              <option value="2">Escala de grises</option>
              <option value="3">Tinte rojo</option>
              <option value="4">Tinte verde</option>
              <option value="5">Tinte azul</option>
              <option value="6">Sepia</option>
            </select>
            <span></span>
          </div>
        </section>
      </aside>
    </main>

    <script>
      document.addEventListener('DOMContentLoaded', async () => {
        const byId = (id) => document.getElementById(id);
        const setText = (el, value) => {
          if (el) el.textContent = value;
        };

        const baseHost = document.location.origin;
        const streamImg = byId('stream');
        const btnStream = byId('btn-stream');
        const btnStill = byId('btn-still');
        const btnSimple = byId('btn-simple');

        const lamp = byId('lamp');
        const brightness = byId('brightness');
        const contrast = byId('contrast');
        const saturation = byId('saturation');
        const specialEffect = byId('special_effect');

        const lampValue = byId('lamp-value');
        const brightnessValue = byId('brightness-value');
        const contrastValue = byId('contrast-value');
        const saturationValue = byId('saturation-value');
        const lampMaxLabel = byId('lamp-max');
        const camNameEl = byId('cam-name');
        const profileInfo = byId('profile-info');

        const profileButtons = {
          low: byId('profile-low'),
          medium: byId('profile-medium'),
          high: byId('profile-high')
        };

        if (!streamImg) {
          console.error('Vista full: elemento #stream no encontrado.');
          return;
        }

        const setControl = async (name, value) => {
          const query = `${baseHost}/control?var=${encodeURIComponent(name)}&val=${encodeURIComponent(value)}`;
          await fetch(query);
        };

        let streamUrl = '';
        let streamEnabled = false;

        const stopStream = () => {
          window.stop();
          streamImg.removeAttribute('src');
          streamEnabled = false;
          setText(btnStream, 'Iniciar Stream');
        };

        const startStream = () => {
          if (!streamUrl) return;
          streamImg.src = streamUrl;
          streamEnabled = true;
          setText(btnStream, 'Detener Stream');
        };

        const markProfile = (profileKey, profileLabel, frameSize, qualityPct, xclk) => {
          Object.values(profileButtons).forEach(btn => {
            if (btn) btn.classList.remove('active');
          });
          if (profileButtons[profileKey]) {
            profileButtons[profileKey].classList.add('active');
          }
          setText(profileInfo, `Perfil activo: ${profileLabel} | Resolución ${frameSize} | Calidad ${qualityPct}% | XCLK ${xclk} MHz`);
        };

        try {
          const state = await fetch(`${baseHost}/status`).then(r => r.json());
          const camName = state.cam_name || 'Maker Campus Microscopio UDD';

          setText(camNameEl, camName);
          document.title = `${camName} | Vista Full`;
          streamUrl = state.stream_url || '';

          if (lamp) {
            lamp.max = state.lamp_max ?? 80;
            lamp.value = state.lamp ?? 0;
          }
          if (brightness) brightness.value = state.brightness ?? 0;
          if (contrast) contrast.value = state.contrast ?? 0;
          if (saturation) saturation.value = state.saturation ?? 0;
          if (specialEffect && state.special_effect != null) specialEffect.value = String(state.special_effect);

          setText(lampMaxLabel, state.lamp_max ?? 80);
          setText(lampValue, `${state.lamp ?? 0}%`);
          setText(brightnessValue, String(state.brightness ?? 0));
          setText(contrastValue, String(state.contrast ?? 0));
          setText(saturationValue, String(state.saturation ?? 0));

          markProfile(
            state.profile_key,
            state.profile_label,
            state.framesize_name,
            state.quality_pct,
            state.xclk
          );

          startStream();
        } catch (error) {
          console.error('Error iniciando vista full:', error);
        }

        if (btnStream) {
          btnStream.addEventListener('click', () => {
            if (streamEnabled) stopStream();
            else startStream();
          });
        }

        if (btnStill) {
          btnStill.addEventListener('click', () => {
            stopStream();
            streamImg.src = `${baseHost}/capture?_cb=${Date.now()}`;
          });
        }

        if (btnSimple) {
          btnSimple.addEventListener('click', () => {
            window.open('/?view=simple', '_self');
          });
        }

        if (lamp) {
          lamp.addEventListener('input', () => {
            setText(lampValue, `${lamp.value}%`);
          });
          lamp.addEventListener('change', async () => {
            await setControl('lamp', lamp.value);
          });
        }

        if (brightness) {
          brightness.addEventListener('input', () => setText(brightnessValue, brightness.value));
          brightness.addEventListener('change', async () => setControl('brightness', brightness.value));
        }
        if (contrast) {
          contrast.addEventListener('input', () => setText(contrastValue, contrast.value));
          contrast.addEventListener('change', async () => setControl('contrast', contrast.value));
        }
        if (saturation) {
          saturation.addEventListener('input', () => setText(saturationValue, saturation.value));
          saturation.addEventListener('change', async () => setControl('saturation', saturation.value));
        }
        if (specialEffect) {
          specialEffect.addEventListener('change', async () => setControl('special_effect', specialEffect.value));
        }

        for (const [profileKey, button] of Object.entries(profileButtons)) {
          if (!button) continue;
          button.addEventListener('click', async () => {
            await setControl('profile', profileKey);
            const current = await fetch(`${baseHost}/status`).then(r => r.json());
            if (lamp && current.lamp != null) lamp.value = current.lamp;
            setText(lampValue, `${current.lamp ?? 0}%`);
            markProfile(current.profile_key, current.profile_label, current.framesize_name, current.quality_pct, current.xclk);
            if (!streamEnabled) {
              streamImg.src = `${baseHost}/capture?_cb=${Date.now()}`;
            }
          });
        }
      });
    </script>
  </body>
</html>)=====";

size_t index_ov2640_html_len = sizeof(index_ov2640_html) - 1;
