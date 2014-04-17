/*
 * ili9341 - ILI9341 SPI LCD Driver for dsPIC
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

#include "compiler.h"

#ifndef ILI9341
#define ILI9341

void ili9341_init();
void ili9341_paint();
void ili9341_paint_partial(int16_t x, int16_t y, int16_t width, int16_t height);
char ili9341_is_painting();
void ili9341_do_processing();
void ili9341_sleep(void);
void ili9341_wake(void);
void ili9341_display_on(void);
void ili9341_display_off(void);


#endif
