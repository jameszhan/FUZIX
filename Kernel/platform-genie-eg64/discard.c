#include <kernel.h>
#include <devhd.h>
#include <devtty.h>
#include <tty.h>
#include <kdata.h>
#include "devgfx.h"
#include <devide.h>
#include "trs80.h"

static void vt_check_lower(void)
{
  *VT_BASE = 'a';
  if (*VT_BASE == 'a')
    video_lower = 1;
}

void device_init(void)
{
  vt_check_lower();
#ifdef CONFIG_RTC
  /* Time of day clock */
  inittod();
#endif
  hd_probe();
  devide_init();
  trstty_probe();
  gfx_init();
}

void map_init(void)
{
}

/* string.c
 * Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */
static int strcmp(const char *d, const char *s)
{
	register char *s1 = (char *) d, *s2 = (char *) s, c1, c2;

	while ((c1 = *s1++) == (c2 = *s2++) && c1);
	return c1 - c2;
}

uint8_t platform_param(char *p)
{
    if (strcmp(p, "pcg80") == 0) {
     trs80_udg = UDG_PCG80;
     return 1;
    }
    if (strcmp(p, "80gfx") == 0) {
     trs80_udg = UDG_80GFX;
     return 1;
    }
    if (strcmp(p, "micro") == 0) {
     trs80_udg = UDG_MICROFIRMA;
     return 1;
    }
    return 0;
}
