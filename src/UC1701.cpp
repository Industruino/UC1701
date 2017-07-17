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


#include "UC1701.h"

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <avr/pgmspace.h>


/*
 * If this was a ".h", it would get added to sketches when using
 * the "Sketch -> Import Library..." menu on the Arduino IDE...
 */
#include "charset.cpp"


UC1701::UC1701(unsigned char sclk, unsigned char sid,
                 unsigned char cs1,
                 unsigned char a0):
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
    pinMode(this->pin_sclk, OUTPUT);
    pinMode(this->pin_sid, OUTPUT);

    // Reset the controller state...
    digitalWrite(this->pin_cs1, LOW);
    digitalWrite(this->pin_a0, LOW);
    digitalWrite(this->pin_cs1, HIGH);

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

void UC1701::setCursor(unsigned char column, unsigned char line)
{
       int i, j;
       column = column+4;
       this->column = column;
       this->line = line;

       i=(column&0xF0)>>4;
       j=column&0x0F;
       digitalWrite(this->pin_cs1, LOW);
       this->Transfer_command(0xb0+line); 
       this->Transfer_command(0x10+i); 
       this->Transfer_command(j);
}

#if ARDUINO < 100
void UC1701::write(uint8_t chr)
#else
size_t UC1701::write(uint8_t chr)
#endif
{
    // ASCII 7-bit only...
    if (chr >= 0x80) {
#if ARDUINO < 100
        return;
#else
        return 0;
#endif
    }

    if (chr == '\r') {
        this->setCursor(0, this->line);
#if ARDUINO < 100
        return;
#else
        return 1;
#endif
    } else if (chr == '\n') {
        this->setCursor(this->column, this->line + 1);
#if ARDUINO < 100
        return;
#else
        return 1;
#endif
    }

    const unsigned char *glyph;
    unsigned char pgm_buffer[5];

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
    for (unsigned char i = 0; i < 5; i++) {
        this->Transfer_data(glyph[i]);
    }

    // One column between characters...
   this->Transfer_data( 0x00);

    // Update the cursor position...
    this->column = (this->column + 6) % this->width;

    if (this->column == 0) {
        this->line = (this->line + 1) % (this->height/9 + 1);
    }

#if ARDUINO >= 100
    return 1;
#endif
}

void UC1701::createChar(unsigned char chr, const unsigned char *glyph)
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

    for (unsigned char i = 4; i < 132; i++) {
        this->Transfer_data( 0x00);
    }

    this->setCursor(0, this->line);
}

void UC1701::home()
{
    this->setCursor(0, this->line);
}

void UC1701::drawBitmap(const unsigned char *data, unsigned char columns, unsigned char lines)
{
    unsigned char scolumn = this->column;
    unsigned char sline = this->line;

    // The bitmap will be clipped at the right/bottom edge of the display...
    unsigned char mx = (scolumn + columns > this->width) ? (this->width - scolumn) : columns;
    unsigned char my = (sline + lines > this->height/8) ? (this->height/8 - sline) : lines;

    for (unsigned char y = 0; y < my; y++) {
        this->setCursor(scolumn, sline + y);

        for (unsigned char x = 0; x < mx; x++) {
            this->Transfer_data(data[y * columns + x]);
        }
    }

    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + columns, sline);
}

void UC1701::drawColumn(unsigned char lines, unsigned char value)
{
    unsigned char scolumn = this->column;
    unsigned char sline = this->line;

    // Keep "value" within range...
    if (value > lines*8) {
        value = lines*8;
    }

    // Find the line where "value" resides...
    unsigned char mark = (lines*8 - 1 - value)/8;
    
    // Clear the lines above the mark...
    for (unsigned char line = 0; line < mark; line++) {
        this->setCursor(scolumn, sline + line);
        this->Transfer_data( 0x00);
    }

    // Compute the byte to draw at the "mark" line...
    unsigned char b = 0xff;
    for (unsigned char i = 0; i < lines*8 - mark*8 - value; i++) {
        b <<= 1;
    }

    this->setCursor(scolumn, sline + mark);
    this->Transfer_data(b);

    // Fill the lines below the mark...
    for (unsigned char line = mark + 1; line < lines; line++) {
        this->setCursor(scolumn, sline + line);
        this->Transfer_data(0xff);
    }
  
    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + 1, sline); 
}

void UC1701::Transfer_command(int data1)
{
   char i;
   digitalWrite(this->pin_cs1, LOW);
   digitalWrite(this->pin_a0, LOW);
   for (i=0; i<8; i++)
               {
                 digitalWrite(this->pin_sclk, LOW);
                 if(data1&0x80) digitalWrite(this->pin_sid, HIGH);
                 else digitalWrite(this->pin_sid, LOW);
                 delayMicroseconds(2);
                 digitalWrite(this->pin_sclk, HIGH);
                 delayMicroseconds(2);
                 data1=data1<<1;
               }
}

void UC1701::Transfer_data(int data1)
{
   char i;
   digitalWrite(this->pin_cs1, LOW);
   digitalWrite(this->pin_a0, HIGH);
   for (i=0; i<8; i++)
               {
                 digitalWrite(this->pin_sclk, LOW);
                 if(data1&0x80) digitalWrite(this->pin_sid, HIGH);
                 else digitalWrite(this->pin_sid, LOW);
                 delayMicroseconds(2);
                 digitalWrite(this->pin_sclk, HIGH);
                 delayMicroseconds(2);
                 data1=data1<<1;
               }
}

