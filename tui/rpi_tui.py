import os
import subprocess
import sys
import curses

MENU = [
    "Run RF Script(s)",
    "Reboot Raspberry Pi",
    "Open Shell (CLI)",
    "Exit"
]

RF_SCRIPT_PATH = os.path.join(os.path.dirname(__file__), '../rf/setup_pi.sh')

def run_rf_script():
    subprocess.run(['bash', RF_SCRIPT_PATH])

def reboot_pi():
    subprocess.run(['sudo', 'reboot'])

def open_shell():
    os.system('bash')

def main_menu(stdscr):
    curses.curs_set(0)
    current_row = 0
    while True:
        stdscr.clear()
        h, w = stdscr.getmaxyx()
        for idx, row in enumerate(MENU):
            x = w//2 - len(row)//2
            y = h//2 - len(MENU)//2 + idx
            if idx == current_row:
                stdscr.attron(curses.color_pair(1))
                stdscr.addstr(y, x, row)
                stdscr.attroff(curses.color_pair(1))
            else:
                stdscr.addstr(y, x, row)
        stdscr.refresh()
        key = stdscr.getch()
        if key == curses.KEY_UP and current_row > 0:
            current_row -= 1
        elif key == curses.KEY_DOWN and current_row < len(MENU) - 1:
            current_row += 1
        elif key in [curses.KEY_ENTER, ord('\n')]:
            if current_row == 0:
                run_rf_script()
            elif current_row == 1:
                reboot_pi()
            elif current_row == 2:
                open_shell()
            elif current_row == 3:
                break

def setup_curses():
    curses.wrapper(main_menu)

if __name__ == "__main__":
    curses.initscr()
    curses.start_color()
    curses.init_pair(1, curses.COLOR_BLACK, curses.COLOR_CYAN)
    setup_curses()
