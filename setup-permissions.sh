#!/bin/bash

# Setup permissions for the project
echo "Setting up permissions..."

# Set execute permissions for the script
chmod +x setup-permissions.sh

# Set permissions for the project directories
chmod -R 755 src
chmod -R 755 include
chmod -R 755 tests
chmod -R 755 docs
chmod -R 755 /home/dae/svakla/plugins

echo "Permissions set up successfully."
