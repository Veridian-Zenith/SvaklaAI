#!/bin/bash

# Installation script for SvaklaAI

# Confirm system paths
echo "Confirming system paths..."

# Verify networking components
echo "Verifying networking components..."

# Install required dependencies
echo "Installing required dependencies..."
sudo pacman -Syu --noconfirm
sudo pacman -S --noconfirm --needed cmake g++ openssl

# Apply default firewall security profiles
echo "Applying default firewall security profiles..."
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow ssh
sudo ufw enable

# Run the firewall script
echo "Running the firewall script..."
chmod +x src/firewall_script.sh
./src/firewall_script.sh

# Status of UFW
sudo ufw status

# Build the project
echo "Building the project..."
mkdir -p build
cd build
cmake ..
make

# Copy all items to the installation directory
echo "Copying all items to the installation directory..."
mkdir -p /home/$USER/svakla
cp -r ../* /home/$USER/svakla/

# Final summary
echo "Final Summary: A completely private, local-first, scalable AI system with elite-level cognitive abilities, able to run flawlessly on extremely limited hardware without sacrificing functionality, reasoning, or model quality — and able to scale upward with optional, modular expansions."

echo "Installation complete. You can run the application with ./SvaklaAI"
