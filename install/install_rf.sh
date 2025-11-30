#!/usr/bin/env bash
# Wrapper to run RF setup scripts (non-interactive)
set -euo pipefail

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root: sudo $0"
  exit 1
fi

RF_SCRIPT="/opt/rpi-lab/rf/setup_pi.sh"
if [ ! -f "$RF_SCRIPT" ]; then
  echo "RF script not found at $RF_SCRIPT"
  exit 2
fi

bash "$RF_SCRIPT"
 echo "RF installation finished."
