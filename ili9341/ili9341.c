/*
 * ili9341 - ILI9341 SPI LCD Driver for Embedded Systems
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

#include <spi.h> 
#include <rtc.h>
#include "ili9341.h"
#include <lg.h>

typedef struct ILI9341_PARTIAL_PAINT
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned char pending;
}
ILI9341_PARTIAL_PAINT;

#define ILI9341_PARTIAL_PAINT_LIMIT 32U	/* MUST BE POWER OF 2 */
#define ILI9341_ENQUEUE_PAINT_REQUESTS

static unsigned char painting;
static unsigned int x;
static unsigned int y;
static unsigned int x_start;
static unsigned int x_end;
static unsigned int y_start;
static unsigned int y_end;
static unsigned char current_byte = 0;

#if defined(ILI9341_ENQUEUE_PAINT_REQUESTS)
	static ILI9341_PARTIAL_PAINT partial_paint[ILI9341_PARTIAL_PAINT_LIMIT];
	static unsigned int partial_paint_index;
	static unsigned int partial_paint_head;
#endif

/*
// configuration
*/
#define ILI9341_GET_PIXEL(x, y)			lg_get_pixel(x, y)			/* function to get pixel value */
#define ILI9341_ASSERT_CS()						/* assert chip-select macro */
#define ILI9341_DEASSERT_CS()					/* de-assert chip-select macro */
#define ILI9341_ASSERT_DATA()			IO_PIN_WRITE(B, 10, 1); Nop(); Nop()
#define ILI9341_ASSERT_CMD()			IO_PIN_WRITE(B, 10, 0); Nop(); Nop()		/* assert command line */
#define ILI9341_ASSERT_RESET()			IO_PIN_WRITE(A, 2, 0); Nop()
#define ILI9341_DEASSERT_RESET()		IO_PIN_WRITE(A, 2, 1); Nop()
#define ILI9341_IO_INIT()				\
	IO_PIN_WRITE(A, 2, 1);				\
	IO_PIN_WRITE(B, 10, 0);				\
	Nop();								\
	IO_PIN_SET_AS_OUTPUT(A, 2);			\
	IO_PIN_SET_AS_OUTPUT(B, 10)

#define ILI9341_CMD_POWER_CONTROL_A							(0x3B)
#define ILI9341_CMD_POWER_CONTROL_B							(0xCF)
#define ILI9341_CMD_DRIVER_TIMING_CONTROL_A					(0xE8)
#define ILI9341_CMD_DRIVER_TIMING_CONTROL_B					(0xEA)
#define ILI9341_CMD_POWER_ON_SEQ_CONTROL					(0xED)
#define ILI9341_CMD_PUMP_RATIO_CONTROL						(0xF7)
#define ILI9341_CMD_POWER_CONTROL_1							(0xC0)
#define ILI9341_CMD_POWER_CONTROL_2							(0xC1)
#define ILI9341_CMD_VCOM_CONTROL_1							(0xC5)
#define ILI9341_CMD_VCOM_CONTROL_2							(0xC7)
#define ILI9341_CMD_MEMORY_ACCESS_CONTROL					(0x36)
#define ILI9341_CMD_COLMOD									(0x3A)
#define ILI9341_FRAME_RATE_CONTROL							(0xB1)
#define ILI9341_DISPLAY_FUNCTION_CONTROL					(0xB6)
#define ILI9341_CMD_ENTER_SLEEP_MODE						(0x10)
#define ILI9341_SLEEP_OUT									(0x11)
#define ILI9341_CMD_DISPLAY_OFF								(0x28)
#define ILI9341_CMD_DISPLAY_ON								(0x29)
#define ILI9341_COLUMN_ADDRESS_SET							(0x2A)
#define ILI9341_PAGE_ADDRESS_SET							(0x2B)
#define ILI9341_MEMORY_WRITE								(0x2C)
#define ILI9341_CMD_PARTIAL_AREA							(0x30)
#define ILI9341_CMD_PARTIAL_MODE_ON							(0x12)
#define ILI9341_CMD_NORMAL_DISPLAY_ON						(0x13)
#define ILI9341_CMD_RESET									(0x01)
#define ILI9341_CMD_BACKLIGHT_CONTROL_8						(0xBF)
#define ILI9341_CMD_WRITE_DISPLAY_BRIGHTNESS				(0x51)
#define ILI9341_CMD_WRITE_CTRL_DISPLAY						(0x53)
#define ILI9341_CMD_3GAMMA_ENABLE							(0xF2)
#define ILI9341_CMD_GAMMA_SET								(0x26)
#define ILI9341_CMD_POSITIVE_GAMMA_CORRECTION				(0xE0)
#define ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION				(0xE1)


