#pragma once

#include "keys.h"
#include <stdbool.h>
#include <wchar.h>

typedef struct _win_st {
} WINDOW;

typedef struct {
} cchar_t;

typedef unsigned chtype;

typedef chtype attr_t;

#define ERR (-1)
#define NCURSES_COLOR_T short
#define NCURSES_PAIRS_T short
extern int COLOR_PAIRS;

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

extern WINDOW *curscr;
int curs_set(int);
int nodelay(WINDOW *, bool);
int idlok(WINDOW *, bool);
int keypad(WINDOW *, bool);
int delwin(WINDOW *);
int noraw(void);
int nonl(void);
int nl(void);
int echo(void);
int wmove(WINDOW *, int, int);
int wrefresh(WINDOW *);
int endwin(void);
int refresh(void);
int prefresh(WINDOW *, int, int, int, int, int, int);
int wget_wch(WINDOW *, wint_t *);
int init_pair(NCURSES_PAIRS_T, NCURSES_COLOR_T, NCURSES_COLOR_T);
WINDOW *initscr(void);
bool has_colors(void);
int start_color(void);
int noecho(void);
int raw(void);
WINDOW *newpad(int, int);
int setcchar(cchar_t *, const wchar_t *, const attr_t, NCURSES_PAIRS_T, const void *);
int bkgrnd(const cchar_t *);
int wbkgrnd(WINDOW *, const cchar_t *);
int erase(void);
int werase(WINDOW *);
int beep(void);
int scrollok(WINDOW *, bool);
int wscrl(WINDOW *, int);
int wadd_wch(WINDOW *, const cchar_t *);
int wclrtobot(WINDOW *);
int wclrtoeol(WINDOW *);
int winsertln(WINDOW *);
int wdeleteln(WINDOW *);
int wins_wch(WINDOW *, const cchar_t *);
int wdelch(WINDOW *);

// int getmaxy(const WINDOW *);
// int getmaxx(const WINDOW *);
// #define getmaxyx(win, y, x) (y = getmaxy(win), x = getmaxx(win))
void getmaxyx(WINDOW *, int, int);
