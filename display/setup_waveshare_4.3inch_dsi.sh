#!/usr/bin/env bash
# Waveshare 4.3 DSI installer (idempotent)
# Usage: sudo ./setup_waveshare_4.3inch_dsi.sh [--reboot] [--no-reboot]

set -euo pipefail

FALLBACK_CONFIG="/boot/config.txt"
FIRMWARE_CONFIG="/boot/firmware/config.txt"
CONFIG="$FALLBACK_CONFIG"
if [ -f "$FIRMWARE_CONFIG" ]; then
  CONFIG="$FIRMWARE_CONFIG"
fi

OVERLAY_LINE="dtoverlay=vc4-kms-dsi-4.3inch"
VC4_GENERIC_PATTERN="^dtoverlay=vc4-kms-"
DO_REBOOT="prompt"

while [ "$#" -gt 0 ]; do
  case "$1" in
    --reboot) DO_REBOOT=yes; shift ;;
    --no-reboot) DO_REBOOT=no; shift ;;
    -h|--help) echo "Usage: $0 [--reboot|--no-reboot]"; exit 0 ;;
    *) echo "Unknown option: $1"; exit 2 ;;
  esac
done

echo "Starting Waveshare 4.3inch DSI setup..."

if [ ! -f "$CONFIG" ]; then
    echo "ERROR: $CONFIG not found. Are you running on a Raspberry Pi?" >&2
    exit 2
fi

# Backup config if not already backed up
if [ ! -f "${CONFIG}.orig" ]; then
  sudo cp -v "$CONFIG" "${CONFIG}.orig"
fi

# If a generic vc4 overlay is present (like vc4-kms-v3d), comment it out to avoid conflicts
if grep -E "$VC4_GENERIC_PATTERN" "$CONFIG" >/dev/null 2>&1; then
  echo "Found generic vc4 overlay(s) — commenting them out to prefer the DSI overlay"
  sudo cp -v "$CONFIG" "${CONFIG}.bak.vc4.$(date +%s)"
  # Comment any line that starts with dtoverlay=vc4-kms-
  sudo sed -ri 's/^(dtoverlay=vc4-kms-[^[:space:]]*)/# \1/' "$CONFIG"
fi

# Add DSI overlay if missing
if ! grep -Fxq "$OVERLAY_LINE" "$CONFIG"; then
  sudo cp -v "$CONFIG" "${CONFIG}.bak.$(date +%s)"
  echo "Adding display overlay to $CONFIG"
  echo "" | sudo tee -a "$CONFIG" >/dev/null
  echo "$OVERLAY_LINE" | sudo tee -a "$CONFIG" >/dev/null
else
  echo "Overlay already present in $CONFIG"
fi

echo "Installing required packages (idempotent)"
sudo apt-get update
sudo apt-get install -y --no-install-recommends xserver-xorg-input-evdev xinput-calibrator tslib

cat <<'EOF2'
Setup complete.

Notes:
- A reboot is required for the overlay changes to take effect.
- If you plan to run the TUI on the console (no X), ensure the service runs on a TTY.
EOF2

case "$DO_REBOOT" in
  yes)
    echo "Rebooting..."
    sudo reboot
    ;;
  no)
    echo "Skipping reboot as requested."
    ;;
  prompt)
    read -p "Reboot now? (y/N): " REBOOT
    case "${REBOOT,,}" in
      y|yes) sudo reboot ;;
      *) echo "Please reboot manually to apply changes." ;;
    esac
    ;;
esac
