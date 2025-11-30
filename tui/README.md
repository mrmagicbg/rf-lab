RPI TUI service
================

If you change the unit file `rpi_tui.service`, reload systemd and (re)enable the service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now rpi_tui.service
```

The service includes a short startup delay to allow the system to reach `multi-user.target`.
