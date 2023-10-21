/*
  Charliplexing.cpp - Using timer2 with 1ms resolution
  
  Alex Wenger <a.wenger@gmx.de> http://arduinobuch.wordpress.com/
  Matt Mets <mahto@cibomahto.com> http://cibomahto.com/
  
  Timer init code from MsTimer2 - Javier Valencia <javiervalencia80@gmail.com>
  Misc functions from Benjamin Sonnatg <benjamin@sonntag.fr>
  
  History:
    2009-12-30 - V0.0 wrote the first version at 26C3/Berlin
    2010-01-01 - V0.1 adding misc utility functions 
      (Clear, Vertical,  Horizontal) comment are Doxygen complaints now
    2010-05-27 - V0.2 add double-buffer mode
    2010-08-18 - V0.9 Merge brightness and grayscale

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <inttypes.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Yalinka.h"


/* -----------------------------------------------------------------  */
/// Determines whether the display is in single or double buffer mode
uint8_t displayMode = SINGLE_BUFFER;


/** Table for the LED multiplexing cycles
 * Each frame is made of 24 bytes (for the 12 display cycles)
 * There are SHADES-1 frames per buffer in grayscale mode (one for each
 * brightness) and twice that many to support double-buffered grayscale.
 */
struct videoPage {
    uint16_t pixels[12*(SHADES-1)];
}; 

/// Display buffers; only account two if DOUBLE_BUFFER is configured
#ifdef DOUBLE_BUFFER
volatile boolean videoFlipPage = false;
videoPage leds[2], *displayBuffer, *workBuffer;
#else
videoPage leds;
#define displayBuffer (&leds)
#define workBuffer (&leds)
#endif

/// Pointer inside the buffer that is currently being displayed
uint16_t* displayPointer;


// Timer counts to display each page for, plus off time
typedef struct timerInfo {
    uint8_t counts[SHADES];
    uint8_t prescaler[SHADES];
};

/// Timing buffers (see SetBrightness())
volatile boolean videoFlipTimer = false;
timerInfo timer[2], *frontTimer, *backTimer;


// Number of ticks of the prescaled timer per cycle per frame, based on the
// CPU clock speed and the desired frame rate.
#define	TICKS		(F_CPU + 6 * (FRAMERATE << SLOWSCALERSHIFT)) / (12 * (FRAMERATE << SLOWSCALERSHIFT))

// Cutoff below which we need to use a lower prescaler.  This is designed
// so that TICKS is always <128, to avoid arithmetic overflow calculating
// individual "page" times.
// TODO: Technically the 128 cutoff depends on SHADES, FASTSCALERSHIFT,
// and the gamma curve.
#define	CUTOFF(scaler)	((128 * 12 - 6) * FRAMERATE * scaler)

const uint8_t
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega8__)
#   if F_CPU < CUTOFF(8)
        fastPrescaler = _BV(CS20),				// 1
        slowPrescaler = _BV(CS21);				// 8
#       define SLOWSCALERSHIFT 3
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(32)
        fastPrescaler = _BV(CS21),				// 8
        slowPrescaler = _BV(CS21) | _BV(CS20);			// 32
#       define SLOWSCALERSHIFT 5
#       define FASTSCALERSHIFT 2
#   elif F_CPU < CUTOFF(64)
        fastPrescaler = _BV(CS21),				// 8
        slowPrescaler = _BV(CS22);				// 64
#       define SLOWSCALERSHIFT 6
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(128)
        fastPrescaler = _BV(CS21) | _BV(CS20),			// 32
        slowPrescaler = _BV(CS22) | _BV(CS20);			// 128
#       define SLOWSCALERSHIFT 7
#       define FASTSCALERSHIFT 2
#   elif F_CPU < CUTOFF(256)
        fastPrescaler = _BV(CS21) | _BV(CS20),			// 32
        slowPrescaler = _BV(CS22) | _BV(CS21);			// 256
#       define SLOWSCALERSHIFT 8
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(1024)
        fastPrescaler = _BV(CS22) | _BV(CS20),			// 128
        slowPrescaler = _BV(CS22) | _BV(CS21) | _BV(CS20);	// 1024
