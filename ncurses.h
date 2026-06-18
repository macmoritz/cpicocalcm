#pragma once

#include "keys.h"
#include "lcd.h"
#include <pico/time.h>
#include <stdbool.h>
#include <wchar.h>

#define ERR -1
#define OK 0
#define NCURSES_COLOR_T short
#define NCURSES_PAIRS_T short
#define SHORT_MAX 0x7fff

enum CursorVisibility {
    invisible = 0,
    visible,
    very_visible
};

enum Color {
    COLOR_BLACK = 0,
    COLOR_BLUE,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_YELLOW,
    COLOR_WHITE,
    COLOR_COUNT,
};

static const COLOR_TYPE RGB_COLOR[COLOR_COUNT] = {BLACK, BLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, WHITE};

#define COLOR_SHIFT 3 // log2(COLOR_COUNT)

/**
 * @brief Data structure representing a character cell of the display content.
 *
 * content: the character to be displayed in the cell
 * blink: whether the cell should blink
 * underline: whether the cell should be underlined
 * bold: whether the cell should be bold
 * color_pair_index: the index of the color pair used for the cell
 * background: the background color of the cell
 * foreground: the foreground color of the cell
 */
typedef struct {
    char content;
    bool blink;
    bool underline;
    bool bold;

    NCURSES_COLOR_T color_pair_index;
    enum Color background;
    enum Color foreground;
} cchar_t;

/**
 * @brief Data structure representing the display content and state managed by ncurses.
 *
 * lines: vertical, y; cols: horizontal, x; size of the window/content
 * cury: Cursor Y (horizontal, lines); curx: Cursor X (vertical, cols); current cursor position
 * nodelay: 0 = nodelay, <0 = blocking, >0 = delay
 * echo: Enables direct output by ncurses. Not used by tnylpo.
 * content: Screen char content
 */
typedef struct _win_st {
    short lines, cols;
    short cury, curx;
    bool nodelay;
    bool echo;
    bool scroll;
    bool blinkstate;
    enum CursorVisibility cursorVisibility;
    enum CursorVisibility prevCursorVisibility;

    cchar_t *content;
} WINDOW;

typedef unsigned int chtype;

typedef chtype attr_t;

/*
 * Maximum count of color pairs.
 */
extern int COLOR_PAIRS;

/*
 * List of all initialized color_pairs by `init_pair`.
 */
extern NCURSES_PAIRS_T *defined_color_pairs;

// #define COLOR_BLACK 0
// #define COLOR_RED 1
// #define COLOR_GREEN 2
// #define COLOR_YELLOW 3
// #define COLOR_BLUE 4
// #define COLOR_MAGENTA 5
// #define COLOR_CYAN 6
// #define COLOR_WHITE 7

extern WINDOW *curscr;
extern WINDOW *displayscr;

/*
 * curs_set adjusts the cursor visibility to “invisible”, “visible”, “very
 * visible”, as its argument is 0, 1, or 2, respectively.  It returns the
 * previous visibility if the requested one is supported, and ERR
 * otherwise.
 */
int curs_set(int);
/*
 * nodelay  configures  the  input  character  reading function to be non-
 * blocking for window win.  If no input is ready,  the  reading  function
 * returns  ERR.  If disabled (bf is FALSE), the reading function does not
 * return until it has input.
 */
int nodelay(WINDOW *, bool);
/*
 * Setting win's idlok property to TRUE causes curses to consider using
 * the insert/delete line capabilities of terminal types possessing them
 * according to the terminfo database.  Enable this option if the
 * application explicitly requires these operations, as a full-screen text
 * editor might; otherwise the results may be visually annoying to the
 * user.
 */
