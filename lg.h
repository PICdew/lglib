/*
 * lglib - Lightweight Graphics Library for Embedded Systems
 * Copyright (C) 2013 Fernando Rodriguez (support@fernansoft.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef FONT_H
#define FONT_H
#include <stdlib.h>
#include "compiler.h"

#define FONT_COMPILE_W_FONT

typedef uint32_t LG_RGB;

/*
typedef struct LG_RGB
{
	unsigned char a;
	unsigned char b;
	unsigned char g;
	unsigned char r;
}
LG_RGB;
*/

typedef void (*LG_DISPLAY_PAINT)(void);
typedef void (*LG_DISPLAY_PAINT_PARTIAL)(uint16_t x, uint16_t y, uint16_t width, uint16_t height);


/**
 * <summary>Initializes LiteGUI</summary>
 */
void lg_init
(
	LG_DISPLAY_PAINT display_paint, 
	LG_DISPLAY_PAINT_PARTIAL display_paint_partial
);

/**
 * <summary>Sets the background color of the display.</summary>
 */
void lg_set_background
(
	LG_RGB color
);

/**
 * <summary>Gets the value of a pixel
 */
LG_RGB lg_get_pixel
(
	uint16_t x, 
	uint16_t y
);

/**
 * <summary>Adds a label to the display.</summary>
 */
int16_t lg_label_add
(
	unsigned char* str, 
	unsigned char* font, 
	unsigned char font_size, 
	unsigned char spacing, 
	LG_RGB color, 
	uint16_t x, 
	uint16_t y
);

/**
 * <summary>Sets the string of a label.</summary>
 */
void lg_label_set_string
(
	uint16_t index, 
	unsigned char* string
);

/**
 * <summary>Sets the visibility of a label.</summary>
 */
void lg_label_set_visibility
(
	uint16_t index, 
	char visible
);

/**
 * <summary>Sets the color of  a label.</summary>
 */
void lg_label_set_color(uint16_t index, LG_RGB color);

#endif
