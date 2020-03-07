/**
 * @file kprintf.c
 * TA-BOT:MAILTO noah.asaria@marquette.edu m.price@marquette.edu
 */

/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */

#include <xinu.h>

#define UNGETMAX 10             /* Can un-get at most 10 characters. */

static unsigned char ungetArray[UNGETMAX];
int bufp = 0;
/**
 * Synchronously read a character from a UART.  This blocks until a character is
 * available.  The interrupt handler is not used.
 *
 * @return
 *      The character read from the UART as an <code>unsigned char</code> cast
 *      to an <code>int</code>.
 */
syscall lock_acquire(spinlock_t lock);
syscall lock_release(spinlock_t lock);
syscall kgetc(void)
{
    volatile struct pl011_uart_csreg *regptr;

    /* Pointer to the UART control and status registers.  */
    regptr = (struct pl011_uart_csreg *)0x3F201000;
   
    unsigned ch;
    // TODO: First, check the unget buffer for a character.
    //       Otherwise, check UART flags register, and
    //       once the receiver is not empty, get character c.

    /* Checking the unget buffer for a character */
    if (bufp > 0) {
        return ungetArray[--bufp];
    }
    else {
	while(PL011_FR_RXFE && ((regptr->fr >> 4) & 0x01)) { } 
	ch = (regptr->dr & 0xFF);
	return (int) ch;
		
    }
    return SYSERR;
}

/**
 * kcheckc - check to see if a character is available.
 * @return true if a character is available, false otherwise.
 */
syscall kcheckc(void)
{
    volatile struct pl011_uart_csreg *regptr;
    regptr = (struct pl011_uart_csreg *)0x3F201000;

    // TODO: Check the unget buffer and the UART for characters.
    if (bufp > 0) return 1;
    if (regptr->dr) return 1;	
    return 0;
    return SYSERR;
}

/**
 * kungetc - put a serial character "back" into a local buffer.
 * @param c character to unget.
 * @return c on success, SYSERR on failure.
 */
syscall kungetc(unsigned char c)
{
    // TODO: Check for room in unget buffer, put the character in or discard.
	if (bufp < UNGETMAX) {
		ungetArray[bufp++] = c;
		return c;
	}
	return SYSERR;
}


/**
 * Synchronously write a character to a UART.  This blocks until the character
 * has been written to the hardware.  The interrupt handler is not used.
 *
 * @param c
 *      The character to write.
 *
 * @return
 *      The character written to the UART as an <code>unsigned char</code> cast
 *      to an <code>int</code>.
 */
syscall kputc(uchar c)
{
    volatile struct pl011_uart_csreg *regptr;    
    /* Pointer to the UART control and status registers.  */
    regptr = (struct pl011_uart_csreg *)0x3F201000;
    // TODO: Check UART flags register.
    //       Once the Transmitter FIFO is not full, send character c.
   
    while (PL011_FR_TXFF && ((regptr->fr >> 5) & 0x01)) { }
    regptr->dr = c;
    return (int) c;
    return SYSERR;
}

/**
 * kernel printf: formatted, synchronous output to SERIAL0.
 *
 * @param format
 *      The format string.  Not all standard format specifiers are supported by
 *      this implementation.  See _doprnt() for a description of supported
 *      conversion specifications.
 * @param ...
 *      Arguments matching those in the format string.
 *
 * @return
 *      The number of characters written.
 */
syscall kprintf(const char *format, ...)
{
    int retval;
    va_list ap;
 
    lock_acquire(serial_lock);

    va_start(ap, format);
    retval = _doprnt(format, ap, (int (*)(int, int))kputc, 0);
    va_end(ap);

    lock_release(serial_lock);
    return retval;
}
