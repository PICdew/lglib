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

#include "font.h"
#include <string.h>
#include "lg.h"

#define LG_MAX_STRINGS		10

typedef struct LG_LABEL
{
	unsigned char* string;
	char visible;
	char in_use;
	unsigned char size;
	unsigned char spacing;
	unsigned char* font_data;
	size_t length;
	uint16_t x;
	uint16_t y;
	LG_RGB color;
}
LG_LABEL;

LG_LABEL labels[LG_MAX_STRINGS];
LG_RGB background;
static LG_DISPLAY_PAINT paint;
static LG_DISPLAY_PAINT_PARTIAL paint_partial;

/*
// prototypes
*/
LG_RGB lg_get_string_pixel(LG_LABEL str, uint16_t x, uint16_t y, LG_RGB background);

/*
// initializes the lite gui library
*/
void lg_init(LG_DISPLAY_PAINT display_paint, LG_DISPLAY_PAINT_PARTIAL display_paint_partial)
{
	paint = display_paint;
	paint_partial = display_paint_partial;	
}

/*
// sets the screen background color
*/
void lg_set_background(LG_RGB color)
{
	background = color;	
	paint();
}

/*
// adds a label to the display
*/
int16_t lg_label_add(unsigned char* str, unsigned char* font, 
	unsigned char font_size, unsigned char spacing, LG_RGB color, uint16_t x, uint16_t y)
{
	int16_t i;
	for (i = 0; i < LG_MAX_STRINGS; i++)
	{
		if (!labels[i].in_use)
		{
			labels[i].string = str;
			labels[i].x = x;
			labels[i].y = y;
			labels[i].length = strlen((char*)str);
			labels[i].size = font_size;
			labels[i].spacing = spacing;
			labels[i].color = color;
			labels[i].visible = 1;
			labels[i].in_use = 1;
			
			paint_partial(labels[i].x, labels[i].y, labels[i].length * ((8 + labels[i].spacing) * labels[i].size), 8 * labels[i].size);
			
			return i;
		}
	}
	return - 1;
}

/*
// changes the label string
*/
void lg_label_set_string(uint16_t index, unsigned char* string)
{
	int newlen = strlen((char*)string);
	labels[index].string = string;
	
	if (newlen > labels[index].length)
		labels[index].length = newlen;
	
	paint_partial(labels[index].x, labels[index].y, 
		labels[index].length * ((8 + labels[index].spacing) * labels[index].size), 8 * labels[index].size);
	labels[index].length = newlen;
}

/*
// changes the label visibility
*/
void lg_label_set_visibility(uint16_t index, char visible)
{
	if (labels[index].visible != visible)
	{
		labels[index].visible = visible;
		/*
		// repaint
		*/
		paint_partial(labels[index].x, labels[index].y, 
			labels[index].length * ((8 + labels[index].spacing) * labels[index].size), 8 * labels[index].size);
	}
}

/*
// sets the label color
*/
void lg_label_set_color(uint16_t index, LG_RGB color)
{
	if (labels[index].color != color)
	{
		/*
		// update label color
		*/ 
		labels[index].color = color;
		/*
		// repaint
		*/
		paint_partial(labels[index].x, labels[index].y, 
			labels[index].length * ((8 + labels[index].spacing) * labels[index].size), 8 * labels[index].size);
	}
}

/*
// gets the value of a pixel, driver must call this function
*/
LG_RGB lg_get_pixel(uint16_t x, uint16_t y)
{
	static int i;
	LG_RGB pixel;
	
	pixel = background;
	
	for (i = 0; i < LG_MAX_STRINGS; i++)
	{
		if (labels[i].visible)
		{
			pixel = lg_get_string_pixel(labels[i], x, y, pixel);
		
		}
	}
	
	return pixel;
}

/*
// gets the value of a label pixel
*/
LG_RGB lg_get_string_pixel(LG_LABEL str, uint16_t x, uint16_t y, LG_RGB background)
{
	static uint16_t str_pos;
	static uint16_t char_x;
	static uint16_t char_y;
	/*
	// if the label is invisible return the background color
	*/
	if (!str.visible)
		return background;
	/*
	// if the pixel is not within this label return background color
	*/
	if (x < str.x) return background;
	if (y < str.y) 
	{
		return background;
	}	
	else if (y >= (str.y + (8 * str.size)))
	{
		return background;
	}
	/*
	// find the position of the current char on the string
	*/
	str_pos = (x - str.x) / ((8 + str.spacing) * str.size);
	/*
	// if we've reached the end of the string return the
	// background color
	*/
	if (str_pos >= str.length)
		return background;
	/*
	// calculate position of the pixel within the font
	*/
	char_x = ((x - str.x) - (str_pos * ((8 + str.spacing) * str.size))) / str.size;
	char_y = (y - str.y) / str.size; 
	/*
	// return the pixel value
	*/	
	if ((font[str.string[str_pos]][char_y] << char_x) & 0x80)
	{
		return str.color;
	}
	else
	{	
		return background;
	}	
}

