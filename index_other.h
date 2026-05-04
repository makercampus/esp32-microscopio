/*
 * Vista simple + portal + error
 */

const uint8_t index_simple_html[] = R"=====(<!doctype html>
<html lang="es">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Maker Campus | Vista Simple</title>
    <style>
      :root {
        --bg: #0f172a;
        --panel: #17253a;
        --text: #f8fafc;
        --muted: #bfd0ea;
        --accent: #22d3ee;
        --accent-2: #0ea5a8;
      }

      * { box-sizing: border-box; }

      body {
        margin: 0;
        font-family: "Segoe UI", Tahoma, sans-serif;
        color: var(--text);
        background: radial-gradient(circle at top, #173154 0%, var(--bg) 60%);
      }

      main {
        max-width: 980px;
        margin: 0 auto;
        padding: 16px;
      }

      .card {
        border-radius: 12px;
        background: linear-gradient(180deg, #1b2e47, var(--panel));
        border: 1px solid #325175;
        box-shadow: 0 12px 30px rgba(0,0,0,0.28);
      }

      .header {
        padding: 14px;
        display: flex;
        flex-wrap: wrap;
        gap: 8px;
        justify-content: space-between;
        align-items: center;
      }

      .title {
        margin: 0;
        font-size: 1rem;
      }

      .meta {
        margin-top: 4px;
        font-size: 0.85rem;
        color: var(--muted);
      }

      .actions {
        display: flex;
        gap: 8px;
      }

      button {
        border: 0;
        border-radius: 10px;
        padding: 10px 14px;
        font-weight: 600;
        cursor: pointer;
        color: #052933;
        background: linear-gradient(90deg, var(--accent), var(--accent-2));
      }

      button.ghost {
        color: #fff;
        background: #334e70;
      }

      button:hover { filter: brightness(1.08); }

      .viewer {
        padding: 0 14px 14px;
      }

      .viewer img {
        width: 100%;
        max-height: 72vh;
        object-fit: contain;
        background: #0b121d;
        border: 1px solid #39587b;
        border-radius: 12px;
      }

      .help {
        margin-top: 10px;
        color: var(--muted);
        font-size: 0.85rem;
      }
    </style>
  </head>

  <body>
    <main>
      <section class="card">
        <div class="header">
          <div>
            <h1 class="title" id="cam-name">Maker Campus Microscopio UDD</h1>
            <div class="meta" id="fw"></div>
          </div>
          <div class="actions">
            <button id="btn-full" class="ghost">Vista Full</button>
            <button id="btn-lamp">Encender luz</button>
          </div>
        </div>
        <div class="viewer">
          <img id="stream" alt="Video de cámara">
        </div>
      </section>
    </main>

    <script>
      document.addEventListener('DOMContentLoaded', async () => {
        const baseHost = document.location.origin;
        const streamImg = document.getElementById('stream');
        const btnLamp = document.getElementById('btn-lamp');
        const btnFull = document.getElementById('btn-full');

        const setControl = async (name, value) => {
          const query = `${baseHost}/control?var=${encodeURIComponent(name)}&val=${encodeURIComponent(value)}`;
          await fetch(query);
        };

        const state = await fetch(`${baseHost}/status`).then(r => r.json());

        document.getElementById('cam-name').textContent = state.cam_name || 'Maker Campus Microscopio UDD';
        document.title = `${state.cam_name || 'Maker Campus'} | Vista Simple`;

        streamImg.src = state.stream_url;

        let lampOn = Number(state.lamp) > 0;
        const simpleLamp = Number(state.lamp_simple_default || 25);

        const syncLampButton = () => {
          btnLamp.textContent = lampOn ? `Apagar luz` : `Encender luz`;
        };

        syncLampButton();

        btnLamp.addEventListener('click', async () => {
          lampOn = !lampOn;
          await setControl('lamp', lampOn ? simpleLamp : 0);
          syncLampButton();
        });

        btnFull.addEventListener('click', () => {
          window.open('/?view=full', '_self');
        });
      });
    </script>
  </body>
</html>)=====";

size_t index_simple_html_len = sizeof(index_simple_html) - 1;

const std::string portal_html = R"=====(<!doctype html>
<html lang="es">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title><CAMNAME> | Portal</title>
    <style>
      body {
        margin: 0;
        font-family: "Segoe UI", Tahoma, sans-serif;
        background: #0f172a;
        color: #f8fafc;
        text-align: center;
      }
      .wrap {
        max-width: 720px;
        margin: 24px auto;
        padding: 16px;
      }
      h1 { margin-top: 0; }
      .actions {
        display: flex;
        justify-content: center;
        flex-wrap: wrap;
        gap: 10px;
      }
      a { text-decoration: none; }
      button {
        border: 0;
        border-radius: 10px;
        padding: 10px 14px;
        font-weight: 600;
        cursor: pointer;
      }
    </style>
  </head>
  <body>
    <div class="wrap">
      <h1><CAMNAME></h1>
      <p>Portal de acceso Maker Campus - Universidad del Desarrollo</p>
      <div class="actions">
        <a href="<APPURL>?view=simple" target="_blank"><button>Vista Simple</button></a>
        <a href="<APPURL>?view=full" target="_blank"><button>Vista Full</button></a>
        <a href="<STREAMURL>" target="_blank"><button>Stream Directo</button></a>
      </div>
    </div>
  </body>
</html>)=====";

const std::string error_html = R"=====(<!doctype html>
<html lang="es">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title><CAMNAME> | Error</title>
    <style>
      body {
        margin: 0;
        font-family: "Segoe UI", Tahoma, sans-serif;
        background: #0f172a;
        color: #f8fafc;
        text-align: center;
      }
      .wrap { max-width: 720px; margin: 24px auto; padding: 16px; }
    </style>
  </head>
  <body>
    <div class="wrap">
      <h1><CAMNAME></h1>
      <ERRORTEXT>
    </div>
    <script>
      setTimeout(function() { location.replace(document.URL); }, 60000);
    </script>
  </body>
</html>)=====";
