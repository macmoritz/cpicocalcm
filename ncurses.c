#include "ncurses.h"
#include "font.h"
#include "lcd.h"
#include "picocalc.h"
#include <pico/multicore.h>
#include <pico/time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int COLOR_PAIRS;
NCURSES_PAIRS_T *defined_color_pairs = NULL;
WINDOW *curscr;
WINDOW *displayscr;
struct repeating_timer contentBlinkTimer;

int curs_set(int visibility) {
    curscr->prevCursorVisibility = curscr->cursorVisibility;
    curscr->cursorVisibility = visibility;
    return curscr->prevCursorVisibility;
}

int nodelay(WINDOW *w, bool status) {
    w->nodelay = status;
    return -1;
}

int idlok(WINDOW *w, bool a) {
    // probably no function needed
    return OK;
}

int keypad(WINDOW *w, bool a) {
    // TODO: needed?
    return OK;
}

int delwin(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }

    // Since pad and window are the same in this implementation and tnylpo calls `delwin(pad_p)` before `delwin(win_p)`,
    // this trap for the first call is needed.
    if (w->content != NULL) {
        free(w->content);
        free(displayscr->content);
        w->content = NULL;
        return OK;
    }

    free(w);
    free(displayscr);
    curscr = NULL;
    displayscr = NULL;

    if (defined_color_pairs != NULL) {
        free(defined_color_pairs);
        defined_color_pairs = NULL;
    }

    return OK;
}

int endwin(void) {
    lcd_clear();
    return OK;
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
    if (!curscr) {
        return ERR;
    }
    curscr->echo = true;
    return OK;
}

int noecho(void) {
    if (!curscr) {
        return ERR;
    }
    curscr->echo = false;
    return OK;
}

int wmove(WINDOW *w, int y, int x) {
    if (w == NULL) {
        return ERR;
    }
    if (y >= w->cols || x >= w->lines) {
        return ERR;
    }
    w->cury = y;
    w->curx = x;
    return OK;
}

int wrefresh(WINDOW *w) {
    if (w == NULL || w->content == NULL) {
        return ERR;
    }

    memcpy(displayscr->content, w->content, w->lines * w->cols * sizeof(cchar_t));
    displayscr->cursorVisibility = w->cursorVisibility;
    displayscr->prevCursorVisibility = w->prevCursorVisibility;
    displayscr->curx = w->curx;
    displayscr->cury = w->cury;
    displayscr->cols = w->cols;
    displayscr->lines = w->lines;

    return OK;
}

int refresh(void) {
    return wrefresh(curscr);
}

int prefresh(WINDOW *w, int a, int b, int c, int d, int e, int f) {
    return wrefresh(w);
}

// TODO: implement buffering
int wget_wch(WINDOW *w, wint_t *a) {
    if (w == NULL || a == NULL) {
        return ERR;
    }
    int key = picocalc_read_kbd();
    if (key == -1) {
        return ERR;
    }

    // window `echo` is not used by tnylpo, therefor it is not implemented

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
    case 8:
        *a = KEY_BACKSPACE;
        return KEY_CODE_YES;
    case 10:
        *a = 13;   // Carriage return
        return OK; // passing to CP/M by tnylpo
    }

    *a = key;
    return OK;
}

int init_pair(NCURSES_PAIRS_T index, NCURSES_COLOR_T foreground, NCURSES_COLOR_T background) {
    if (defined_color_pairs == NULL || index == 0 || index >= COLOR_PAIRS || foreground >= COLOR_COUNT || background >= COLOR_COUNT) {
        return ERR;
    }

    if (defined_color_pairs[index] != SHORT_MAX) {
        for (int i = 0; i < curscr->lines * curscr->cols; i++) {
            if (curscr->content[i].color_pair_index != index) {
                continue;
            }
            curscr->content[i].foreground = foreground;
            curscr->content[i].background = background;
        }
    }
    defined_color_pairs[index] = (foreground << COLOR_SHIFT) + background;

    return OK;
}

