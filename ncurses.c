#include "ncurses.h"
#include "picocalc.h"
#include <stdio.h>
#include <stdlib.h>

int COLOR_PAIRS;
WINDOW *curscr;

int curs_set(int a) {
    return -1;
}

int nodelay(WINDOW *w, bool status) {
    w->nodelay = status;
    return -1;
}

int idlok(WINDOW *w, bool a) {
    return -1;
}

int keypad(WINDOW *w, bool a) {
    return -1;
}

int delwin(WINDOW *w) {
    return -1;
}

int noraw(void) {
    return -1;
}

int nonl(void) {
    return -1;
}

int nl(void) {
    return -1;
}

int echo(void) {
    return -1;
}

int wmove(WINDOW *w, int a, int b) {
    return -1;
}

int wrefresh(WINDOW *w) {
    return -1;
}

int endwin(void) {
    return -1;
}

int refresh(void) {
    return -1;
}

int prefresh(WINDOW *w, int a, int b, int c, int d, int e, int f) {
    return -1;
}

// TODO: implement buffering
// TODO: implement echo
int wget_wch(WINDOW *w, wint_t *a) {
    if (w == NULL || a == NULL) {
        return ERR;
    }
    int key = picocalc_read_kbd();
    if (key == -1) {
        return ERR;
    }
    if (key >= 129 && key <= 137) {
        *a = KEY_F(key - 128);
        return KEY_CODE_YES;
    }
    switch (key) {
    case 144:
        *a = KEY_F(10);
        return KEY_CODE_YES;
    case 180:
        *a = KEY_LEFT;
        return KEY_CODE_YES;
    case 181:
        *a = KEY_UP;
        return KEY_CODE_YES;
    case 182:
        *a = KEY_DOWN;
        return KEY_CODE_YES;
    case 183:
        *a = KEY_RIGHT;
        return KEY_CODE_YES;
    case 210:
        *a = KEY_HOME;
        return KEY_CODE_YES;
    case 212:
        *a = KEY_DC; // delete character
        return KEY_CODE_YES;
    case 213:
        *a = KEY_END;
        return KEY_CODE_YES;
    }

    *a = key;
    return OK;
}

int init_pair(NCURSES_PAIRS_T a, NCURSES_COLOR_T b, NCURSES_COLOR_T c) {
    return ERR;
}

WINDOW *initscr(void) {
    if (curscr) {
        return curscr;
    }

    curscr = (WINDOW *)malloc(sizeof(WINDOW));
    if (!curscr) {
        return NULL;
    }
    return curscr;
}

bool has_colors(void) {
    return 0;
}

int start_color(void) {
    return 1;
}

int noecho(void) {
    return -1;
}

int raw(void) {
    return -1;
}

WINDOW *newpad(int a, int b) {
    return curscr;
}

int setcchar(cchar_t *a, const wchar_t *b, const attr_t c, NCURSES_PAIRS_T d, const void *e) {
    return -1;
}

int bkgrnd(const cchar_t *a) {
    return -1;
}

int wbkgrnd(WINDOW *w, const cchar_t *a) {
    return -1;
}

int erase(void) {
    return -1;
}

int werase(WINDOW *w) {
    return -1;
}

int beep(void) {
    picocalc_beep(750, 500);
    return 0;
}

int scrollok(WINDOW *w, bool a) {
    return -1;
}

int wscrl(WINDOW *w, int a) {
    return -1;
}

int wadd_wch(WINDOW *w, const cchar_t *a) {
    return -1;
}

int wclrtobot(WINDOW *w) {
    return -1;
}

int wclrtoeol(WINDOW *w) {
    return -1;
}

int winsertln(WINDOW *w) {
    return -1;
}

int wdeleteln(WINDOW *w) {
    return -1;
}

int wins_wch(WINDOW *w, const cchar_t *a) {
    return -1;
}

int wdelch(WINDOW *w) {
    return -1;
}

void getmaxyx(WINDOW *w, int y, int x) {
    y = x = 40;
}