#       define SLOWSCALERSHIFT 10
#       define FASTSCALERSHIFT 3
#   else
#       error frame rate is too low
#   endif
#elif defined (__AVR_ATmega128__)
#   if F_CPU < CUTOFF(8)
        fastPrescaler = _BV(CS20),		// 1
        slowPrescaler = _BV(CS21);		// 8
#       define SLOWSCALERSHIFT 3
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(64)
        fastPrescaler = _BV(CS21),		// 8
        slowPrescaler = _BV(CS21) | _BV(CS20);	// 64
#       define SLOWSCALERSHIFT 6
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(256)
        fastPrescaler = _BV(CS21) | _BV(CS20),	// 64
        slowPrescaler = _BV(CS22);		// 256
#       define SLOWSCALERSHIFT 8
#       define FASTSCALERSHIFT 2
#   elif F_CPU < CUTOFF(1024)
        fastPrescaler = _BV(CS22),		// 256
        slowPrescaler = _BV(CS22) | _BV(CS20);	// 1024
#       define SLOWSCALERSHIFT 10
#       define FASTSCALERSHIFT 2
#   else
#       error frame rate is too low
#   endif
#elif defined (__AVR_ATmega32U4__)
#   if F_CPU < CUTOFF(8)
        fastPrescaler = _BV(WGM12) | _BV(CS10),			// 1
        slowPrescaler = _BV(WGM12) | _BV(CS11);			// 8
#       define SLOWSCALERSHIFT 3
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(64)
        fastPrescaler = _BV(WGM12) | _BV(CS11),			// 8
        slowPrescaler = _BV(WGM12) | _BV(CS11) | _BV(CS10);	// 64
#       define SLOWSCALERSHIFT 6
#       define FASTSCALERSHIFT 3
#   elif F_CPU < CUTOFF(256)
        fastPrescaler = _BV(WGM12) | _BV(CS11) | _BV(CS10),	// 64
        slowPrescaler = _BV(WGM12) | _BV(CS12);			// 256
#       define SLOWSCALERSHIFT 8
#       define FASTSCALERSHIFT 2
#   elif F_CPU < CUTOFF(1024)
        fastPrescaler = _BV(WGM12) | _BV(CS12),			// 256
        slowPrescaler = _BV(WGM12) | _BV(CS12) | _BV(CS10);	// 1024
#       define SLOWSCALERSHIFT 10
#       define FASTSCALERSHIFT 2
#   else
#       error frame rate is too low
#   endif
#else
#   error no support for this chip
#endif


static bool initialized = false;


/// Uncomment to set analog pin 5 high during interrupts, so that an
/// oscilloscope can be used to measure the processor time taken by it
#undef MEASURE_ISR_TIME
#ifdef MEASURE_ISR_TIME
const uint8_t statusPIN = 19;
#endif


/* -----------------------------------------------------------------  */
/** Table for LED Position in leds[] ram table
 */
typedef struct LEDPosition {
    uint8_t high;
    uint8_t cycle;
};