/*
// memory access command arguments
*/
#define ILI9341_CMD_MEMORY_ACCESS_INVERT_ROW_ORDER			(0b10000000)
#define ILI9341_CMD_MEMORY_ACCESS_INVERT_COL_ORDER			(0b01000000)
#define ILI9341_CMD_MEMORY_ACCESS_HORIZONTAL				(0b00100000)
#define ILI9341_CMD_MEMORY_ACCESS_INVERT_VERTICAL_REFRESH	(0b00010000)
#define ILI9341_CMD_MEMORY_ACCESS_BGR						(0b00001000)
#define ILI9341_CMD_MEMORY_ACCESS_INVERT_HORIZONTAL_REFRESH	(0b00000100)

/*
// backlight control 8 command arguments
*/
#define ILI9341_BACKLIGHT_CONTROL_8_LEDON					(0b00000111)
#define ILI9341_BACKLIGHT_CONTROL_8_LEDOFF					(0b00000101)

/*
// CTRL Display command arguments
*/
#define ILI9341_CTRL_DISPLAY_BRIGHTNESS_CONTROL_ON			(0b00100000)
#define ILI9341_CTRL_DISPLAY_DIMMING_ON						(0b00001000)
#define ILI9341_CTRL_DISPLAY_BACKLIGHT_ON					(0b00000100)


/*
// color mode values
*/
#define ILI9341_COLMOD_16BIT								(0b01010000)
#define ILI9341_COLMOD_18BIT								(0b01100000)

/*
// screen dimensions
*/
#define LCD_SCREEN_WIDTH 320
#define LCD_SCREEN_HEIGHT 240

static unsigned char delay_temp;

/*
// macro for writing data via spi
*/
#define LCD_WRITE_DATA(data)				\
{											\
	for(delay_temp=0; delay_temp < 0x7f; delay_temp++);								\
	ILI9341_ASSERT_DATA();					\
	spi_write(SPI_GET_MODULE(2), data);		\
}

#define LCD_WRITE_DATA_ASYNC(data)				\
{											\
	ILI9341_ASSERT_DATA();					\
	spi_write_async(SPI_GET_MODULE(2), data);		\
}

/*
// macro for sending commands via spi
*/
#define LCD_WRITE_CMD(cmd)					\
{											\
	for(delay_temp=0; delay_temp < 0x7f; delay_temp++);								\
	ILI9341_ASSERT_CMD();					\
	spi_write(SPI_GET_MODULE(2), cmd);		\
}


/*
// prototypes
*/
void ili9341_set_address(uint16_t x1, uint16_t y1, uint16_t x2,uint16_t y2);

/*
// send set address command
*/
void ili9341_set_address(uint16_t x1, uint16_t y1, uint16_t x2,uint16_t y2)
{	
	LCD_WRITE_CMD(ILI9341_COLUMN_ADDRESS_SET);
	LCD_WRITE_DATA(x1 >> 8);
	LCD_WRITE_DATA(x1);
	LCD_WRITE_DATA(x2 >> 8);
	LCD_WRITE_DATA(x2);
 
	LCD_WRITE_CMD(ILI9341_PAGE_ADDRESS_SET);
	LCD_WRITE_DATA(y1 >> 8);
	LCD_WRITE_DATA(y1);
	LCD_WRITE_DATA(y2 >> 8);
	LCD_WRITE_DATA(y2);    				 
	LCD_WRITE_CMD(ILI9341_MEMORY_WRITE);
}