int idlok(WINDOW *, bool);
/*
 * keypad enables recognition of a terminal's function keys.  If enabled
 * (bf is TRUE) then when an input character reading function reads ESC,
 * it waits for further input corresponding to an escape sequence defined
 * by the terminal type description.  If a valid sequence populates the
 * input stream, the input character reading function returns a value
 * representing the function key, such as KEY_LEFT.  (Wide-character API
 * users: wget_wch(3X) returns KEY_CODE_YES to indicate the availability
 * of a function key code in its wch parameter.)  If the sequence is
 * invalid, the input character reading function returns only its last
 * character.  If disabled (bf is FALSE), curses does not treat function
 * keys specially and the program has to interpret escape sequences
 * itself.  If the terminal type description defines the keypad_local
 * (rmkx) and keypad_xmit (smkx) capabilities, enabling a window's keypad
 * mode sets the terminal's keypad to transmit, and disabling keypad mode
 * sets the terminal's keypad to work locally.  By default, a window's
 * keypad mode is off.
 */
int keypad(WINDOW *, bool);
/*
 * Calling delwin deletes the named window, freeing all memory associated
 * with it (it does not actually erase the window's screen image).
 * Subwindows must be deleted before the main window can be deleted.
 */
int delwin(WINDOW *);
/*
 * The program must also call endwin for each terminal being used before
 * exiting from curses.  If newterm is called more than once for the same
 * terminal, the first terminal referred to must be the last one for which
 * endwin is called.
 */
int endwin(void);
/*
 * raw configures the terminal to read input in raw mode, which is similar
 * to cbreak mode (see cbreak above) except that it furthermore passes
 * through the terminal's configured interrupt, quit, suspend, and flow
 * control characters uninterpreted to the application, instead of
 * generating a signal or acting on I/O flow.  The behavior of the
 * terminal's “Break” key (if any) depends on terminal driver
 * configuration parameters that curses does not handle.  noraw restores
 * the terminal's canonical (“cooked”) line discipline.
 */
int raw(void);
/*
 * raw configures the terminal to read input in raw mode, which is similar
 * to cbreak mode (see cbreak above) except that it furthermore passes
 * through the terminal's configured interrupt, quit, suspend, and flow
 * control characters uninterpreted to the application, instead of
 * generating a signal or acting on I/O flow.  The behavior of the
 * terminal's “Break” key (if any) depends on terminal driver
 * configuration parameters that curses does not handle.  noraw restores
 * the terminal's canonical (“cooked”) line discipline.
 */
int noraw(void);
/*
 * Initially, whether the terminal reports a carriage return using the
 * character code for a line feed in cbreak or raw modes depends on the
 * configuration of the terminal driver; see termios(3).  nl configures
 * the terminal to perform this translation.  nonl disables it.  Under its
 * canonical (“cooked”) line discipline, the terminal driver always
 * translates carriage returns to line feeds.
 */
int nonl(void);
/*
 * Initially, whether the terminal reports a carriage return using the
 * character code for a line feed in cbreak or raw modes depends on the
 * configuration of the terminal driver; see termios(3).  nl configures
 * the terminal to perform this translation.  nonl disables it.  Under its
 * canonical (“cooked”) line discipline, the terminal driver always
 * translates carriage returns to line feeds.
 */
int nl(void);
/*
 * echo and noecho determine whether characters typed by the user are
 * written to the curses window by the input character reading function as
 * they are typed.  curses always disables the terminal driver's own
 * echoing.  By default, a curses screen's echo option is set.  Authors of
 * most interactive programs prefer to do their own echoing in a
 * controlled area of the screen, or not to echo at all, so they call
 * noecho.  The man page for the input character reading function
 * discusses how echo and noecho interact with cbreak and nocbreak.
 */
int echo(void);
/*
 * echo and noecho determine whether characters typed by the user are
 * written to the curses window by the input character reading function as
 * they are typed.  curses always disables the terminal driver's own
 * echoing.  By default, a curses screen's echo option is set.  Authors of
 * most interactive programs prefer to do their own echoing in a
 * controlled area of the screen, or not to echo at all, so they call
 * noecho.  The man page for the input character reading function
 * discusses how echo and noecho interact with cbreak and nocbreak.
 */
