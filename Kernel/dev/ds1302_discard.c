/*-----------------------------------------------------------------------*/
/* DS1202 and DS1302 Serial Real Time Clock driver                       */
/* 2014-12-30 Will Sowerbutts                                            */
/*-----------------------------------------------------------------------*/

#define _DS1302_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <ds1302.h>

/****************************************************************************/
/* Code in this file is used only once, at startup, so we want it to live   */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file.                                                        */
/****************************************************************************/

void ds1302_write_register(uint8_t reg, uint8_t val)
{
    ds1302_set_pin_ce(true);
    ds1302_send_byte(reg);
    ds1302_send_byte(val);
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
}

void ds1302_write_seconds(uint8_t seconds)
{
    irqflags_t irq = di();
    ds1302_write_register(0x8E, 0x00);    /* write to control register: disable write-protect */
    ds1302_write_register(0x80, seconds); /* write to seconds register (bit 7 set: halts clock) */
    ds1302_write_register(0x8E, 0x80);    /* write to control register: enable write-protect */
    irqrestore(irq);
}

void ds1302_check_rtc(void)
{
    uint8_t buffer[7];

    ds1302_read_clock(buffer, 7); /* read all calendar data */

    if(buffer[0] & 0x80){ /* is the clock halted? */
        kputs("ds1302: start clock\n");
        ds1302_write_seconds(buffer[0] & 0x7F); /* start it */
    }
}

void ds1302_init(void)
{
    time_t tod;

    /* initialise the hardware into a sensible state */
    ds1302_set_pin_data_driven(true);
    ds1302_set_pin_data(false);
    ds1302_set_pin_ce(false);
    ds1302_set_pin_clk(false);
    ds1302_check_rtc();
    wrtime(&tod);
}
