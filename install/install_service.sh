#!/usr/bin/env bash
# Install script: copies repo to /opt, creates venv, installs requirements, installs systemd service and enables it.
set -euo pipefail

REPO_SRC="$PWD"
TARGET="/opt/rpi-lab"
SERVICE_SRC="$REPO_SRC/tui/rpi_tui.service"
SERVICE_DST="/etc/systemd/system/rpi_tui.service"
VENV_DIR="$TARGET/.venv"

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root: sudo $0"
  exit 1
fi

# Copy repo to /opt
rm -rf "$TARGET"
cp -a "$REPO_SRC" "$TARGET"
chown -R root:root "$TARGET" || true

# Create venv and install requirements (if file exists)
if command -v python3 >/dev/null 2>&1; then
  python3 -m venv "$VENV_DIR"
  "$VENV_DIR/bin/pip" install --upgrade pip
  if [ -f "$TARGET/tui/requirements.txt" ]; then
    "$VENV_DIR/bin/pip" install -r "$TARGET/tui/requirements.txt"
  fi
fi

# Install service
# The service ExecStart should point to the venv python if available
if [ -f "$SERVICE_SRC" ]; then
  cp -v "$SERVICE_SRC" "$SERVICE_DST"
  chmod 644 "$SERVICE_DST"
fi

systemctl daemon-reload
systemctl enable rpi_tui.service
systemctl restart rpi_tui.service || true

echo "Installed rpi_tui.service and started it (if possible)."