int noecho(void);
/*
 * wmove relocates the cursor associated with the curses window win to
 * line y and column x.  The terminal's cursor does not move until
 * refresh(3X) is called.  The position (y, x) is relative to the upper
 * left-hand corner of the window, which has coordinates (0, 0).  move
 * similarly moves the cursor in the stdscr window.
 */
int wmove(WINDOW *, int, int);
/*
 * The refresh and wrefresh routines (or wnoutrefresh and doupdate) must
 * be called to get actual output to the terminal, as other routines
 * merely manipulate data structures.  The routine wrefresh copies the
 * named window to the physical screen, taking into account what is
 * already there to do optimizations.  The refresh routine is the same,
 * using stdscr as the default window.  Unless leaveok(3X) has been
 * enabled, the physical cursor of the terminal is left at the location of
 * the cursor for that window.
 */
int wrefresh(WINDOW *);
/*
 * The refresh and wrefresh routines (or wnoutrefresh and doupdate) must
 * be called to get actual output to the terminal, as other routines
 * merely manipulate data structures.  The routine wrefresh copies the
 * named window to the physical screen, taking into account what is
 * already there to do optimizations.  The refresh routine is the same,
 * using stdscr as the default window.  Unless leaveok(3X) has been
 * enabled, the physical cursor of the terminal is left at the location of
 * the cursor for that window.
 */
int refresh(void);
/*
 * prefresh is analogous to wrefresh(3X) except that it operates on pad
 * rather than window.
 * It requires additional parameters are needed to indicate what portions
 * of the pad and screen are involved.
 *
 * - pminrow and pmincol specify the upper left-hand corner of a
 *  rectangular view of the pad.
 *
 * - sminrow, smincol, smaxrow, and smaxcol specify the vertices of the
 *   rectangle to be displayed on the screen.
 *
 * The lower right-hand corner of the rectangle to be displayed in the pad
 * is calculated from the screen coordinates, since the rectangles must be
 * the same size.  Both rectangles must be entirely contained within their
 * respective structures.  curses treats negative values of any of these
 * parameters as zero.
 */
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
/*
 * The init_pair routine changes the definition of a color pair.  It takes
 * three arguments: the number of the color pair to be changed, the
 * foreground color number, and the background color number.  For portable
 * applications:
 *
 * - The first argument must be a valid color pair value.  If default
 *     colors are used (see use_default_colors(3X)) the upper limit is
 *     adjusted to allow for extra pairs which use a default color in
 *     foreground and/or background.

 * - The second and third arguments must be valid color values.
 *
 * If the color pair was previously initialized, the screen is refreshed
 * and all occurrences of that color pair are changed to the new
 * definition.
 *
 * As an extension, ncurses allows you to set color pair 0 via the
 * assume_default_colors(3X) routine, or to specify the use of default
 * colors (color number -1) if you first invoke the use_default_colors(3X)
 * routine.
 */
int init_pair(NCURSES_PAIRS_T, NCURSES_COLOR_T, NCURSES_COLOR_T);
WINDOW *initscr(void);
/*
 * has_colors returns TRUE if the terminal supports colors and FALSE if it
 * does not.  initscr(3X) or newterm(3X) must be called first, but
 * start_color need not be.
 */
bool has_colors(void);
/*
 * The start_color routine requires no arguments.  It must be called if
 * the programmer wants to use colors, and before any other color
 * manipulation routine is called.  It is good practice to call this
 * routine right after initscr.  start_color does this:
 *
 * - It initializes two global variables, COLORS and COLOR_PAIRS
 *   (respectively defining the maximum number of colors and color pairs
 *   the terminal can support).
 *
 * - It initializes the special color pair 0 to the default foreground
 *   and background colors.  No other color pairs are initialized.
 *
 * - It restores the colors on the terminal to the values they had when
 *   the terminal was just turned on.
 *
 * start_color returns ERR if it cannot allocate memory for its color
 * pair table.
 *
 * Annotations:
 * - COLOR_PAIRS corresponds to the terminal database's max_pairs
 *   capability, (see terminfo(5)).
 *
 * - valid color pair values are in the range 1 to COLOR_PAIRS-1,
 *   inclusive.
 *
 * - color pair 0 is special; it denotes “no color”.
 *
 * - Color pair 0 is assumed to be white on black, but is actually
 *   whatever the terminal implements before color is initialized.  It
 *   cannot be modified by the application.
 */
