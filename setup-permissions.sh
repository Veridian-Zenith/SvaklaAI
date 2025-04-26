#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Please run as root${NC}"
    exit 1
fi

echo -e "${GREEN}Setting up SvaklaAI system...${NC}"

# Install required packages
echo -e "${YELLOW}Installing dependencies...${NC}"
if command -v pacman &> /dev/null; then
    # Arch-based systems
    pacman -Syu --noconfirm boost boost-libs openssl lmdb nlohmann-json gtk3 curl dbus libicu gcc make
elif command -v apt-get &> /dev/null; then
    # Debian-based systems
    apt-get update
    apt-get install -y libboost-all-dev libssl-dev liblmdb-dev nlohmann-json3-dev libgtk-3-dev libcurl4-openssl-dev libdbus-1-dev
else
    echo -e "${RED}Unsupported package manager. Please install dependencies manually:${NC}"
    echo "- Boost (system, filesystem, thread, beast)"
    echo "- OpenSSL"
    echo "- LMDB"
    echo "- nlohmann-json"
    echo "- GTK3"
    echo "- libcurl"
    echo "- dbus"
    exit 1
fi

# Create required directories if they don't exist
mkdir -p /opt/svakla/plugins
mkdir -p /etc/svakla
mkdir -p /var/lib/svakla/chat

# Set permissions
echo -e "${YELLOW}Setting up permissions...${NC}"
chown -R root:root /opt/svakla
chmod -R 755 /opt/svakla
chmod 1777 /opt/svakla/plugins

chown -R root:root /etc/svakla
chmod -R 755 /etc/svakla

chown -R root:root /var/lib/svakla
chmod -R 755 /var/lib/svakla
chmod 1777 /var/lib/svakla/chat

# Setup firewall rules
echo -e "${YELLOW}Configuring firewall rules...${NC}"
if command -v ufw &> /dev/null; then
    ufw allow 992/tcp comment 'SvaklaAI HTTP interface'
    ufw allow 776/tcp comment 'SvaklaAI WebSocket interface'
    ufw allow 933/tcp comment 'SvaklaAI CLI API interface'
    ufw reload
else
    echo -e "${YELLOW}UFW not found. Please configure your firewall manually:${NC}"
    echo "- Allow TCP port 992 (HTTP interface)"
    echo "- Allow TCP port 776 (WebSocket interface)"
    echo "- Allow TCP port 933 (CLI API interface)"
fi

# Create systemd service
echo -e "${YELLOW}Creating systemd service...${NC}"
cat > /etc/systemd/system/svaklaai.service << EOL
[Unit]
Description=SvaklaAI Service
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/svaklaai
Restart=always
User=root
Group=root
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOL

# Reload systemd
systemctl daemon-reload

echo -e "${GREEN}Setup complete!${NC}"
echo -e "${YELLOW}To start SvaklaAI:${NC}"
echo "  systemctl start svaklaai"
echo -e "${YELLOW}To enable SvaklaAI at boot:${NC}"
echo "  systemctl enable svaklaai"
echo -e "${YELLOW}To check status:${NC}"
echo "  systemctl status svaklaai"
echo -e "${YELLOW}To view logs:${NC}"
echo "  journalctl -u svaklaai"