/*
// request a full screen paint
*/
void ili9341_paint()
{
	if (!painting)
	{
		painting = 1;
		x = 0;
		y = 0;
		x_end = LCD_SCREEN_WIDTH;
		y_end = LCD_SCREEN_HEIGHT;
		current_byte = 0;
		ili9341_set_address(x, y, x_end - 1, y_end - 1);
	}
	else
	{
		#if defined(ILI9341_ENQUEUE_PAINT_REQUESTS)
			_ASSERT(partial_paint[partial_paint_head].pending == 0);
			partial_paint[partial_paint_head].x = 0;
			partial_paint[partial_paint_head].y = 0;
			partial_paint[partial_paint_head].width = LCD_SCREEN_WIDTH;
			partial_paint[partial_paint_head].height = LCD_SCREEN_HEIGHT;
			partial_paint[partial_paint_head].pending = 1;
			partial_paint_head++;
			//partial_paint_head &= (ILI9341_PARTIAL_PAINT_LIMIT - 1);
			if (partial_paint_head == ILI9341_PARTIAL_PAINT_LIMIT)
				partial_paint_head = 0;
		#endif	
	}	
}

/*
// request a partial paint
*/
void ili9341_paint_partial(int16_t x_pos, int16_t y_pos, int16_t width, int16_t height)
{
	if (painting)
	{
		#if defined(ILI9341_ENQUEUE_PAINT_REQUESTS)
			_ASSERT(partial_paint[partial_paint_head].pending == 0);
			partial_paint[partial_paint_head].x = x_pos;
			partial_paint[partial_paint_head].y = y_pos;
			partial_paint[partial_paint_head].width = width;
			partial_paint[partial_paint_head].height = height;
			partial_paint[partial_paint_head].pending = 1;
			partial_paint_head++;
			//partial_paint_head &= (ILI9341_PARTIAL_PAINT_LIMIT - 1);
			if (partial_paint_head == ILI9341_PARTIAL_PAINT_LIMIT)
				partial_paint_head = 0;
		#else
			if ((x_pos < x_start || x_pos + width > x_end) ||
				(y_pos < y_start || y_pos + height > y_end))			
			{
				x_start = x = MIN(x_start, x_pos);
				y_start = y = MIN(y_start, y_pos);
				x_end = MAX(x_end, x_pos + width);
				y_end = MAX(y_end, y_pos + height);
				/*
				// set the partial area
				*/
				current_byte = 0;
				ili9341_set_address(x, y, x_end - 1, y_end - 1);
			}
		#endif
	}
	else
	{
		painting = 1;
		x = x_pos;
		y = y_pos;
		x_start = x;
		y_start = y;
		x_end = x + width;
		y_end = y + height;
		/*
		// set the partial area
		*/
		ili9341_set_address(x, y, x_end - 1, y_end - 1);
	}
}

/*
// turn display off
*/
void ili9341_display_off(void)
{
	LCD_WRITE_CMD(ILI9341_CMD_DISPLAY_OFF);
}

/*
// turn display on
*/
void ili9341_display_on(void)
{
	LCD_WRITE_CMD(ILI9341_CMD_DISPLAY_ON);
}

/*
// put lcd driver in sleep mode
*/
void ili9341_sleep(void)
{
	/*
	// turn backlight off
	*/
	LCD_WRITE_CMD(ILI9341_CMD_WRITE_CTRL_DISPLAY);
	LCD_WRITE_DATA(
		ILI9341_CTRL_DISPLAY_BRIGHTNESS_CONTROL_ON |
		ILI9341_CTRL_DISPLAY_DIMMING_ON);

	LCD_WRITE_CMD(ILI9341_CMD_WRITE_DISPLAY_BRIGHTNESS);
	LCD_WRITE_DATA(0x01)
	//LCD_WRITE_CMD(ILI9341_CMD_BACKLIGHT_CONTROL_8);
	//LCD_WRITE_DATA(ILI9341_BACKLIGHT_CONTROL_8_LEDOFF);
	/*
	// enter sleep mode
	*/
	LCD_WRITE_CMD(ILI9341_CMD_DISPLAY_OFF);
	LCD_WRITE_CMD(ILI9341_CMD_ENTER_SLEEP_MODE); 
	rtc_sleep(5);
}

