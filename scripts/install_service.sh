#!/usr/bin/env bash
# Install script: copies repo to /opt, installs systemd service and enables it.
set -euo pipefail

REPO_SRC="$PWD"
TARGET="/opt/rpi-lab"
SERVICE_SRC="$REPO_SRC/tui/rpi_tui.service"
SERVICE_DST="/etc/systemd/system/rpi_tui.service"

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root: sudo $0"
  exit 1
fi

# Copy repo to /opt
rm -rf "$TARGET"
cp -a "$REPO_SRC" "$TARGET"
chown -R root:root "$TARGET" || true

# Install service
cp -v "$SERVICE_SRC" "$SERVICE_DST"
chmod 644 "$SERVICE_DST"

systemctl daemon-reload
systemctl enable rpi_tui.service
systemctl restart rpi_tui.service || true

echo "Installed rpi_tui.service and started it (if possible)."
