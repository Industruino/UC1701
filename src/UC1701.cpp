/*
 * UC7101 - Interface with UC7101 (or compatible) LCDs.
 *
 * Copyright (c) 2014 Rustem Iskuzhin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#define USE_UC1701_SPIMODE 1 // 1:Hardware SPI 0:software spi(shitout)
#define USE_BITBAND        1 // ARM-M3 ビットバンド機能利用 1:利用 0:利用しない

#include "UC1701.h"
#include <SPI.h>
#include <Arduino.h>
#include <libmaple/bitband.h>


/*
 * If this was a ".h", it would get added to sketches when using
 * the "Sketch -> Import Library..." menu on the Arduino IDE...
 */

#include "charset.cpp"

UC1701::UC1701(uint8_t sclk, uint8_t sid,
                 uint8_t cs1,
                 uint8_t a0):
    pin_sclk(sclk),
    pin_sid(sid),
    pin_cs1(cs1),
    pin_a0(a0)
{}

void UC1701::begin()
{
    this->width = 128;
    this->height = 64;

    this->column = 0;
    this->line = 0;
    // All pins are outputs 
    pinMode(this->pin_cs1, OUTPUT);
    pinMode(this->pin_a0, OUTPUT);
#if USE_UC1701_SPIMODE == 0    
    pinMode(this->pin_sclk, OUTPUT);
    pinMode(this->pin_sid, OUTPUT);
#endif
    // Reset the controller state...
    digitalWrite(this->pin_cs1, LOW);
    digitalWrite(this->pin_a0, LOW);
    digitalWrite(this->pin_cs1, HIGH);
#if  USE_UC1701_SPIMODE == 1
	SPI.begin();
	SPI.setBitOrder(MSBFIRST); 
  SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
#endif
    // Set the LCD parameters...
   this->Transfer_command(0xE2);  //System Reset
   this->Transfer_command(0x40); // Set display start line to 0 
   this->Transfer_command(0xA1); //Set SEG Direction
   this->Transfer_command(0xC0); //Set COM Direction
   this->Transfer_command(0xA2); //Set Bias = 1/9
   this->Transfer_command(0x2C);  //Boost ON
   this->Transfer_command(0x2E); //Voltage Regular On
   this->Transfer_command(0x2F); //Voltage Follower On
   this->Transfer_command(0xF8); //Set booster ratio to 
   this->Transfer_command(0x00); //4x
   this->Transfer_command(0x23); //Set Resistor Ratio = 3
   this->Transfer_command(0x81);
   this->Transfer_command(0x28); //Set Electronic Volume = 40
   this->Transfer_command(0xAC);//Set Static indicator off
   this->Transfer_command(0x00);
   this->Transfer_command(0XA6); // Disable inverse
   this->Transfer_command(0xAF); //Set Display Enable
   delay(100);
   this->Transfer_command(0xA5); //display all points
   delay(200);
   this->Transfer_command(0xA4); //normal display
   digitalWrite(this->pin_cs1, LOW);
   this->clear();
}

void UC1701::clear()
{
for  (unsigned short j = 0; j < 8; j++) 
{
    this->setCursor(0, j);
    for (unsigned short i = 0; i < 132 ; i++) {
        this->Transfer_data(0x00);
    }
}

    this->setCursor(0, 0);
}

void UC1701::setCursor(uint8_t column, uint8_t line) {
       int i, j;
       column = column+4;
       this->column = column;
       this->line = line;

       i = (column&0xF0)>>4;
       j = column&0x0F;
       digitalWrite(this->pin_cs1, LOW);
       this->Transfer_command(0xb0+line) ; 
       this->Transfer_command(0x10+i); 
       this->Transfer_command(j);
}