/*
// wake up from sleep
*/
void ili9341_wake(void)
{
	/*
	// turn backlight on
	*/
	LCD_WRITE_CMD(ILI9341_CMD_WRITE_CTRL_DISPLAY);
	LCD_WRITE_DATA(
		ILI9341_CTRL_DISPLAY_BRIGHTNESS_CONTROL_ON |
		ILI9341_CTRL_DISPLAY_DIMMING_ON | 
		ILI9341_CTRL_DISPLAY_BACKLIGHT_ON);

	//LCD_WRITE_CMD(ILI9341_CMD_WRITE_DISPLAY_BRIGHTNESS);
	//LCD_WRITE_DATA(0xFF)
	//LCD_WRITE_CMD(ILI9341_CMD_BACKLIGHT_CONTROL_8);
	//LCD_WRITE_DATA(ILI9341_BACKLIGHT_CONTROL_8_LEDON);
	/*
	// exit sleep mode
	*/
	LCD_WRITE_CMD(ILI9341_SLEEP_OUT); 
	rtc_sleep(100);
	LCD_WRITE_CMD(ILI9341_CMD_DISPLAY_ON);
}


char ili9341_is_painting()
{
	return painting;
}

/*
// performs the painting in the "background"
// must be called from application loop
*/
void ili9341_do_processing()
{
	static char pixel_fetched = 0;
	static uint32_t pixel_color;
		
	if (painting)
	{
		/*
		// get pixel from graphics library
		*/
		if (!pixel_fetched)
		{
			pixel_color = ILI9341_GET_PIXEL(x, y);
			pixel_color &= 0x7e7e7e;
			pixel_fetched = 1;
		}
		/*
		// if the spi is busy return
		*/
		if (!spi_ready(SPI_GET_MODULE(2)))
			return;
		/*
		// send pixel to LCD driver
		*/
		switch (current_byte)
		{
			case 0:
				LCD_WRITE_DATA_ASYNC(((unsigned char*) &pixel_color)[2]);
				current_byte = 1;
				break;
			case 1:			
				LCD_WRITE_DATA_ASYNC(((unsigned char*) &pixel_color)[1]);
				current_byte = 2;
				break;
			case 2:
				LCD_WRITE_DATA_ASYNC(((unsigned char*) &pixel_color)[0]);
				current_byte = 0;
				pixel_fetched = 0;
				
				x++;
				if (x >= x_end) 
				{
					x = x_start;
					y++;
					if (y >= y_end)
					{
						while (!SPI2STATbits.SPIRBF);
						painting = SPI2BUF;
						painting = 0;
					}
				}
				break;
		}
	}
	else
	{
		#if defined(ILI9341_ENQUEUE_PAINT_REQUESTS)
		if (partial_paint[partial_paint_index].pending)
		{
			ili9341_paint_partial
			(
				partial_paint[partial_paint_index].x,
				partial_paint[partial_paint_index].y,
				partial_paint[partial_paint_index].width,
				partial_paint[partial_paint_index].height
			);
			partial_paint[partial_paint_index].pending = 0;
			partial_paint_index++;
			//partial_paint_index &= ILI9341_PARTIAL_PAINT_LIMIT - 1;	
			if (partial_paint_index == ILI9341_PARTIAL_PAINT_LIMIT)
				partial_paint_index = 0;
		}
		#endif	
	}
}


