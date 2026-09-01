#!/usr/bin/env bash
# Script para hacer la barra superior transparente directamente en los temas de Ubuntu Yaru

echo "Respaldando y modificando temas de GNOME Shell para transparencia directa..."

# Backup original files if not already backed up
if [ ! -f /usr/share/gnome-shell/theme/Yaru-dark/gnome-shell.css.bak ]; then
    sudo cp /usr/share/gnome-shell/theme/Yaru-dark/gnome-shell.css /usr/share/gnome-shell/theme/Yaru-dark/gnome-shell.css.bak
fi

if [ ! -f /usr/share/gnome-shell/theme/Yaru/gnome-shell.css.bak ]; then
    sudo cp /usr/share/gnome-shell/theme/Yaru/gnome-shell.css /usr/share/gnome-shell/theme/Yaru/gnome-shell.css.bak
fi

# Replace background-color: black; in #panel with rgba(0, 0, 0, 0.15)
sudo sed -i 's/#panel {/ #panel {\n  background-color: rgba(0, 0, 0, 0.15) !important;/g' /usr/share/gnome-shell/theme/Yaru-dark/gnome-shell.css
sudo sed -i 's/#panel {/ #panel {\n  background-color: rgba(0, 0, 0, 0.15) !important;/g' /usr/share/gnome-shell/theme/Yaru/gnome-shell.css

echo "✓ Cambios directos aplicados."
