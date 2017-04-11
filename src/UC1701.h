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


#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif


class UC1701: public Print {
    public:
        // All the pins can be changed from the default values...
        UC1701(unsigned char sclk  = 21,   /* clock       (SCK) */
                unsigned char sid = 20,   /* data-in     (MOSI) */
                unsigned char cs1   = 19,   /* data select (CS) */
                unsigned char a0  = 22);  /* a0     (A0)*/
               

        // Display initialization (dimensions in pixels)...
        void begin();

        // Erase everything on the display...
        void clear();
        void clearLine();  // ...or just the current line
        
    
        // Place the cursor at the start of the current line...
        void home();

        // Place the cursor at position (column, line)...
        void setCursor(unsigned char column, unsigned char line);

        // Assign a user-defined glyph (5x8) to an ASCII character (0-31)...
        void createChar(unsigned char chr, const unsigned char *glyph);

        // Write an ASCII character at the current cursor position (7-bit)...
#if ARDUINO < 100
        virtual void write(uint8_t chr);
#else        
        virtual size_t write(uint8_t chr);
#endif

        // Draw a bitmap at the current cursor position...
        void drawBitmap(const unsigned char *data, unsigned char columns, unsigned char lines);

        // Draw a chart element at the current cursor position...
        void drawColumn(unsigned char lines, unsigned char value);

        // Send a command or data to the display...
        void Transfer_command(int data1);
        void Transfer_data(int data1);

    private:
        unsigned char pin_sclk;
        unsigned char pin_sid;
        unsigned char pin_cs1;
        unsigned char pin_a0;

        // The size of the display, in pixels...
        unsigned char width;
        unsigned char height;

        // Current cursor position...
        unsigned char column;
        unsigned char line;

        // User-defined glyphs (below the ASCII space character)...
        const unsigned char *custom[' '];


};


