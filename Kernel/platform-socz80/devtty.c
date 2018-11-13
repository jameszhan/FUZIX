#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>


char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
    {   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 }
};

static tcflag_t console_mask[4] = {
	_ISYS,
	_OSYS,
	_CSYS,
	_LSYS
};

/* FIXME: will do for now though */
tcflag_t *termios_mask[NUM_DEV_TTY + 1] = {
	NULL,
	console_mask,
	console_mask
};

/* console helper */
void kputchar(char c)
{
    /* handle CRLF */
    if(c=='\n')
        tty_putc(1, '\r');
    tty_putc(1, c);
}

/* Called to set baud rate etc */
void tty_setup(uint8_t minor, uint8_t flag)
{
    minor;
}

/* For the moment */
int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}

void tty_sleeping(uint8_t minor)
{
    minor;
}

void tty_data_consumed(uint8_t minor)
{
}
