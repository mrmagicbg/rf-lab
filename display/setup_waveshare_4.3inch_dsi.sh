#!/bin/bash
# Script to setup Waveshare 4.3inch DSI LCD on Raspberry Pi 3
# Reference: https://www.waveshare.com/wiki/4.3inch_DSI_LCD

set -e

# Update system
sudo apt update && sudo apt upgrade -y

# Enable DSI display overlay
sudo sed -i '/^#dtoverlay=vc4-kms-dsi-7inch$/a dtoverlay=vc4-kms-dsi-4.3inch' /boot/config.txt

# Enable touch support (should be automatic, but ensure evdev is installed)
sudo apt install -y xserver-xorg-input-evdev

# Reboot required for changes to take effect
read -p "Setup complete. Reboot now? (y/n): " REBOOT
if [[ "$REBOOT" =~ ^[Yy]$ ]]; then
    sudo reboot
else
    echo "Please reboot manually to apply changes."
fi