int start_color(void);
/*
 * newpad creates and returns a pointer to a new pad data structure with
 * the given number of lines, nlines, and columns, ncols.
 */
WINDOW *newpad(int, int);
/*
 * The setcchar function initializes the location pointed to by  wcval  by
 * using:
 *
 * - The character attributes in attrs
 *
 * - The color pair in color_pair
 *
 * - The  wide-character  string  pointed to by wch.  The string must be
 *   L'\0' terminated, contain at most one spacing character, which must
 *   be the first.
 *
 *   Up to CCHARW_MAX-1 non-spacing characters may  follow.   Additional
 *   non-spacing characters are ignored.
 *
 *   The string may contain a single control character instead.  In that
 *   case, no non-spacing characters are allowed.
 *
 * Upon successful completion, setcchar returns OK.  Otherwise, it returns
 * ERR.
 */
int setcchar(cchar_t *, const wchar_t *, const attr_t, NCURSES_PAIRS_T, const void *);
/*
 * See `wbkgrnd`.
 */
int bkgrnd(const cchar_t *);
/*
 * bkgrnd  and wbkgrnd set the background property of stdscr or the speci‐
 * fied window and then apply this setting to every character cell in that
 * window.
 *
 * - The rendition of every character in the window changes to  the  new
 *   background rendition.
 *
 * - Wherever the former background character appears, it changes to the
 *   new background character.
 *
 * ncurses  updates  the rendition of each character cell by comparing the
 * character, non-color attributes, and colors.  The  library  applies  to
 * following  procedure  to  each cell in the window, whether or not it is
 * blank.
 *
 * - ncurses first compares the cell's character to the previously spec‐
 *   ified blank character; if they match, ncurses writes the new  blank
 *   character to the cell.
 *
 * - ncurses then checks if the cell uses color, that is, its color pair
 *   value  is  nonzero.   If not, it simply replaces the attributes and
 *   color pair in the cell with those from the new  background  charac‐
 *   ter.
 *
 * - If  the  cell  uses color, and its background color matches that of
 *   the current window background, ncurses removes attributes that  may
 *   have  come  from the current background and adds those from the new
 *   background.  It finishes by setting the cell's  background  to  use
 *   the new window background color.
 *
 * - If  the  cell  uses  color, and its background color does not match
 *   that of the current window background,  ncurses  updates  only  the
 *   non-color  attributes, first removing those that may have come from
 *   the current background, and then adding  attributes  from  the  new
 *   background.
 *
 * ncurses  treats  a  background  character  value of zero (0) as a blank
 * character.
 *
 * If the terminal does not support color, or if color has not  been  ini‐
 * tialized with start_color(3NCURSES), ncurses ignores the new background
 * character's color attribute.
 */
int wbkgrnd(WINDOW *, const cchar_t *);
/*
 * The erase and werase routines copy blanks to every position in the
 * window, clearing the screen.
 *
 * Blanks created by erasure have the current background rendition (as set
 * by wbkgdset(3X)) merged into them.
 */
int erase(void);
/*
 * The erase and werase routines copy blanks to every position in the
 * window, clearing the screen.
 *
 * Blanks created by erasure have the current background rendition (as set
 * by wbkgdset(3X)) merged into them.
 */
int werase(WINDOW *);
/*
 * Ring the bell of the terminal.
 * Returns OK if the terminal type supports the corresponding capability.
 * Otherwise it returns ERR.
 */
int beep(void);
/*
 * The scrollok option controls what happens when a window's cursor moves
 * off the edge of the window or scrolling region, as a result of either
 * (1) writing a newline anywhere on its bottom line, or (2) writing a
 * character that advances the cursor to the last position on its bottom
 * line.  If disabled (bf is FALSE), curses leaves the cursor on the
 * bottom line of the window.  If enabled (bf is TRUE), curses scrolls the
 * window up one line.  (To get the physical scrolling effect on the
 * terminal, the application must also enable idlok).
 */
