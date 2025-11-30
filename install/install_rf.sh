#!/usr/bin/env bash
# Wrapper to run RF setup scripts (non-interactive)
set -euo pipefail

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root: sudo $0"
  exit 1
fi

# Find RF script
if [ -f "/opt/rpi-lab/rf/setup_pi.sh" ]; then
    RF_SCRIPT="/opt/rpi-lab/rf/setup_pi.sh"
elif [ -f "$HOME/rpi-lab/rf/setup_pi.sh" ]; then
    RF_SCRIPT="$HOME/rpi-lab/rf/setup_pi.sh"
else
    echo "RF script not found"
    exit 2
fi

bash "$RF_SCRIPT"
echo "RF installation finished."