#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
#define	P(pin)  ((pin < 5) ? (pin + 1) : (pin == 5) ? (2) : (pin))
#elif defined (__AVR_ATmega32U4__)
#define	P(pin)	((pin == 2) ? (1) : (pin == 3) ? (0) : (pin == 5) ? (2) : (pin == 6) ? (7) : (pin == 7) ? (5) : (pin == 12) ? (6) : (pin == 13) ? (3) : (pin))
#else
#define	P(pin)	(pin)
#endif
#if !defined (__AVR_ATmega32U4__)
#define L(high, low)	{ P(high), (P(low) - 2) }
#else
// Since the offset of 2 doesn't have to do anything with the ports anymore and just adds complexity we omit it.
#define L(high, low)	{ P(high), P(low) }
#endif
const LEDPosition PROGMEM ledMap[42] = {

	// L(2, 12),    L(2, 4),   L(2, 5),   L(2, 6),   L(2, 7),   L(2, 8),
	// L(12, 2),   L(12, 4),  L(12, 5),  L(12, 6),  L(12, 7),   L(12, 8),
	// L(4, 2),   L(4, 12),   L(4, 5),   L(4, 6),   L(4, 7),   L(4, 8),
	// L(5, 12),    L(5, 2),   L(5, 4),   L(5, 6),   L(5, 7),   L(5, 8),
	// L(6, 2),   L(6, 12),   L(6, 4),   L(6, 5),   L(6, 7),   L(6, 8),
	// L(7, 2),   L(7, 12),   L(7, 4),   L(7, 5),   L(7, 6),   L(7, 8),
	// L(8, 2),    L(8, 4),   L(8, 6),   L(8, 7),   L(8, 5),   L(8, 12),
	
L(5,12), //19
L(5,2), //20
L(5,4), //21
L(5,6), //22
L(5,7), //23
L(5,8), //24
L(6,2), //25
L(6,12), //26
L(6,4), //27
L(6,5), //28
L(6,7), //29
L(6,8), //30
L(7,2), //31
L(7,12), //32
L(7,4), //33
L(7,5), //34
L(7,6), //35
L(7,8), //36
L(8,2), //37
L(8,4), //38
L(8,6), //39
L(8,7), //40
L(8,5), //41
L(8,12), //42
L(2,12), //1
L(2,4), //2
L(2,5), //3
L(2,6), //4
L(2,7), //5
L(2,8), //6
L(12,2), //7
L(12,4), //8
L(12,5), //9
L(4,8), //18
L(4,7), //17
L(4,6), //16
L(4,5), //15
L(4,12), //14
L(4,2), //13
L(12,8), //12
L(12,7), //11
L(12,6), //10
};
#undef P(pin)
#undef L(high, low)

/*
Converting the pin numbers to indices usable with Leonardo.

pin number -> Leonardo port number -> logical index
---------------------------------------------------
02         -> D1                   -> 01
03         -> D0                   -> 00
04         -> D4                   -> 04
05         -> C6                   -> 02
06         -> D7                   -> 07
07         -> E6                   -> 05
08         -> B4                   -> 08
09         -> B5                   -> 09
10         -> B6                   -> 10
11         -> B7                   -> 11
12         -> D6                   -> 06
13         -> C7                   -> 03

This yields the horrible macro
#define P(pin)  ((pin == 2) ? (1) : (pin == 3) ? (0) : (pin == 5) ? (2) : (pin == 6) ? (7) : (pin == 7) ? (5) : (pin == 12) ? (6) : (pin == 13) ? (3) : (pin))
TODO If anyone has a better idea how to handle this, feel free to change it.
TODO Another possibility would be to just add another ledMap without the remap macros.

The order in which the LEDs light up is now pretty random, but shouldn't be visible with a high update rate.

The used ports are
B7, B6, B5, B4, --, --, --, --
C7, C6, --, --, --, --, --, --
D7, D6, --, D4, --, --, D1, D0
--, E6, --, --, --, --, --, --

Luckily, these can merge together into the following contiguous order, stored in the 16bit pixels variable.
--, --, --, --, B7, B6, B5, B4
D7, D6, E6, D4, C7, C6, D1, D0

Now the ISR can work in the same way, it just has to extract the ports like this
PORTB = (pixels >> 4) & 0xF0);
PORTC = (pixels << 4) & 0xC0);
PORTD = (pixels << 0) & 0xD3);
PORTE = (pixels << 1) & 0x40);
*/

/* -----------------------------------------------------------------  */
/** Constructor : Initialize the interrupt code. 
 * should be called in setup();
 */
