#pragma once

#include "keys.h"
#include <stdbool.h>
#include <wchar.h>

/*
 * nodelay: 0 = nodelay, <0 = blocking, >0 = delay
 */
typedef struct _win_st {
    // short _cury, _curx; /* current cursor position */
    bool nodelay;
} WINDOW;

typedef struct {
} cchar_t;

typedef unsigned chtype;

typedef chtype attr_t;

#define ERR -1
#define OK 0
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
/*
 * nodelay  configures  the  input  character  reading function to be non-
 * blocking for window win.  If no input is ready,  the  reading  function
 * returns  ERR.  If disabled (bf is FALSE), the reading function does not
 * return until it has input.
 */
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
/*
 * wget_wch gathers a key event from the terminal keyboard associated with
 * a curses window  win,  returning  OK  if  a  wide  character  is  read,
 * KEY_CODE_YES  if  a  function  key  is read, and ERR if no key event is
 * available.
 *
 * When input is pending, wget_wch stores an integer identifying  the  key
 * event  in  wch;  for  alphanumeric  and  punctuation  keys,  this value
 * corresponds to the character encoding used by the terminal.  Use of the
 * control key as a modifier,  by  holding  it  down  while  pressing  and
 * releasing  another key, often results in a distinct code.  The behavior
 * of other keys depends on whether win is in keypad mode; see subsections
 * "Keypad Mode" and "Key Codes" in getch(3x).
 *
 * If no input is pending, then if the no-delay flag is set in the  window
 * (see  nodelay(3x)),  the  function returns ERR; otherwise, curses waits
 * until the terminal has  input.   If  cbreak(3x)  or  raw(3x)  has  been
 * called,  this  happens after one character is read.  If nocbreak(3x) or
 * noraw(3x) has been called, it occurs when the  next  newline  is  read.
 * (Because  the  terminal's  canonical or "cooked" mode is line-buffered,
 * multiple wget_wch calls may  then  be  necessary  to  empty  the  input
 * queue.)   If halfdelay(3x) has been called, curses waits until input is
 * available or the specified delay elapses.
 *
 * If echo(3x) has been called, and the window is not a pad, curses writes
 * the wide character from the input queue to the window  (at  the  cursor
 * position) per the following rules.
 *
 * - If  the  wide character matches the terminal's erase character (see
 *   erasewchar(3x)), the cursor moves leftward one position and the new
 *   position is erased as if wmove(3x) and then wdelch(3x) were called.
 *   When the window's keypad mode is enabled (see below), KEY_LEFT  and
 *   KEY_BACKSPACE are handled the same way.
 *
 * - curses  writes  any  other  wide  character  to the window, as with
 *   wecho_wchar(3x).
 *
 * - If the window win has been moved or modified since the last call to
 *   wrefresh(3x), curses calls wrefresh on it.
 *
 * If the wide character is a carriage return and nl(3x) has been  called,
 * wget_wch stores the wide character code for line feed in wch instead.
 */
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