void ili9341_init()
{
	/*
	// initialize IO pins
	*/
	ILI9341_IO_INIT();
	/*
	// initialize spi module
	*/
	spi_init(SPI_GET_MODULE(2));
	spi_set_clock(SPI_GET_MODULE(2), 9000000L);
	/*
	// reset
	*/
	ILI9341_ASSERT_RESET();
	rtc_sleep(1);
	ILI9341_DEASSERT_RESET();	
	/*
	// assert chip-select line
	*/
	ILI9341_ASSERT_CS();
	/*
	// send software reset command
	*/
	LCD_WRITE_CMD(ILI9341_CMD_RESET);
	rtc_sleep(5);
	LCD_WRITE_CMD(ILI9341_CMD_DISPLAY_OFF);
	/*
	// set power on seq control register
	*/
	LCD_WRITE_CMD(ILI9341_CMD_POWER_ON_SEQ_CONTROL);  
	LCD_WRITE_DATA(0x64); 
	LCD_WRITE_DATA(0x03); 
	LCD_WRITE_DATA(0X12); 
	LCD_WRITE_DATA(0X81); 
	/*
	// set timing control registers
	*/	
	LCD_WRITE_CMD(ILI9341_CMD_DRIVER_TIMING_CONTROL_A);  
	LCD_WRITE_DATA(0x85); 
	LCD_WRITE_DATA(0x00); 
	LCD_WRITE_DATA(0x78); 
	
	LCD_WRITE_CMD(ILI9341_CMD_DRIVER_TIMING_CONTROL_B);  
	LCD_WRITE_DATA(0x00); 
	LCD_WRITE_DATA(0x00); 
	/*
	// initialize power control registers
	*/
	LCD_WRITE_CMD(ILI9341_CMD_POWER_CONTROL_A);  
	LCD_WRITE_DATA(0x39); 
	LCD_WRITE_DATA(0x2C); 
	LCD_WRITE_DATA(0x00); 
	LCD_WRITE_DATA(0x34); 
	LCD_WRITE_DATA(0x02); 
	
	LCD_WRITE_CMD(ILI9341_CMD_POWER_CONTROL_B);  
	LCD_WRITE_DATA(0x00); 
	LCD_WRITE_DATA(0XC1); 
	LCD_WRITE_DATA(0X30); 
	
	LCD_WRITE_CMD(ILI9341_CMD_PUMP_RATIO_CONTROL);
	LCD_WRITE_DATA(0x20); 
	
	LCD_WRITE_CMD(ILI9341_CMD_POWER_CONTROL_1);
	LCD_WRITE_DATA(0x09);	/* 3.3V */
	
	LCD_WRITE_CMD(ILI9341_CMD_POWER_CONTROL_2);
	LCD_WRITE_DATA(0x01);	/* DDVDH = VCI*2 | VGH = VCI*7 | -VGL = VCI*3 */
	
	LCD_WRITE_CMD(ILI9341_CMD_VCOM_CONTROL_1);
	LCD_WRITE_DATA(0x18);	/* VCOMH = 3.3V */
	LCD_WRITE_DATA(0x64); 	/* VCOML = 0V */
	
	LCD_WRITE_CMD(ILI9341_CMD_VCOM_CONTROL_2);
	LCD_WRITE_DATA(0x9F);	/* VM = 1 | VCOMH = VMH | VCOML = VML  */
	/*
	// set video memory format to BGR/horizontal
	*/
	LCD_WRITE_CMD(ILI9341_CMD_MEMORY_ACCESS_CONTROL);
	LCD_WRITE_DATA(	
		ILI9341_CMD_MEMORY_ACCESS_INVERT_COL_ORDER | 
		ILI9341_CMD_MEMORY_ACCESS_INVERT_ROW_ORDER | 
		ILI9341_CMD_MEMORY_ACCESS_HORIZONTAL | 
		ILI9341_CMD_MEMORY_ACCESS_BGR);
	/*
	// set color mode to 18-bit
	*/
	LCD_WRITE_CMD(ILI9341_CMD_COLMOD);    
	LCD_WRITE_DATA(ILI9341_COLMOD_18BIT); 
	/*
	// set frame rate control
	*/	
	LCD_WRITE_CMD(ILI9341_FRAME_RATE_CONTROL);
	LCD_WRITE_DATA(0x00);  
	LCD_WRITE_DATA(0x18); 
	/*
	// set display function register
	*/
	LCD_WRITE_CMD(ILI9341_DISPLAY_FUNCTION_CONTROL);
	LCD_WRITE_DATA(0x0A); 
	LCD_WRITE_DATA(0x82);
	LCD_WRITE_DATA(0x27);  
	LCD_WRITE_DATA(0x00);
	/*
	// disable 3Gamma
	*/
	LCD_WRITE_CMD(ILI9341_CMD_3GAMMA_ENABLE);
	LCD_WRITE_DATA(0x00); 
	
	LCD_WRITE_CMD(ILI9341_CMD_GAMMA_SET);    //Gamma curve selected 
	LCD_WRITE_DATA(0x01); 
	
	LCD_WRITE_CMD(ILI9341_CMD_POSITIVE_GAMMA_CORRECTION);
	LCD_WRITE_DATA(0x0F); 
	LCD_WRITE_DATA(0x31); 
	LCD_WRITE_DATA(0x2B); 
	LCD_WRITE_DATA(0x0C); 
	LCD_WRITE_DATA(0x0E); 
	LCD_WRITE_DATA(0x08); 
	LCD_WRITE_DATA(0x4E); 
	LCD_WRITE_DATA(0xF1); 
	LCD_WRITE_DATA(0x37); 
	LCD_WRITE_DATA(0x07); 
	LCD_WRITE_DATA(0x10); 
	LCD_WRITE_DATA(0x03); 
	LCD_WRITE_DATA(0x0E); 
	LCD_WRITE_DATA(0x09); 
	LCD_WRITE_DATA(0x00); 
	
	LCD_WRITE_CMD(ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION);
	LCD_WRITE_DATA(0x00); 
	LCD_WRITE_DATA(0x0E); 
	LCD_WRITE_DATA(0x14); 
	LCD_WRITE_DATA(0x03); 
	LCD_WRITE_DATA(0x11); 
	LCD_WRITE_DATA(0x07); 
	LCD_WRITE_DATA(0x31); 
	LCD_WRITE_DATA(0xC1); 
	LCD_WRITE_DATA(0x48); 
	LCD_WRITE_DATA(0x08); 
	LCD_WRITE_DATA(0x0F); 
	LCD_WRITE_DATA(0x0C); 
	LCD_WRITE_DATA(0x31); 
	LCD_WRITE_DATA(0x36); 
	LCD_WRITE_DATA(0x0F); 
	/*
	// disable 3G
	*/
	LCD_WRITE_CMD(ILI9341_CMD_3GAMMA_ENABLE);
	LCD_WRITE_DATA(0x03);
	/*
	// set backlight control registers
	*/ 
	LCD_WRITE_CMD(ILI9341_CMD_BACKLIGHT_CONTROL_8);
	LCD_WRITE_DATA(0x04);

	/*
	// enable brightness control
	*/
	LCD_WRITE_CMD(ILI9341_CMD_WRITE_CTRL_DISPLAY);
	LCD_WRITE_DATA(0x20);
		/*ILI9341_CTRL_DISPLAY_BRIGHTNESS_CONTROL_ON |
		ILI9341_CTRL_DISPLAY_DIMMING_ON | 
		ILI9341_CTRL_DISPLAY_BACKLIGHT_ON);*/
	/*
	// set display brightness
	*/
	LCD_WRITE_CMD(ILI9341_CMD_WRITE_DISPLAY_BRIGHTNESS);
	LCD_WRITE_DATA(0xFF)
	/*
	// exit sleep mode
	*/
	LCD_WRITE_CMD(ILI9341_SLEEP_OUT); 
	rtc_sleep(100);
	/*
	// set video ram pointer to pixel (0,0) and define screen dimensions
	*/
	ili9341_set_address(0, 0, LCD_SCREEN_WIDTH - 1, LCD_SCREEN_HEIGHT - 1);
	LCD_WRITE_CMD(ILI9341_MEMORY_WRITE);
	/*
	// initialize variables to known values
	*/	
	x_start = 0;
	x = 0;
	y_start = 0;
	y = 0;
	x_end = LCD_SCREEN_WIDTH;
	y_end = LCD_SCREEN_HEIGHT;
	painting = 0;
}