void Yalinka::Init(uint8_t mode)
{
#ifdef MEASURE_ISR_TIME
    pinMode(statusPIN, OUTPUT);
    digitalWrite(statusPIN, LOW);
#endif

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
    TIMSK2 &= ~(_BV(TOIE2) | _BV(OCIE2A));
    TCCR2A &= ~(_BV(WGM21) | _BV(WGM20));
    TCCR2B &= ~_BV(WGM22);
    ASSR &= ~_BV(AS2);
#elif defined (__AVR_ATmega8__)
    TIMSK &= ~(_BV(TOIE2) | _BV(OCIE2));
    TCCR2 &= ~(_BV(WGM21) | _BV(WGM20));
    ASSR &= ~_BV(AS2);
#elif defined (__AVR_ATmega128__)
    TIMSK &= ~(_BV(TOIE2) | _BV(OCIE2));
    TCCR2 &= ~(_BV(WGM21) | _BV(WGM20));
#elif defined (__AVR_ATmega32U4__)
    // The only 8bit timer on the Leonardo is used by default, so we use the 16bit Timer1
    // in CTC mode with a compare value of 256 to achieve the same behaviour.
    TIMSK1 &= ~(_BV(TOIE1) | _BV(OCIE1A));
    TCCR1A &= ~(_BV(WGM10) | _BV(WGM11));
    OCR1A = 256;
#endif
	
    // Record whether we are in single or double buffer mode
    displayMode = mode;

#ifdef DOUBLE_BUFFER
    videoFlipPage = false;
    // If we are in single buffered mode, point the work buffer
    // at the same physical buffer as the display buffer.  Otherwise,
    // point it at the second physical buffer.
    if (displayMode & DOUBLE_BUFFER)
        workBuffer = &leds[1];
    else
        workBuffer = &leds[0];
    displayBuffer = &leds[0];
#endif

    // Point the display buffer to the first physical buffer
    displayPointer = displayBuffer->pixels;

    // Set up the timer buffering
    videoFlipTimer = false;
    backTimer = &timer[1];
    frontTimer = &timer[0];

    Yalinka::SetBrightness(127);

    // Then start the display
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
    TIMSK2 |= _BV(TOIE2);
    TCCR2B = fastPrescaler;
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega128__)
    TIMSK |= _BV(TOIE2);
    TCCR2 = fastPrescaler;
#elif defined (__AVR_ATmega32U4__)
    // Enable output compare match interrupt
    TIMSK1 |= _BV(OCIE1A);
    TCCR1B = fastPrescaler;
#endif
    // interrupt ASAP
#if !defined (__AVR_ATmega32U4__)
    TCNT2 = 255;
#else
    TCNT1 = 255;
#endif

    initialized = true;
}


#ifdef DOUBLE_BUFFER
/* -----------------------------------------------------------------  */
/** Signal that the front and back buffers should be flipped
 * @param blocking if true : wait for flip before returning, if false :
 *                 return immediately.
 */
void Yalinka::Flip(bool blocking)
{
    // Just set the flip flag, the buffer will flip between redraws
    videoFlipPage = true;

    // If we are blocking, sit here until the page flips.
    if (blocking)
        while (videoFlipPage)
            ;
}
#endif


/* -----------------------------------------------------------------  */
/** Clear the screen completely
 * @param set if 1 : make all led ON, if not set or 0 : make all led OFF
 */
void Yalinka::Clear(uint8_t c) {
    for (uint8_t z=0; z<42; z++) 
            Set(z, c);
}

/* -----------------------------------------------------------------  */
/** Set : switch on and off the leds. All the position #for char in frameString:
 * calculations are done here, so we don't need to do in the
 * interrupt code
 */
void Yalinka::Set(uint8_t z, uint8_t c)
{
  if (z < 0) z = 0;
  if (z > 41) z = 41;
#ifdef GRAYSCALE
    // If we aren't in grayscale mode, just map any pin brightness to max
    if (c > 0 && !(displayMode & GRAYSCALE))
        c = SHADES-1;
#else
    if (c)
        c = SHADES-1;
#endif

    const LEDPosition *map = &ledMap[z];
    uint16_t mask = 1 << pgm_read_byte_near(&map->high);
    uint8_t cycle = pgm_read_byte_near(&map->cycle);

    uint16_t *p = &workBuffer->pixels[cycle*(SHADES-1)];
    uint8_t i;
    for (i = 0; i < c; i++)
	*p++ |= mask;   // ON;
    for (; i < SHADES-1; i++)
	*p++ &= ~mask;   // OFF;
}