WINDOW *initscr(void) {
    if (curscr) {
        return curscr;
    }

    curscr = calloc(1, sizeof(WINDOW));
    if (!curscr) {
        return NULL;
    }
    curs_set(visible);

    displayscr = calloc(1, sizeof(WINDOW));
    if (!displayscr) {
        return NULL;
    }

    add_repeating_timer_ms(-500, contentBlinkCallback, displayscr, &contentBlinkTimer);

    return curscr;
}

bool has_colors(void) {
    return true;
}

int start_color(void) {
    if (defined_color_pairs != NULL) {
        return OK;
    }

    COLOR_PAIRS = COLOR_COUNT * COLOR_COUNT + 1; // +1 is needed to stop complains from tnylpo about limit of color pairs
    defined_color_pairs = calloc(COLOR_PAIRS, sizeof(NCURSES_PAIRS_T));
    memset(defined_color_pairs, SHORT_MAX, COLOR_PAIRS);
    if (defined_color_pairs == NULL) {
        return ERR;
    }

    // init pair 0 to white on black
    // init_pair(0, COLOR_BLACK, COLOR_WHITE); is not allowed since pair 0 is reserved
    defined_color_pairs[0] = (COLOR_WHITE << COLOR_SHIFT) + COLOR_BLACK;
    return OK;
}

int raw(void) {
    return -1;
}

WINDOW *newpad(int xsize, int ysize) {
    if (!curscr || !displayscr) {
        return NULL;
    }
    curscr->lines = ysize;
    displayscr->lines = ysize;
    curscr->cols = xsize;
    displayscr->cols = xsize;
    curscr->content = calloc(curscr->lines * curscr->cols, sizeof(cchar_t));
    if (curscr->content == NULL) {
        printf("calloc failed\n");
        return NULL;
    }
    displayscr->content = calloc(displayscr->lines * displayscr->cols, sizeof(cchar_t));
    if (displayscr->content == NULL) {
        printf("calloc failed\n");
        return NULL;
    }
    return curscr;
}

int setcchar(cchar_t *wcval, const wchar_t *wch, const attr_t attrs, NCURSES_PAIRS_T color_pair_index, const void *opts) {
    memset(wcval, 0, sizeof(*wcval));
    wcval->content = (char)wch[0];
    wcval->color_pair_index = 0;

    if (defined_color_pairs != NULL && color_pair_index >= 0 && color_pair_index <= COLOR_PAIRS && defined_color_pairs[color_pair_index] != SHORT_MAX) {
        wcval->color_pair_index = color_pair_index;
    }
    NCURSES_PAIRS_T color_pair = defined_color_pairs[wcval->color_pair_index];
    wcval->background = color_pair & (COLOR_COUNT - 1);
    wcval->foreground = (color_pair >> COLOR_SHIFT) & (COLOR_COUNT - 1);

    if (attrs & A_STANDOUT || attrs & A_REVERSE) { // invert colors
        enum Color temp = wcval->background;
        wcval->background = wcval->foreground;
        wcval->foreground = temp;
    }

    wcval->blink = (attrs & A_BLINK) != 0;
    wcval->underline = (attrs & A_UNDERLINE) != 0;
    wcval->bold = (attrs & A_BOLD) != 0;

    return OK;
}

int bkgrnd(const cchar_t *a) {
    return wbkgrnd(curscr, a);
}

int wbkgrnd(WINDOW *w, const cchar_t *bkgrnd) {
    if (w == NULL || bkgrnd == NULL) {
        return ERR;
    }
    for (int i = 0; i < w->lines * w->cols; i++) {
        w->content[i].color_pair_index = 0;
        w->content[i].foreground = bkgrnd->foreground;
        w->content[i].background = bkgrnd->background;
    }
    return OK;
}

int erase(void) {
    return werase(curscr);
}

int werase(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }
    clearContent(w->content, w->lines * w->cols);
    return OK;
}

int beep(void) {
    picocalc_beep(440, 500);
    return 0;
}

int scrollok(WINDOW *w, bool status) {
    if (w == NULL) {
        return ERR;
    }
    w->scroll = status;
    return OK;
}

