// NOTE: termios is needed to compile tnylpo, but not used if graphical output is done with the help of ncurses. Only this output mode is working in this portation.
// /usr/include/bits/termios-c_iflag.h
/* c_iflag bits */
#define IGNBRK 0000001  /* Ignore break condition.  */
#define BRKINT 0000002  /* Signal interrupt on break.  */
#define IGNPAR 0000004  /* Ignore characters with parity errors.  */
#define PARMRK 0000010  /* Mark parity and framing errors.  */
#define INPCK 0000020   /* Enable input parity check.  */
#define ISTRIP 0000040  /* Strip 8th bit off characters.  */
#define INLCR 0000100   /* Map NL to CR on input.  */
#define IGNCR 0000200   /* Ignore CR.  */
#define ICRNL 0000400   /* Map CR to NL on input.  */
#define IUCLC 0001000   /* Map uppercase characters to lowercase on input \
                           (not in POSIX).  */
#define IXON 0002000    /* Enable start/stop output control.  */
#define IXANY 0004000   /* Enable any character to restart output.  */
#define IXOFF 0010000   /* Enable start/stop input control.  */
#define IMAXBEL 0020000 /* Ring bell when input queue is full \
                           (not in POSIX).  */
#define IUTF8 0040000   /* Input is UTF8 (not in POSIX).  */

// /usr/include/bits/termios-c_lflag.h
/* c_lflag bits */
#define ISIG 0000001   /* Enable signals.  */
#define ICANON 0000002 /* Canonical input (erase and kill processing).  */
#define ECHO 0000010   /* Enable echo.  */

// /usr/include/bits/termios-c_cc.h
/* c_cc characters */
#define VINTR 0
#define VQUIT 1
#define VERASE 2
#define VKILL 3
#define VEOF 4
#define VTIME 5
#define VMIN 6
#define VSWTC 7
#define VSTART 8
#define VSTOP 9
#define VSUSP 10
#define VEOL 11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE 14
#define VLNEXT 15
#define VEOL2 16

// /usr/include/bits/termios.h
typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

// /usr/include/bits/termios-struct.h
#define NCCS 32
struct termios {
    tcflag_t c_iflag; /* input mode flags */
    tcflag_t c_oflag; /* output mode flags */
    tcflag_t c_cflag; /* control mode flags */
    tcflag_t c_lflag; /* local mode flags */
    cc_t c_line;      /* line discipline */
    cc_t c_cc[NCCS];  /* control characters */
};

// /usr/include/termios.h
/* Set the state of FD to *TERMIOS_P.
   Values for OPTIONAL_ACTIONS (TCSA*) are in <bits/termios.h>.  */
extern int tcsetattr(int __fd, int __optional_actions, const struct termios *__termios_p);

/* Put the state of FD into *TERMIOS_P.  */
extern int tcgetattr(int __fd, struct termios *__termios_p);

// /usr/include/bits/termios-tcflow.h
/* tcsetattr uses these.  */
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2

// /usr/include/bits/termios-c_oflag.h
/* c_oflag bits */
#define OPOST 0000001 /* Post-process output.  */