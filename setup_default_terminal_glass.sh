#!/usr/bin/env bash
# Configurar la Terminal predeterminada de Ubuntu (Ptyxis / GNOME Terminal) para ser transparente estilo Liquid Glass

PROFILES=$(gsettings get org.gnome.Ptyxis profile-uuids 2>/dev/null | tr -d "[]',")

for uuid in $PROFILES; do
    echo "Aplicando transparencia Liquid Glass a perfil Ptyxis: $uuid"
    gsettings set org.gnome.Ptyxis.Profile:/org/gnome/Ptyxis/Profiles/$uuid/ opacity 0.70
    gsettings set org.gnome.Ptyxis.Profile:/org/gnome/Ptyxis/Profiles/$uuid/ palette 'Hacker'
done

# También habilitar transparencia si gnome-terminal clásico estuviera presente
if which gnome-terminal >/dev/null 2>&1; then
    GT_PROFILES=$(dconf list /org/gnome/terminal/legacy/profiles:/ 2>/dev/null)
    for profile in $GT_PROFILES; do
        dconf write /org/gnome/terminal/legacy/profiles:/${profile}use-theme-colors false 2>/dev/null
        dconf write /org/gnome/terminal/legacy/profiles:/${profile}use-transparent-background true 2>/dev/null
        dconf write /org/gnome/terminal/legacy/profiles:/${profile}background-transparency-percent 30 2>/dev/null
    done
fi

echo "✓ Terminal predeterminada de Ubuntu configurada con transparencia Liquid Glass."