uint8_t Yalinka::DisplayMode(){
	return displayMode;
}

/* Set the overall brightness of the screen
 * @param brightness LED brightness, from 0 (off) to 127 (full on)
 */

void Yalinka::SetBrightness(uint8_t brightness)
{
    // An exponential fit seems to approximate a (perceived) linear scale
    const unsigned long brightnessPercent = ((unsigned int)brightness * (unsigned int)brightness + 8) >> 4; /*7b*2-4b = 10b*/

    /*   ---- This needs review! Please review. -- thilo  */
    // set up page counts
    // TODO: make SHADES a function parameter. This would require some refactoring.
    uint8_t i;
    const int ticks = TICKS;
    const unsigned long m = (ticks << FASTSCALERSHIFT) * brightnessPercent; /*10b*/
#define	C(x)	((m * (unsigned long)(x * 1024) + (1<<19)) >> 20) /*10b+10b-20b=0b*/
#if SHADES == 2
    const int counts[SHADES] = {
	0.0f,
	C(1.0f),
    };
#elif SHADES == 8
    const int counts[SHADES] = {
	0.0f,
	C(0.030117819624378613658712f),
	C(0.104876339357015456218728f),
	C(0.217591430058779512857041f),
	C(0.365200625214741116475101f),
	C(0.545719579451565749226202f),
	C(0.757697368024318811680598f),
	C(1.0f),
    };
#else
    // NOTE: Changing "scale" invalidates any tables above!
    const float scale = 1.8f;
    int counts[SHADES]; 

    counts[0] = 0.0f;
    for (i=1; i<SHADES; i++)
        counts[i] = C(pow(i / (float)(SHADES - 1), scale));
#endif

    // Wait until the previous brightness request goes through
    while (videoFlipTimer)
        ;

    // Compute on time for each of the pages
    // Use the fast timer; slow timer is only useful for < 3 shades.
    for (i = 0; i < SHADES - 1; i++) {
        int interval = counts[i + 1] - counts[i];
        backTimer->counts[i] = 256 - (interval ? interval : 1);
        backTimer->prescaler[i] = fastPrescaler;
    }

    // Compute off time
    int interval = ticks - (counts[i] >> FASTSCALERSHIFT);
    backTimer->counts[i] = 256 - (interval ? interval : 1);
    backTimer->prescaler[i] = slowPrescaler;

    if (!initialized)
        *frontTimer = *backTimer;

    /*   ---- End of "This needs review! Please review." -- thilo  */

    // Have the ISR update the timer registers next run
    videoFlipTimer = true;
}


/* -----------------------------------------------------------------  */
/** The Interrupt code goes here !  
 */
