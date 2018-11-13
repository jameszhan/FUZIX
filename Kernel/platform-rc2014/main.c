#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devtty.h>
#include <devfd.h>
#include <devinput.h>
#include <rtc.h>
#include <ds1302.h>

extern unsigned char irqvector;
uint8_t timermsr = 0;
uint16_t swap_dev = 0xFFFF;

void platform_discard(void)
{
}

void platform_idle(void)
{
	/* FIXME: for the non CTC case we should poll the DS1302 here and
	   fake up appopriate timer events */
	/* Let's go to sleep while we wait for something to interrupt us;
	 * Makes the HALT LED go yellow, which amuses me greatly. */
//	__asm halt __endasm;
	irqflags_t irq = di();
	sync_clock();
	/* FIXME: need to cover ACIA option.. */
	tty_pollirq_sio();
	irqrestore(irq);
}

uint8_t platform_param(unsigned char *p)
{
	used(p);
	return 0;
}

void platform_interrupt(void)
{
	if (1) { 	/* CTC == 0 when we do the work */
#ifdef CONFIG_FLOPPY
		fd_tick();
#endif
//		poll_input();
//		timer_interrupt();
	}
	/* FIXME: need to cover ACIA option.. */
	tty_pollirq_sio();
	return;
}

/*
 *	Logic for tickless system. If you have an RTC you can ignore this.
 */

static uint8_t newticks = 0xFF;
static uint8_t oldticks;

static uint8_t re_enter;

/*
 *	Hardware specific logic to get the seconds. We really ought to enhance
 *	this to check minutes as well just in case something gets stuck for
 *	ages.
 */
static void sync_clock_read(void)
{
	uint8_t s;
	oldticks = newticks;
	ds1302_read_clock(&s, 1);
	s = (s & 0x0F) + (((s & 0xF0) >> 4) * 10);
	newticks = s;
}

/*
 *	The OS core will invoke this routine when idle (via platform_idle) but
 *	also after a system call and in certain other spots to ensure the clock
 *	is roughly valid. It may be called from interrupts, without interrupts
 *	or even recursively so it must protect itself using the framework
 *	below.
 *
 *	Having worked out how much time has passed in 1/10ths of a second it
 *	performs that may timer_interrupt events in order to advance the clock.
 *	The core kernel logic ensures that we won't do anything silly from a
 *	jump forward of many seconds.
 *
 *	We also choose to poll the ttys here so the user has some chance of
 *	getting control back on a messed up process.
 */
void sync_clock(void)
{
	if (!timermsr) {
		irqflags_t irq = di();
		int16_t tmp;
		if (!re_enter++) {
			sync_clock_read();
			if (oldticks != 0xFF) {
				tmp = newticks - oldticks;
				if (tmp < 0)
					tmp += 60;
				tmp *= 10;
				while(tmp--) {
					timer_interrupt();
				}
				/* FIXME: need to cover ACIA option.. */
				tty_pollirq_sio();
			}
		}
		re_enter--;
		irqrestore(irq);
	}
}

/*
 *	This method is called if the kernel has changed the system clock. We
 *	don't work out how much work we need to do by using it as a reference
 *	so we don't care.
 */
void update_sync_clock(void)
{
}
