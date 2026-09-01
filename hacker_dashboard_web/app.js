/* ==========================================================================
   CYBERPUNK COMMAND CENTER - INTERACTIVE LOGIC & ENGINES
   ========================================================================== */

document.addEventListener('DOMContentLoaded', () => {

  // --- Theme & Configuration State ---
  let soundEnabled = false;
  let synthActive = false;
  let audioCtx = null;

  // --- Elements ---
  const matrixCanvas = document.getElementById('matrix-canvas');
  const matrixCtx = matrixCanvas.getContext('2d');
  const sparklineCanvas = document.getElementById('sparkline-canvas');
  const sparklineCtx = sparklineCanvas.getContext('2d');
  const spectrumCanvas = document.getElementById('spectrum-canvas');
  const spectrumCtx = spectrumCanvas.getContext('2d');

  const hudClock = document.getElementById('hud-clock');
  const themeSelect = document.getElementById('theme-select');
  const crtBtn = document.getElementById('toggle-crt-btn');
  const audioBtn = document.getElementById('toggle-audio-btn');
  const fullscreenBtn = document.getElementById('toggle-fullscreen-btn');

  const cpuVal = document.getElementById('cpu-val');
  const cpuBar = document.getElementById('cpu-bar');
  const ramVal = document.getElementById('ram-val');
  const ramBar = document.getElementById('ram-bar');
  const netVal = document.getElementById('net-val');
  const netBar = document.getElementById('net-bar');

  const terminalOutput = document.getElementById('terminal-output');
  const terminalForm = document.getElementById('terminal-form');
  const terminalInput = document.getElementById('terminal-input');
  const snifferFeed = document.getElementById('sniffer-feed');
  const playSynthBtn = document.getElementById('play-synth-btn');

  // ==========================================================================
  // 1. Matrix Digital Rain Canvas Engine
  // ==========================================================================
  let width = matrixCanvas.width = window.innerWidth;
  let height = matrixCanvas.height = window.innerHeight;

  const characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@#$%&*+=-~アエイオカキクケコサシスセソタチツテト".split('');
  const fontSize = 16;
  let columns = Math.floor(width / fontSize);
  let drops = Array(columns).fill(1);

  window.addEventListener('resize', () => {
    width = matrixCanvas.width = window.innerWidth;
    height = matrixCanvas.height = window.innerHeight;
    columns = Math.floor(width / fontSize);
    drops = Array(columns).fill(1);
  });

  function getPrimaryColor() {
    const computed = getComputedStyle(document.documentElement);
    return computed.getPropertyValue('--color-primary').trim() || '#00ff66';
  }

  function drawMatrix() {
    matrixCtx.fillStyle = 'rgba(7, 10, 14, 0.08)';
    matrixCtx.fillRect(0, 0, width, height);

    matrixCtx.fillStyle = getPrimaryColor();
    matrixCtx.font = `${fontSize}px 'Share Tech Mono', monospace`;

    for (let i = 0; i < drops.length; i++) {
      const text = characters[Math.floor(Math.random() * characters.length)];
      const x = i * fontSize;
      const y = drops[i] * fontSize;

      matrixCtx.fillText(text, x, y);

      if (y > height && Math.random() > 0.975) {
        drops[i] = 0;
      }
      drops[i]++;
    }
  }

  setInterval(drawMatrix, 45);

  // ==========================================================================
  // 2. Web Audio API Sound Generator
  // ==========================================================================
  function initAudio() {
    if (!audioCtx) {
      audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }
  }

  function playKeyClick() {
    if (!soundEnabled) return;
    initAudio();
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();

    osc.type = 'triangle';
    osc.frequency.setValueAtTime(800 + Math.random() * 400, audioCtx.currentTime);
    osc.frequency.exponentialRampToValueAtTime(100, audioCtx.currentTime + 0.03);

    gain.gain.setValueAtTime(0.08, audioCtx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + 0.03);

    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start();
    osc.stop(audioCtx.currentTime + 0.03);
  }

  function playTerminalBeep(freq = 600, duration = 0.08) {
    if (!soundEnabled) return;
    initAudio();
    const osc = audioCtx.createOscillator();
    const gain = audioCtx.createGain();

    osc.type = 'sine';
    osc.frequency.setValueAtTime(freq, audioCtx.currentTime);

    gain.gain.setValueAtTime(0.12, audioCtx.currentTime);
    gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + duration);

    osc.connect(gain);
    gain.connect(audioCtx.destination);
    osc.start();
    osc.stop(audioCtx.currentTime + duration);
  }

  // Synthesizer Beat Simulation
  let synthTimer = null;
  function toggleSynthBeat() {
    initAudio();
    synthActive = !synthActive;

    if (synthActive) {
      playSynthBtn.textContent = '■ DETENER SINTETIZADOR';
      playSynthBtn.style.background = 'var(--color-accent)';
      playSynthBtn.style.color = '#fff';

      let beatStep = 0;
      synthTimer = setInterval(() => {
        if (!synthActive) return;
        const baseFreq = (beatStep % 4 === 0) ? 120 : (beatStep % 2 === 0 ? 240 : 360);
        playTerminalBeep(baseFreq + Math.random() * 50, 0.06);
        beatStep = (beatStep + 1) % 16;
      }, 180);
    } else {
      playSynthBtn.textContent = '▶ SINTETIZADOR HACKER';
      playSynthBtn.style.background = '';
      playSynthBtn.style.color = '';
      if (synthTimer) clearInterval(synthTimer);
    }
  }

  playSynthBtn.addEventListener('click', toggleSynthBeat);

  // ==========================================================================
  // 3. Telemetry & Sparkline Engine
  // ==========================================================================
  const cpuHistory = Array(30).fill(25);

  function updateTelemetry() {
    // Clock
    const now = new Date();
    hudClock.textContent = now.toUTCString().split(' ')[4] + ' UTC';

    // Simulate CPU fluctuation
    const newCpu = Math.min(95, Math.max(12, Math.floor(25 + Math.sin(now.getTime() / 1000) * 15 + Math.random() * 20)));
    cpuVal.textContent = `${newCpu}%`;
    cpuBar.style.width = `${newCpu}%`;

    cpuHistory.push(newCpu);
    cpuHistory.shift();

    // Render Sparkline
    sparklineCtx.clearRect(0, 0, sparklineCanvas.width, sparklineCanvas.height);
    sparklineCtx.beginPath();
    sparklineCtx.strokeStyle = getPrimaryColor();
    sparklineCtx.lineWidth = 2;

    const stepX = sparklineCanvas.width / (cpuHistory.length - 1);
    for (let i = 0; i < cpuHistory.length; i++) {
      const x = i * stepX;
      const y = sparklineCanvas.height - (cpuHistory[i] / 100) * sparklineCanvas.height;
      if (i === 0) sparklineCtx.moveTo(x, y);
      else sparklineCtx.lineTo(x, y);
    }
    sparklineCtx.stroke();
  }

  setInterval(updateTelemetry, 800);

  // ==========================================================================
  // 4. Audio Spectrum Visualizer Canvas Animation
  // ==========================================================================
  function drawSpectrum() {
    spectrumCtx.clearRect(0, 0, spectrumCanvas.width, spectrumCanvas.height);
    const bars = 40;
    const barWidth = (spectrumCanvas.width / bars) - 2;

    for (let i = 0; i < bars; i++) {
      const barHeight = synthActive ? Math.random() * 80 + 10 : Math.sin(Date.now() / 200 + i) * 20 + 25;
      const x = i * (barWidth + 2);
      const y = spectrumCanvas.height - barHeight;

      spectrumCtx.fillStyle = (i % 5 === 0) ? 'var(--color-secondary)' : getPrimaryColor();
      spectrumCtx.fillRect(x, y, barWidth, barHeight);
    }

    requestAnimationFrame(drawSpectrum);
  }

  drawSpectrum();

  // ==========================================================================
  // 5. Security Packet Sniffer Simulator
  // ==========================================================================
  const protocols = ['TCP', 'UDP', 'SSH', 'HTTPS', 'MQTT', 'DNS'];
  const sourceIps = ['192.168.1.15', '10.0.4.88', '172.16.0.4', '8.8.8.8', '192.168.1.100'];

  function addSnifferLog() {
    const proto = protocols[Math.floor(Math.random() * protocols.length)];
    const ip = sourceIps[Math.floor(Math.random() * sourceIps.length)];
    const port = [80, 443, 22, 1883, 53][Math.floor(Math.random() * 5)];
    const isAlert = Math.random() < 0.15;

    const line = document.createElement('div');
    line.className = `sniffer-line ${isAlert ? 'alert' : ''}`;
    line.innerHTML = `
      <span>[${new Date().toLocaleTimeString()}] ${proto} ${ip}:${port}</span>
      <span>${isAlert ? 'ALERT: BLOCK' : 'PASS'}</span>
    `;

    snifferFeed.prepend(line);
    if (snifferFeed.children.length > 15) {
      snifferFeed.removeChild(snifferFeed.lastChild);
    }
  }

  setInterval(addSnifferLog, 1200);

  // ==========================================================================
  // 6. Interactive Terminal Engine
  // ==========================================================================
  function printTerminal(text, type = 'normal') {
    const line = document.createElement('div');
    line.className = 'terminal-line';

    if (type === 'cmd') {
      line.innerHTML = `<span class="prompt-symbol">operator@hacker-os:~$</span> ${text}`;
    } else if (type === 'output') {
      line.className = 'cmd-output';
      line.innerHTML = text;
    } else {
      line.textContent = text;
    }

    terminalOutput.appendChild(line);
    terminalOutput.scrollTop = terminalOutput.scrollHeight;
  }

  function handleCommand(cmdText) {
    const trimmed = cmdText.trim().toLowerCase();
    printTerminal(cmdText, 'cmd');
    playTerminalBeep(700, 0.05);

    if (trimmed === '') return;

    const parts = trimmed.split(' ');
    const cmd = parts[0];

    switch (cmd) {
      case 'help':
        printTerminal(`
<strong>COMANDOS DISPONIBLES:</strong><br>
▸ <strong>scan</strong>        : Ejecuta un escaneo de red simulado<br>
▸ <strong>sysinfo</strong>     : Muestra la información del hardware<br>
▸ <strong>matrix</strong>      : Activa ráfaga de código Matrix<br>
▸ <strong>theme [nombre]</strong>: Cambia el tema (matrix, cyber, amber, blood)<br>
▸ <strong>audio</strong>       : Activa/desactiva sonido<br>
▸ <strong>clear</strong>       : Limpia la terminal<br>
▸ <strong>whoami</strong>      : Muestra la identidad del usuario actual
        `, 'output');
        break;

      case 'scan':
        printTerminal('Iniciando Nmap CyberScan v7.92...', 'output');
        setTimeout(() => printTerminal('▸ Scanning 192.168.1.0/24... 5 hosts activos encontrados.', 'output'), 400);
        setTimeout(() => printTerminal('▸ PORT 22/tcp OPEN (SSH - OpenSSH 9.3)', 'output'), 800);
        setTimeout(() => printTerminal('▸ PORT 80/tcp OPEN (HTTP - Nginx Cyberpunk Engine)', 'output'), 1200);
        setTimeout(() => printTerminal('✓ Escaneo completado. Sin vulnerabilidades críticas.', 'output'), 1600);
        break;

      case 'sysinfo':
        printTerminal(`
<strong>HARDWARE SYSTEM INFO:</strong><br>
▸ OS: Ubuntu 26.04 LTS (Cyberpunk Custom Kernel 6.12)<br>
▸ CPU: 8x AMD Ryzen 7 / Intel Core i7 @ 3.8GHz<br>
▸ RAM: 16.0 GB DDR5 High Performance<br>
▸ GRAPHICS: Canvas 2D / WebGL Hardware Accelerated
        `, 'output');
        break;

      case 'matrix':
        printTerminal('Activando ráfaga de código Matrix...', 'output');
        for (let i = 0; i < 5; i++) {
          setTimeout(() => addSnifferLog(), i * 150);
        }
        break;

      case 'theme':
        if (parts[1] && ['matrix', 'cyber', 'amber', 'blood'].includes(parts[1])) {
          document.body.className = `theme-${parts[1]} ${document.body.classList.contains('crt-enabled') ? 'crt-enabled' : ''}`;
          themeSelect.value = `theme-${parts[1]}`;
          printTerminal(`Tema cambiado a: theme-${parts[1]}`, 'output');
        } else {
          printTerminal('Uso: theme [matrix | cyber | amber | blood]', 'output');
        }
        break;

      case 'audio':
        soundEnabled = !soundEnabled;
        audioBtn.textContent = soundEnabled ? 'AUDIO: ON' : 'AUDIO: OFF';
        printTerminal(`Audio cambiado a: ${soundEnabled ? 'ACTIVADO' : 'DESACTIVADO'}`, 'output');
        break;

      case 'clear':
        terminalOutput.innerHTML = '';
        break;

      case 'whoami':
        printTerminal('root@cyberpunk-command-center (Operator Level 5)', 'output');
        break;

      default:
        printTerminal(`Comando no reconocido: '${cmd}'. Escribe 'help' para asistencia.`, 'output');
    }
  }

  terminalForm.addEventListener('submit', (e) => {
    e.preventDefault();
    const val = terminalInput.value;
    terminalInput.value = '';
    handleCommand(val);
  });

  terminalInput.addEventListener('keydown', () => playKeyClick());

  // Quick Action Buttons
  document.querySelectorAll('.action-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const cmd = btn.getAttribute('data-cmd');
      handleCommand(cmd);
    });
  });

  // Controls Event Listeners
  themeSelect.addEventListener('change', (e) => {
    const isCrt = document.body.classList.contains('crt-enabled');
    document.body.className = `${e.target.value} ${isCrt ? 'crt-enabled' : ''}`;
    playTerminalBeep(500, 0.05);
  });

  crtBtn.addEventListener('click', () => {
    document.body.classList.toggle('crt-enabled');
    const isCrt = document.body.classList.contains('crt-enabled');
    crtBtn.textContent = isCrt ? 'CRT: ON' : 'CRT: OFF';
    playTerminalBeep(650, 0.05);
  });

  audioBtn.addEventListener('click', () => {
    soundEnabled = !soundEnabled;
    audioBtn.textContent = soundEnabled ? 'AUDIO: ON' : 'AUDIO: OFF';
    if (soundEnabled) playTerminalBeep(800, 0.1);
  });

  fullscreenBtn.addEventListener('click', () => {
    if (!document.fullscreenElement) {
      document.documentElement.requestFullscreen().catch(() => {});
    } else {
      document.exitFullscreen().catch(() => {});
    }
  });

});
