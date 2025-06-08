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

#ifndef _VT100_H
#define _VT100_H

#define loc_in_virtscreen(cur,y,x) (((cur)->data)+(((y)*((cur)->columns))+(x)))
#define MAX_ANSI_ELEMENTS 16
#define char_to_virtscreen(cur,ch) (((cur)->next_char_send)((cur),(ch)))

#undef VTATTRIB

#ifdef VTATTRIB
typedef short screenchartype;
#else
typedef unsigned char screenchartype;
#endif

typedef struct _virtscreen
{
  int rows;
  int columns; 
  screenchartype *data;

  int xpos;
  int ypos;
  int top_scroll;
  int bottom_scroll;
  int attrib;
  int old_attrib;
  int old_xpos;
  int old_ypos;

  int cur_ansi_number;
  int ansi_elements;
  int ansi_reading_number;
  int ansi_element[MAX_ANSI_ELEMENTS];

  void (*next_char_send)(struct _virtscreen *cur, unsigned char ch);
} virtscreen;

extern virtscreen *newscr;

int init_virtscreen(virtscreen *cur, void *buf, int rows, int columns);
void outscreen(virtscreen *cur);
void update_virt_screen(virtscreen *newscr, unsigned char ch);

#endif  /* _VT100_H */
