/*
 *	Implement the usermem services. Can then use either C or asm code for
 *	the block transfers. This code can be banked as part of the kernel if
 *	using the asm helpers but must be in commonmem if not
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

/* Flat mode has to use its own valaddr */
#ifndef CONFIG_FLAT

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const char *base, usize_t size)
{
	/* FIXME: for Z80 we should make this a udata field so that cp/m
	   emulation alone can touch below 0x100 */
	if (!base || base < (const char *)PROGBASE || base + size < base)
		size = 0;
	else if (base + size > (const char *)(size_t)udata.u_top)
		size = (char *)(size_t)udata.u_top - base;
	if (size == 0)
		udata.u_error = EFAULT;
	return size;
}
#endif


int uget(const void *user, void *dst, usize_t count)
{
	if (!valaddr(user,count))
		return -1;
	return _uget(user,dst,count);
}

int16_t ugetc(const void *user)
{
	if (!valaddr(user, 1))
		return -1;
	return _ugetc(user);
}

uint16_t ugetw(const void *user)
{
	if (!valaddr(user, 2))
		return -1;
	return _ugetw(user);
}

#if ARGT_IS_BIGGER_THAN_INT
arg_t ugeta(const void *user)
{
	if (!valaddr(user, sizeof(arg_t)))
		return -1;
	return _ugeta(user);
}
#endif

/* ugets is a bit odd - we don't know the length of the passed string
   so we trim to the end of the allowed memory and if we don't find a
   \0 in time we error */
int ugets(const void *user, void *dest, usize_t maxlen)
{
	int ret;
	maxlen = valaddr(user, maxlen);
	if (!maxlen)
		return -1;
	ret = _ugets(user, dest, maxlen);
	if (ret == -1)
		udata.u_error = EFAULT;
	return ret;
}

int uput(const void *source,   void *user, usize_t count)
{
	if (!valaddr(user, count))
		return -1;
	return _uput(source,user,count);
}

int uputc(uint16_t value,  void *user)
{
	if (!valaddr(user, 1))
		return -1;
	/* u16_t so we don't get wacky 8bit stack games on SDCC */
	return _uputc(value,user);
}

int uputw(uint16_t value, void *user)
{
	if (!valaddr(user, 2))
		return -1;
	return _uputw(value,user);
}

#if ARGT_IS_BIGGER_THAN_INT
int uputa(arg_t value, void *user)
{
	if (!valaddr(user, sizeof(arg_t)))
		return -1;
	return _uputa(value,user);
}
#endif

int uzero(void *user, usize_t count)
{
	if (!valaddr(user, count))
		return -1;
	return _uzero(user,count);
}

/*
 *	Optional C language implementation for porting to new processors
 *	or where asm isn't needed
 */
#ifdef CONFIG_USERMEM_C

usize_t _uget(const uint8_t *user, uint8_t *dest, usize_t count)
{
	uint8_t tmp;
	while(count--) {
		BANK_PROCESS;
		tmp = *user++;
		BANK_KERNEL;
		*dest++ = tmp;
	}
	return 0;
}

int16_t _ugetc(const uint8_t *user)
{
	uint8_t tmp;
	BANK_PROCESS;
	tmp = *user;
	BANK_KERNEL;
	return tmp;
}

uint16_t _ugetw(const uint16_t *user)
{
	uint16_t tmp;
	BANK_PROCESS;
	tmp = *user;
	BANK_KERNEL;
	return tmp;
}

#if ARGT_IS_BIGGER_THAN_INT
arg_t _ugeta(const arg_t *user)
{
	arg_t tmp;
	BANK_PROCESS;
	tmp = *user;
	BANK_KERNEL;
	return tmp;
}
#endif

int _ugets(const uint8_t *user, uint8_t *dest, usize_t count)
{
	uint8_t tmp;
	while(count--) {
		BANK_PROCESS;
		tmp = *user++;
		BANK_KERNEL;
		*dest++ = tmp;
		if (tmp == '\0')
			return 0;
	}
	/* Ensure terminated */
	dest[-1] = '\0';
	return -1;
}

int _uput(const uint8_t *source, uint8_t *user, usize_t count)
{
	uint8_t tmp;
	while(count--) {
		tmp = *source++;
		BANK_PROCESS;
		*user++ = tmp;
		BANK_KERNEL;
	}
	return 0;

}

int _uputc(uint16_t value,  uint8_t *user)
{
	BANK_PROCESS;
	*user = value;
	BANK_KERNEL;
	return 0;
}

int _uputw(uint16_t value,  uint16_t *user)
{
	BANK_PROCESS;
	*user = value;
	BANK_KERNEL;
	return 0;
}

#if ARGT_IS_BIGGER_THAN_INT
int _uputa(arg_t value,  arg_t *user)
{
	BANK_PROCESS;
	*user = value;
	BANK_KERNEL;
	return 0;
}
#endif

int _uzero(uint8_t *user, usize_t count)
{
	BANK_KERNEL;
	while(count--)
		*user++=0;
	BANK_PROCESS;
	return 0;
}

#endif
#ifdef CONFIG_USERMEM_DIRECT

/* Systems where all memory is always mapped for live processes and kernel */

usize_t _uget(const uint8_t *user, uint8_t *dest, usize_t count)
{
	memcpy(dest, user, count);
	return 0;
}

int16_t _ugetc(const uint8_t *user)
{
	return *user;
}

uint16_t _ugetw(const uint16_t *user)
{
	return *user;
}

#if ARGT_IS_BIGGER_THAN_INT
arg_t _ugeta(const arg_t *user)
{
	return *user;
}
#endif

int _ugets(const uint8_t *user, uint8_t *dest, usize_t count)
{
	while(count--) {
		*dest = *user++;
		if (*dest == '\0')
			return 0;
		dest++;
	}
	/* Ensure terminated */
	dest[-1] = '\0';
	return -1;
}

int _uput(const uint8_t *source, uint8_t *user, usize_t count)
{
	memcpy(user, source, count);
	return 0;
}

int _uputc(uint16_t value,  uint8_t *user)
{
	*user = value;
	return 0;
}

int _uputw(uint16_t value,  uint16_t *user)
{
	*user = value;
	return 0;
}

#if ARGT_IS_BIGGER_THAN_INT
int _uputa(arg_t value, arg_t *user)
{
	*user = value;
	return 0;
}
#endif

int _uzero(uint8_t *user, usize_t count)
{
	memset(user, 0, count);
	return 0;
}
#endif