#if !defined (__AVR_ATmega32U4__)
ISR(TIMER2_OVF_vect) {
#else
ISR(TIMER1_COMPA_vect) {
#endif
#ifdef MEASURE_ISR_TIME
    digitalWrite(statusPIN, HIGH);
#endif

    // For each cycle, we have potential SHADES pages to display.
    // Once every page has been displayed, then we move on to the next
    // cycle.

    // 24 Cycles of Matrix
    static uint8_t cycle = 0;

    // SHADES pages to display
    static uint8_t page = 0;

#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
    TCCR2B = frontTimer->prescaler[page];
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega128__)
    TCCR2 = frontTimer->prescaler[page];
#elif defined (__AVR_ATmega32U4__)
    TCCR1B = frontTimer->prescaler[page];
#endif
#if !defined (__AVR_ATmega32U4__)
    TCNT2 = frontTimer->counts[page];
#else
    TCNT1 = frontTimer->counts[page];
#endif

#if defined (__AVR_ATmega1280__) || defined (__AVR_ATmega2560__)
    static uint16_t sink = 0;

    PINE = (sink << 1) & 0x38;
    PING = (sink << 0) & 0x20;
    PINH = (sink >> 3) & 0x78;
    PINB = (sink >> 6) & 0xf0;
    //delayMicroseconds(1);
    DDRE &= ~0x38;
    DDRG &= ~0x20;
    DDRH &= ~0x78;
    DDRB &= ~0xf0;

    sink = 1 << (cycle+2);
    uint16_t pins = sink;
    if (page < SHADES - 1)
        pins |= *displayPointer++;

    PINE = (PORTE ^ (pins << 1)) & 0x38;
    PING = (PORTG ^ (pins << 0)) & 0x20;
    PINH = (PORTH ^ (pins >> 3)) & 0x78;
    PINB = (PORTB ^ (pins >> 6)) & 0xf0;
    //delayMicroseconds(1);
    DDRE |= (pins << 1) & 0x38;
    DDRG |= (pins << 0) & 0x20;
    DDRH |= (pins >> 3) & 0x78;
    DDRB |= (pins >> 6) & 0xf0;
    PINE = (sink << 1) & 0x38;
    PING = (sink << 0) & 0x20;
    PINH = (sink >> 3) & 0x78;
    PINB = (sink >> 6) & 0xf0;
#elif defined (__AVR_ATmega32U4__)
    static uint16_t sink = 0;

    PINB = (sink >> 4) & 0xF0;
    PINC = (sink << 4) & 0xC0;
    PIND = (sink << 0) & 0xD3;
    PINE = (sink << 1) & 0x40;
    //delayMicroseconds(1);
    DDRB &= ~0xF0;
    DDRC &= ~0xC0;
    DDRD &= ~0xD3;
    DDRE &= ~0x40;

    sink = 1 << (cycle);
    uint16_t pins = sink;
    if (page < SHADES - 1)
        pins |= *displayPointer++;

    PINB = (PORTB ^ (pins >> 4)) & 0xF0;
    PINC = (PORTC ^ (pins << 4)) & 0xC0;
    PIND = (PORTD ^ (pins << 0)) & 0xD3;
    PINE = (PORTE ^ (pins << 1)) & 0x40;
    //delayMicroseconds(1);
    DDRB |= (pins >> 4) & 0xF0;
    DDRC |= (pins << 4) & 0xC0;
    DDRD |= (pins << 0) & 0xD3;
    DDRE |= (pins << 1) & 0x40;
    PINB = (sink >> 4) & 0xF0;
    PINC = (sink << 4) & 0xC0;
    PIND = (sink << 0) & 0xD3;
    PINE = (sink << 1) & 0x40;
#else
    static uint16_t sink = 0;

    // Set sink pin to Vcc/source, turning off current.
    PIND = sink;
    PINB = (sink >> 8);
    //delayMicroseconds(1);
    // Set pins to input mode; Vcc/source become pullups.
    DDRD = 0;
    DDRB = 0;

    sink = 1 << (cycle+2);
    uint16_t pins = sink;
    if (page < SHADES - 1)
        pins |= *displayPointer++;

    // Enable pullups on new output pins.
    PORTD = pins;
    PORTB = (pins >> 8);
    //delayMicroseconds(1);
    // Set pins to output mode; pullups become Vcc/source.
    DDRD = pins;
    DDRB = (pins >> 8);
    // Set sink pin to GND/sink, turning on current.
    PIND = sink;
    PINB = (sink >> 8);
#endif

    page++;

    if (page >= SHADES) {
        page = 0;
        cycle++;

        if (cycle >= 12) {
            cycle = 0;

#ifdef DOUBLE_BUFFER
            // If the page should be flipped, do it here.
            if (videoFlipPage)
            {
                videoFlipPage = false;
    
                videoPage* temp = displayBuffer;
                displayBuffer = workBuffer;
                workBuffer = temp;
            }
#endif

            if (videoFlipTimer) {
                videoFlipTimer = false;

		timerInfo* temp = frontTimer;
                frontTimer = backTimer;
                backTimer = temp;
            }

	    displayPointer = displayBuffer->pixels;
        }
    }

#ifdef MEASURE_ISR_TIME
    digitalWrite(statusPIN, LOW);
#endif
}
