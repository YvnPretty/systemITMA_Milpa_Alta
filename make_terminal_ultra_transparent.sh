#!/usr/bin/env bash
# Make Ubuntu Default Terminal (Ptyxis & GNOME Terminal) Ultra-Transparent (Liquid Glass)

echo "Aplicando transparencia ultra-traslúcida Liquid Glass a la Terminal..."

# 1. Update Ptyxis profile opacity to 0.25 (75% transparency)
PROFILES=$(gsettings get org.gnome.Ptyxis profile-uuids 2>/dev/null | tr -d "[]',")

for uuid in $PROFILES; do
    echo "Ajustando opacidad de perfil Ptyxis $uuid a 0.25..."
    gsettings set org.gnome.Ptyxis.Profile:/org/gnome/Ptyxis/Profiles/$uuid/ opacity 0.25
done

# 2. Add GTK4 and GTK3 CSS overrides for terminal background transparency
mkdir -p "$HOME/.config/gtk-4.0" "$HOME/.config/gtk-3.0"

cat << 'EOF' > "$HOME/.config/gtk-4.0/gtk.css"
/* Liquid Glass Terminal Transparency */
ptyxis-terminal, .ptyxis-terminal, vte-terminal, window.terminal-window, .terminal-window {
    background-color: rgba(5, 8, 12, 0.35) !important;
}
EOF

cat << 'EOF' > "$HOME/.config/gtk-3.0/gtk.css"
/* Liquid Glass Terminal Transparency */
ptyxis-terminal, .ptyxis-terminal, vte-terminal, window.terminal-window, .terminal-window {
    background-color: rgba(5, 8, 12, 0.35) !important;
}
EOF

# 3. Kill active ptyxis processes so it reloads with the new 0.25 opacity
killall ptyxis gnome-terminal-server 2>/dev/null || true

echo "✓ Terminal predeterminada configurada al 75% de transparencia (Liquid Glass)."
