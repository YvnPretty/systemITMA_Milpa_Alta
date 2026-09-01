#!/usr/bin/env bash
# Hacker Tools Installer & Bash Profile Configurator

GREEN='\033[1;32m'
CYAN='\033[1;36m'
YELLOW='\033[1;33m'
WHITE='\033[1;37m'
NC='\033[0m'

echo -e "${CYAN}=== Cyberpunk / Hacker Tools Setup ===${NC}\n"

echo -e "${YELLOW}[1/3] Adding convenience aliases to ~/.bashrc ...${NC}"

BASHRC="$HOME/.bashrc"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Add aliases if not present
if ! grep -q "hacker_matrix.py" "$BASHRC" 2>/dev/null; then
    echo "" >> "$BASHRC"
    echo "# Hacker Environment Custom Aliases" >> "$BASHRC"
    echo "alias matrix='python3 $SCRIPT_DIR/hacker_matrix.py'" >> "$BASHRC"
    echo "alias sysdash='python3 $SCRIPT_DIR/hacker_dashboard.py'" >> "$BASHRC"
    echo "alias hacker-banner='bash $SCRIPT_DIR/hacker_banner.sh'" >> "$BASHRC"
    echo -e "${GREEN}✓ Aliases added: 'matrix', 'sysdash', 'hacker-banner'${NC}"
else
    echo -e "${GREEN}✓ Aliases already present in ~/.bashrc${NC}"
fi

# Make scripts executable
chmod +x "$SCRIPT_DIR/hacker_matrix.py" "$SCRIPT_DIR/hacker_dashboard.py" "$SCRIPT_DIR/hacker_banner.sh"

echo -e "\n${YELLOW}[2/3] Installing native Linux hacker CLI tools (cmatrix, hollywood, btop, htop)...${NC}"
echo -e "${WHITE}If prompted for password, type your sudo password:${NC}"

sudo apt update -y && sudo apt install -y cmatrix hollywood btop htop neofetch fastfetch 2>/dev/null || {
    echo -e "${YELLOW}Note: Sudo installation requires manual terminal execution or elevated privileges.${NC}"
    echo -e "${WHITE}Run this command in your terminal:${NC}"
    echo -e "${CYAN}sudo apt install -y cmatrix hollywood btop htop fastfetch${NC}"
}

echo -e "\n${GREEN}=== SETUP COMPLETE! ===${NC}"
echo -e "You can now run:"
echo -e "  - ${CYAN}matrix${NC}        (Python Matrix digital rain)"
echo -e "  - ${CYAN}sysdash${NC}       (Live hacker telemetry dashboard)"
echo -e "  - ${CYAN}hacker-banner${NC} (System banner)"
