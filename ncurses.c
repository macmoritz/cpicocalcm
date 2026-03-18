#include "ncurses.h"
#include <stdio.h>
#include <stdlib.h>

int COLOR_PAIRS;
WINDOW *curscr;

int curs_set(int a) {
    return -1;
}

int nodelay(WINDOW *w, bool a) {
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

int wget_wch(WINDOW *w, wint_t *a) {
    return -1;
}

int init_pair(NCURSES_PAIRS_T a, NCURSES_COLOR_T b, NCURSES_COLOR_T c) {
    return -1;
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
    printf("ASCII 0x07 BELL\n");
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
