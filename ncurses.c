#include "ncurses.h"
#include "font.h"
#include "lcd.h"
#include "picocalc.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int COLOR_PAIRS;
NCURSES_PAIRS_T *defined_color_pairs = NULL;
WINDOW *curscr;
struct repeating_timer contentBlinkTimer;

int curs_set(int visibility) {
    curscr->prevCursorVisibility = curscr->cursorVisibility;
    curscr->cursorVisibility = visibility;
    wrefreshCursor(curscr);
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
        w->content = NULL;
        return OK;
    }

    curscr = NULL;
    free(w);

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
    w->prevcurx = w->curx;
    w->prevcury = w->cury;
    w->cury = y;
    w->curx = x;
    wrefreshCursor(w);
    return OK;
}

static void wrefreshCursor(WINDOW *w) {
    if (w == NULL) {
        return;
    }
    unsigned char fontSize = font_metadata.char_width;
    // paint over previous cursor position
    if (w->prevcurx != -1 && w->prevcury != -1) {
        drawContent(w, w->prevcurx, w->prevcury, false);
        w->prevcurx = -1;
        w->prevcury = -1;
    }
    // paint current cursor position with blinking
    if (w->cursorVisibility == visible) {
        if (!w->blinkstate) {
            unsigned int foreground = (defined_color_pairs[1] >> COLOR_SHIFT) & (COLOR_COUNT - 1);
            lcd_draw_rect(w->curx * fontSize, w->cury * fontSize, w->curx * fontSize + fontSize - 1, w->cury * fontSize + fontSize - 1, RGB_COLOR[foreground]);
        } else {
            drawContent(w, w->curx, w->cury, false);
        }
    }
    if (w->cursorVisibility == very_visible) {
        drawContent(w, w->curx, w->cury, true);
    }
}

static void drawContent(WINDOW *w, size_t x, size_t y, bool inverted) {
    unsigned char fontSize = font_metadata.char_width;
    cchar_t *cchar = &w->content[y * w->cols + x];
    int fc = RGB_COLOR[cchar->foreground];
    int bc = RGB_COLOR[cchar->background];
    if (inverted) {
        int tmp = fc;
        fc = bc;
        bc = tmp;
    }
    lcd_print_char(cchar->content, fc, bc, x * fontSize, y * fontSize, cchar->underline);
}

int wrefresh(WINDOW *w) {
    if (w == NULL || w->content == NULL) {
        return ERR;
    }
    wrefreshCursor(w);
    unsigned char fontSize = font_metadata.char_width;
    const uint64_t time = to_us_since_boot(get_absolute_time());
    cchar_t *cchar;

    for (int l = 0; l < w->lines; l++) {
        for (int c = 0; c < w->cols; c++) {
            cchar = &w->content[c * w->cols + l];
            if (!cchar->changed && !cchar->blink) {
                continue;
            }
            cchar->changed = false;
            if (cchar->blink && curscr->blinkstate) {
                lcd_draw_rect(l * 8, c * 8, (l * 8) + 8, (c * 8) + 8, RGB_COLOR[cchar->background]);
                continue;
            }
            lcd_print_char(cchar->content, RGB_COLOR[cchar->foreground], RGB_COLOR[cchar->background], l * 8, c * 8, cchar->underline);
        }
    }

    const int64_t delta = absolute_time_diff_us(time, to_us_since_boot(get_absolute_time()));
    printf("wrefresh took %lld us\n", delta);

    return OK;
}

int refresh(void) {
    if (curscr == NULL || curscr->content == NULL) {
        return ERR;
    }
    return wrefresh(curscr); // TODO: stdscr?
}

int prefresh(WINDOW *w, int a, int b, int c, int d, int e, int f) {
    // TODO: Partial refresh not needed?
    if (w == NULL || w->content == NULL) {
        return ERR;
    }
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

    if (w->echo) {
        // TODO: handle erase and delete
        w->content[cursorToIndex(w)].content = key;
        w->content[cursorToIndex(w)].changed = true;
        if (w->curx == (w->lines - 1) && w->cury == (w->cols - 1)) {
            // TODO: what is the desired behavior in bottom right corner of screen?
            return OK;
        }
        // TODO: is wrapping around needed?
        if (w->curx == (w->lines - 1)) {
            w->curx = 0;
            w->cury++;
            return OK;
        }
        w->curx++;
        return OK;
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
            curscr->content[i].changed = true;
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
    curscr->cury = 0;
    curscr->curx = 0;
    curscr->prevcurx = -1;
    curscr->prevcury = -1;
    echo();
    curscr->prevCursorVisibility = invisible;
    curscr->cursorVisibility = invisible;
    curs_set(visible);

    add_repeating_timer_ms(-500, contentBlinkCallback, curscr, &contentBlinkTimer);

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
    curscr->lines = ysize;
    curscr->cols = xsize;
    curscr->content = calloc(curscr->lines * curscr->cols, sizeof(cchar_t));
    return curscr;
}

int setcchar(cchar_t *wcval, const wchar_t *wch, const attr_t attrs, NCURSES_PAIRS_T color_pair_index, const void *opts) {
    memset(wcval, 0, sizeof(*wcval));
    wcval->content = (char)wch[0];
    wcval->changed = true;
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
    // if (attrs & A_BOLD) {}

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
        w->content[i].changed = true;
    }
    return OK;
}

int erase(void) {
    return werase(curscr); // TODO: stdscr?
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
        markContentChanged(&w->content[cursorToIndex(w) + 1], n);
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
        markContentChanged(&w->content[(w->cury + 1) * w->cols], n * w->cols);
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
        markContentChanged(&w->content[w->cury * w->cols], n * w->cols);
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
        markContentChanged(&w->content[cursorToIndex(w)], n);
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
    const cchar_t EMPTY = {
        .content = 0x20, // ASCII space
        .blink = false,
        .changed = true,
        .color_pair_index = 0,
        .background = defined_color_pairs[1] & (COLOR_COUNT - 1),
        .foreground = (defined_color_pairs[1] >> COLOR_SHIFT) & (COLOR_COUNT - 1),
    };

    for (int i = 0; i < length; i++) {
        start[i] = EMPTY;
    }
}

static void markContentChanged(cchar_t *start, size_t length) {
    for (int i = 0; i < length; i++) {
        start[i].changed = true;
    }
}

bool contentBlinkCallback(struct repeating_timer *timer) {
    WINDOW *scr = timer->user_data;
    scr->blinkstate = !scr->blinkstate;
    // TODO: don't call wrefresh here; seems to be not performant
    wrefresh(scr);
    return true;
}
