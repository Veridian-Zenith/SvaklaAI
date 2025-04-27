#!/bin/bash

# Autoconfig firewall script using UFW

# Allow incoming connections on ports 992, 776, 933, and 8443
sudo ufw allow 992/tcp
sudo ufw allow 776/tcp
sudo ufw allow 933/tcp
sudo ufw allow 8443/tcp

# Enable UFW
sudo ufw enable

# Status of UFW
sudo ufw status

# Additional firewall rules can be added here