int wscrl(WINDOW *w, int move) {
    if (w == NULL || !w->scroll || move >= w->lines) {
        return ERR;
    }
    if (move == 0) {
        return OK;
    }
    const int prev_curx = w->curx;
    const int prev_cury = w->cury;

    if (move > 0) {
        memmove(&w->content[0], &w->content[move * w->cols], (w->lines - move) * w->cols * sizeof(w->content[0]));
        clearContent(&w->content[(w->lines - move) * w->cols], move * w->cols * sizeof(w->content[0]));
    } else if (move < 0) {
        memmove(&w->content[move * w->cols], &w->content[0], (w->lines - move) * w->cols * sizeof(w->content[0]));
        clearContent(&w->content[0], move * w->cols * sizeof(w->content[0]));
    }

    w->curx = prev_curx;
    w->cury = prev_cury;
    return OK;
}

int wadd_wch(WINDOW *w, const cchar_t *a) {
    if (w == NULL || a == NULL) {
        return ERR;
    }

    w->content[cursorToIndex(w)] = *a;
    // printf("%c", a->content);
    // TODO: `may advance cursor position`, see `putwchar`
    return OK;
}

int wins_wch(WINDOW *w, const cchar_t *a) {
    if (w == NULL || a == NULL) {
        return ERR;
    }
    const int n = w->cols - w->curx - 1;
    if (n > 0) {
        memmove(&w->content[cursorToIndex(w) + 1], &w->content[cursorToIndex(w)], n * sizeof(w->content[0]));
    }
    w->content[cursorToIndex(w)] = *a;
    return OK;
}

int wclrtobot(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }
    const int n = w->lines * w->cols - cursorToIndex(w);
    if (n > 0) {
        clearContent(&w->content[cursorToIndex(w)], n);
    }
    wrefresh(w);
    return OK;
}

int wclrtoeol(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }
    const int n = w->cols - w->curx;
    if (n > 0) {
        clearContent(&w->content[cursorToIndex(w)], n);
    }
    wrefresh(w);
    return OK;
}

int winsertln(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }
    const int n = w->lines - 1 - w->cury;
    if (n > 0) {
        memmove(&w->content[(w->cury + 1) * w->cols], &w->content[w->cury * w->cols], n * w->cols * sizeof(w->content[0]));
    }
    clearline(w, w->cury);
    wrefresh(w);
    return OK;
}

int wdeleteln(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }
    const int n = w->lines - 1 - w->cury;
    if (n > 0) {
        memmove(&w->content[w->cury * w->cols], &w->content[(w->cury + 1) * w->cols], n * w->cols * sizeof(w->content[0]));
    }
    clearline(w, w->lines - 1);
    wrefresh(w);
    return OK;
}

int wdelch(WINDOW *w) {
    if (w == NULL) {
        return ERR;
    }
    const int n = w->cols - 1 - w->curx;
    if (n > 0) {
        memmove(&w->content[cursorToIndex(w)], &w->content[cursorToIndex(w) + 1], n * sizeof(w->content[0]));
    }
    clearContent(&w->content[w->cury * w->cols + w->cols - 1], 1);
    wrefresh(w);
    return OK;
}

void getmaxyx(WINDOW *w, int y, int x) {
    y = w->lines;
    x = w->cols;
}

static void clearline(WINDOW *w, int line) {
    clearContent(&w->content[line * w->cols], w->cols);
}

static int cursorToIndex(WINDOW *w) {
    return w->cury * w->cols + w->curx;
}

static void clearContent(cchar_t *start, size_t length) {
    cchar_t empty;
    memset(&empty, 0, sizeof(empty));

    empty.content = 0x20; // ASCII space
    empty.color_pair_index = 0;
    empty.background = defined_color_pairs[1] & (COLOR_COUNT - 1);
    empty.foreground = (defined_color_pairs[1] >> COLOR_SHIFT) & (COLOR_COUNT - 1);

    for (int i = 0; i < length; i++) {
        start[i] = empty;
    }
}

bool contentBlinkCallback(struct repeating_timer *timer) {
    WINDOW *scr = timer->user_data;
    scr->blinkstate = !scr->blinkstate;
    return true;
}