int scrollok(WINDOW *, bool);
/*
 * scroll scrolls the given window up one line.  That is, every visible
 * line we might number i becomes line i-1.  wscrl and scrl scroll the
 * specified window or stdscr, respectively, up or down per the sign of n.
 *
 * - For positive n, line i+n becomes i (scrolling up);
 *
 * - for negative n, line i-n becomes i (scrolling down).
 *
 * A line that scrolls beyond the window boundaries disappears; curses
 * populates a new one emerging at the opposite boundary with the
 * background character; see bkgd(3X) (wide-character API users:
 * bkgrnd(3X)).  As an optimization, if the scrolling region of the window
 * is the entire screen, the physical screen may be scrolled at the same
 * time; see curscr(3X).
 *
 * The cursor does not move.  These functions perform no operation unless
 * scrolling is enabled for the window via scrollok(3X).
 *
 * These functions return ERR upon failure and OK upon success.
 *
 * In ncurses, they return ERR if
 *
 * - the curses screen has not been initialized,
 *
 * - (for functions taking a WINDOW pointer argument) win is a null
 *    pointer, or
 *
 * - scrolling is not enabled in the window (as by scrollok(3X)).
 */
int wscrl(WINDOW *, int);
/*
 * wadd_wch writes the curses complex character wch to the window win,
 * then may advance the cursor position, analogously to the standard C
 * library's putwchar(3).  ncurses(3X) describes the variants of this
 * function.
 */
int wadd_wch(WINDOW *, const cchar_t *);
/*
 * wins_wch inserts the curses complex character wch at the cursor
 * position in the window win.  The character previously at the cursor and
 * any to its right move one cell to the right; the formerly rightmost
 * character on the line is discarded.  Unlike add_wch(3X), wins_wch does
 * not advance the cursor.  ncurses(3X) describes the variants of this
 * function.
 */
int wins_wch(WINDOW *, const cchar_t *);
/*
 * The wclrtobot routines erase from the cursor to the end of
 * screen.  That is, they erase all lines below the cursor in the window.
 * Also, the current line to the right of the cursor, inclusive, is
 * erased.
 */
int wclrtobot(WINDOW *);
/*
 * The clrtoeol and wclrtoeol routines erase the current line to the right
 * of the cursor, inclusive, to the end of the current line.
 */
int wclrtoeol(WINDOW *);
/*
 * winsertln inserts a new, empty line of characters above the line at the
 * cursor in win, shifting the existing lines down by one.  The content of
 * the window's bottom line is lost; curses fills the new line with the
 * background character.  The cursor position does not change.
 */
int winsertln(WINDOW *);
/*
 * wdeleteln deletes the line at the cursor in win; all lines below it
 * move up one line.  curses then fills the bottom line of win with the
 * background character configured by wbkgdset(3X) (wide-character API
 * users: wbkgrndset(3X)).  The cursor position does not change.
 */
int wdeleteln(WINDOW *);
/*
 * wdelch deletes the character at the cursor position in win.  It moves
 * all characters to the right of the cursor on the same line to the left
 * one position and replaces the contents of the rightmost position on the
 * line with the window's background character; see bkgd(3X) (wide-
 * character API users: bkgrnd(3X)).  The cursor position does not change
 * (after moving to (y, x), if specified).
 */
int wdelch(WINDOW *);

int getmaxy(const WINDOW *);
int getmaxx(const WINDOW *);
#define getmaxyx(win, y, x) (y = getmaxy(win), x = getmaxx(win))

/*
 * Sets the content of the given row to ASCII_SPACE.
 */
static void clearline(WINDOW *, int);

/**
 * @brief Translates cursor 2d-coordinate to array index.
 *
 * @return int array index representing cursor coordinate
 */
static int cursorToIndex(WINDOW *);

static void clearContent(cchar_t *start, size_t length);
