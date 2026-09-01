#!/usr/bin/env bash
# Script para hacer transparente la barra superior de Ubuntu GNOME (Fecha, Hora, Iconos)

THEME_DIR="$HOME/.themes/CyberpunkGlass/gnome-shell"
mkdir -p "$THEME_DIR"

cat << 'EOF' > "$THEME_DIR/gnome-shell.css"
@import url("/usr/share/gnome-shell/theme/Yaru-dark/gnome-shell.css");

/* Top Bar Panel Transparente / Liquid Glass */
#panel {
  background-color: rgba(0, 0, 0, 0.15) !important;
  border-bottom: 1px solid rgba(0, 255, 102, 0.25) !important;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.4) !important;
  transition-duration: 300ms !important;
}

#panel:overview {
  background-color: rgba(0, 0, 0, 0.3) !important;
}

#panel .panel-button {
  color: #00ff66 !important;
  font-weight: bold !important;
}

#panel .clock-label {
  color: #00f3ff !important;
  font-weight: bold !important;
  text-shadow: 0 0 6px rgba(0, 243, 255, 0.6) !important;
}
EOF

echo "✓ Tema de barra superior transparente creado en: $THEME_DIR/gnome-shell.css"
