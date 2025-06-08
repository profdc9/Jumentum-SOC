/* Jumentum-SOC

  Copyright (C) 2007 by Daniel Marks

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  
  Daniel L. Marks profdc9@gmail.com

*/
/*******************************************************************/


#include <stdio.h>
#include <stdlib.h>

//#include "all.h"
#include "serialunix.h"
#include "cpu.h"

#ifdef STDOUTSER0
char console_uart = 0;
#endif
#ifdef STDOUTSER1
char console_uart = 1;
#endif

#ifdef WIN32
#include <windows.h>

HANDLE hStdIn;

int lastkey;

void init_keyboard()
{
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hStdIn,0);
	lastkey = -1;
}

int xxct=0;

int mykbhit(void)
{
	unsigned long events;
	int bSuccess, key;
	unsigned long nrec;
	INPUT_RECORD ip;
	
	bSuccess = GetNumberOfConsoleInputEvents(hStdIn, &events);
	/* printf("bSuccess=%d events=%d ct=%d\n",bSuccess,events,++xxct); */
	if (bSuccess == 0) {
		printf("PeekConsoleInput error\n");
		return 0;
	}
	if (events == 0)
		return 0;
	bSuccess=ReadConsoleInput(hStdIn,&ip,1,&nrec);	
	if ( (!bSuccess) || (nrec == 0)) return -1;
	/* printf("event=%d key=%d\n",ip.EventType,ip.Event.KeyEvent.uChar.AsciiChar); */
	if (ip.EventType != KEY_EVENT)
		return 0;
	key = ip.Event.KeyEvent.uChar.AsciiChar;
	if (((key<1)||(key>127)) || (!ip.Event.KeyEvent.bKeyDown))
		return 0;
	lastkey = key;
	return 1;
}

int mygetch(void)
{
	int ch = lastkey;
	lastkey = -1;
	return ch;
}

#endif

#ifdef linux
#include <termios.h>
#include <unistd.h>   // for read()

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int mykbhit()
{
unsigned char ch;
int nread;

    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int mygetch()
{
char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}
#endif

/* Initialize Serial Interface UART0 */
static void init_serial0 ( unsigned long baudrate, int databits, int stopbits, int parity )
{
  init_keyboard();
}

/* Write character to Serial Port 0 without \n -> \r\n  */
static int putc_serial0 (int ch)
{
  write(1,&ch,sizeof(char));
}

/* Write character to Serial Port 0 with \n -> \r\n  */
static int putchar_serial0 (int ch)
{
    if (ch == '\n') putc_serial0('\r');
	return putc_serial0(ch);
}


void putstring_serial0 (const char *string)
{
	char ch;

	while ((ch = *string)) {
		putchar_serial0(ch);
		string++;
	}
}

/******************************************************************************
 *
 * Function Name: uart0Getch()
 *
 * Description:  
 *    This function gets a character from the UART receive queue
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    character on success, -1 if no character is available
 *
 *****************************************************************************/
int getkey_serial0(void)
{
 if (!mykbhit())
	return -1; 
  return mygetch();
}

/* Wait for  character from Serial Port   */
int waitkey_serial0 (void)
{
    int ch;
    while ((ch=getkey_serial0()) == -1);
	return ch;
}


/* Initialize Serial Interface UART1 */
void init_serial1 ( unsigned long baudrate, int databits, int stopbits, int parity )
{
}

/* Write character to Serial Port 0 without \n -> \r\n  */
int putc_serial1 (int ch)
{
}


/* Write character to Serial Port 0 with \n -> \r\n  */
int putchar_serial1 (int ch)
{
}

void putstring_serial1 (const char *string)
{
}

int getkey_serial1(void)
{
}

/* Wait for  character from Serial Port   */
int waitkey_serial1 (void)
{
}

/* Holder functions for future expansion to multiple serial ports */
int putchar_serial_c(int com, int ch)
{
	switch (com) {
		case 0: return putchar_serial0(ch);
		case 1: return putchar_serial1(ch);
	}
	return -1;
}

int putc_serial_c(int com, int ch)
{
	switch (com) {
		case 0: return putc_serial0(ch);
		case 1: return putc_serial1(ch);
	}
	return -1;
}

int getkey_serial_c(int com)
{
  switch (com) {
	  case 0:  return getkey_serial0();
	  case 1:  return getkey_serial1();
  }
  return -1;
}

int waitkey_serial_c(int com)
{
  switch (com) {
	  case 0:  return waitkey_serial0();
	  case 1:  return waitkey_serial1();
  }
  return -1;
}

void init_serial_c (int com, unsigned long baudrate, int databits, int stopbits, int parity )
{
  switch (com) {
	  case 0: init_serial0(baudrate,databits,stopbits,parity);
			  break;
	  case 1: init_serial1(baudrate,databits,stopbits,parity);
			  break;
  }
  return;
}

void putstring_serial_c (int com, const char *string)
{
  switch (com) {
		case 0:  putstring_serial0(string);
				 return;
		case 1:  putstring_serial1(string);
				 return;
  }
}