size_t UC1701::write(uint8_t chr)
{
    // ASCII 7-bit only...
    if (chr >= 0x80) {
        return 0;
    }

    if (chr == '\r') {
        this->setCursor(0, this->line);
        return 1;
    } else if (chr == '\n') {
        this->setCursor(this->column, this->line + 1);
        return 1;

    }

    const uint8_t *glyph;
    uint8_t pgm_buffer[5];

    if (chr >= ' ') {
        // Regular ASCII characters are kept in flash to save RAM...
        memcpy_P(pgm_buffer, &charset[chr - ' '], sizeof(pgm_buffer));
        glyph = pgm_buffer;
    } else {
        // Custom glyphs, on the other hand, are stored in RAM...
        if (this->custom[chr]) {
            glyph = this->custom[chr];
        } else {
            // Default to a space character if unset...
            memcpy_P(pgm_buffer, &charset[0], sizeof(pgm_buffer));
            glyph = pgm_buffer;
        }
    }

    // Output one column at a time...
    for (uint8_t i = 0; i < 5; i++) {
        this->Transfer_data(glyph[i]);
    }

    // One column between characters...
   this->Transfer_data( 0x00);

    // Update the cursor position...
    this->column = (this->column + 6) % this->width;

    if (this->column == 0) {
        this->line = (this->line + 1) % (this->height/9 + 1);
    }

    return 1;
}

void UC1701::createChar(uint8_t chr, const uint8_t *glyph)
{
    // ASCII 0-31 only...
    if (chr >= ' ') {
        return;
    }
    
    this->custom[chr] = glyph;
}

void UC1701::clearLine()
{
    this->setCursor(0, this->line);

    for (uint8_t i = 4; i < 132; i++) {
        this->Transfer_data( 0x00);
    }

    this->setCursor(0, this->line);
}

void UC1701::home()
{
    this->setCursor(0, this->line);
}


void UC1701::drawBitmap(const uint8_t *data, uint8_t columns, uint8_t lines)
{
    uint8_t scolumn = this->column;
    uint8_t sline = this->line;

    // The bitmap will be clipped at the right/bottom edge of the display...
    uint8_t mx = (scolumn + columns > this->width) ? (this->width - scolumn) : columns;
    uint8_t my = (sline + lines > this->height/8) ? (this->height/8 - sline) : lines;

    for (uint8_t y = 0; y < my; y++) {
        this->setCursor(scolumn, sline + y);

        for (uint8_t x = 0; x < mx; x++) {
            this->Transfer_data(data[y * columns + x]);
        }
    }

    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + columns, sline);
}

void UC1701::drawColumn(uint8_t lines, uint8_t value)
{
    uint8_t scolumn = this->column;
    uint8_t sline = this->line;

    // Keep "value" within range...
    if (value > lines*8) {
        value = lines*8;
    }

    // Find the line where "value" resides...
    uint8_t mark = (lines*8 - 1 - value)/8;
    
    // Clear the lines above the mark...
    for (uint8_t line = 0; line < mark; line++) {
        this->setCursor(scolumn, sline + line);
        this->Transfer_data( 0x00);
    }

    // Compute the byte to draw at the "mark" line...
    uint8_t b = 0xff;
    for (uint8_t i = 0; i < lines*8 - mark*8 - value; i++) {
        b <<= 1;
    }

    this->setCursor(scolumn, sline + mark);
    this->Transfer_data(b);

    // Fill the lines below the mark...
    for ( line = mark + 1; line < lines; line++) {
        this->setCursor(scolumn, sline + line);
        this->Transfer_data(0xff);
    }
  
    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + 1, sline); 
}

void UC1701::Transfer_command(int data1)
{
   digitalWrite(this->pin_cs1, LOW);
   digitalWrite(this->pin_a0, LOW);
#if USE_UC1701_SPIMODE == 0
   shiftOut(this->pin_sid, this->pin_sclk, MSBFIRST, data1);
#else
   SPI.transfer(data1); 
#endif
   digitalWrite(this->pin_cs1, HIGH);
}

void UC1701::Transfer_data(int data1)
{
   digitalWrite(this->pin_cs1, LOW);
   digitalWrite(this->pin_a0, HIGH);
#if USE_UC1701_SPIMODE == 0
   shiftOut(this->pin_sid, this->pin_sclk, MSBFIRST, data1);
#else
   SPI.transfer(data1); 
#endif
   digitalWrite(this->pin_cs1, HIGH);
}

