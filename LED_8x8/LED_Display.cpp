/*/
 * Library for Displaying Patterns on a 1588AS 8x8 LED Display
 * with a 74HC595 shift regesiter.
 * Andrew Player
 * 2022
/*/


#include "LED_Display.hpp"


/*----------------------------------------------------------------------------------------------------------------*/
/* Macros for speed and cleanliness, but feels illegal to me for some reason.                                     */
/*----------------------------------------------------------------------------------------------------------------*/
#define PORTDWRITE1(PIN) PORTD |=  (0b1 <<  PIN)
#define PORTDWRITE0(PIN) PORTD &= ~(0b1 <<  PIN)

#define PORTBWRITE1(PIN) PORTB |=  (0b1 << (PIN % 8))
#define PORTBWRITE0(PIN) PORTB &= ~(0b1 << (PIN % 8))

#define PICKPORTANDWRITE1(PIN) if (PIN < 8){PORTDWRITE1(PIN);} else {PORTBWRITE1(PIN);}
#define PICKPORTANDWRITE0(PIN) if (PIN < 8){PORTDWRITE0(PIN);} else {PORTBWRITE0(PIN);}

#define CLOCKPULSE(PIN) if (PIN < 8) {PORTDWRITE1(PIN); PORTDWRITE0(PIN);} else {PORTBWRITE1(PIN); PORTBWRITE0(PIN);}
/*----------------------------------------------------------------------------------------------------------------*/


LED_Display::LED_Display()
{

}


LED_Display::LED_Display(const uint8_t shift_pin_array    [N_SHIFT_PINS    ],
                         const uint8_t hard_wire_pin_array[N_HARD_WIRE_PINS],
                         const uint8_t pixel_drawing      [N_PIXELS        ])
{
    init(shift_pin_array, hard_wire_pin_array, pixel_drawing);
}


void LED_Display::init(const uint8_t shift_pin_array    [N_SHIFT_PINS    ],
                       const uint8_t hard_wire_pin_array[N_HARD_WIRE_PINS],
                       const uint8_t pixel_drawing      [N_PIXELS        ])
{
    _setPins(shift_pin_array, hard_wire_pin_array);
    _setPinModes();

    setPixels(pixel_drawing);
}


LED_Display::~LED_Display()
{
    const uint8_t zeros[N_PIXELS] = {0, 0, 0, 0, 0, 0, 0, 0};
  
    setPixels(zeros);

    drawDisplay();
}


void LED_Display::_setPins(const uint8_t shift_pin_array    [N_SHIFT_PINS    ],
                           const uint8_t hard_wire_pin_array[N_HARD_WIRE_PINS])
{
    _DATA_pin  = shift_pin_array[0];
    _SRCLK_pin = shift_pin_array[1];
    _RCLK_pin  = shift_pin_array[2];

    for (uint8_t pin = 0; pin < N_HARD_WIRE_PINS; pin++) 
    {
        _hard_wire_pins[pin] = hard_wire_pin_array[pin];
    }
}


void LED_Display::setPixels(const uint8_t pixel_drawing[N_PIXELS])
{
    for (uint8_t pixel = 0; pixel < N_PIXELS; pixel++)
    {
        _pixels[pixel] = pixel_drawing[pixel];
    }

    _setOutputArray();
}


void LED_Display::_createPatternArray(LED_rc_bits_t patterns[N_PIXELS * 2]) const 
{
    uint8_t placement = 0;

    // Go through each uint8_t pixel row and generate a LED_rc_bits_t pattern row
    for (int8_t row = N_PIXELS - 1; row >= 0; row--) // Going backwards makes sure the image is oriented correctly.
    { 
        LED_rc_bits_t pattern_row = 0b1111111100000000;
 
        const uint8_t drawing_row = _pixels[placement];
        const uint8_t row_shift   = row + 8;

        for (int8_t col = 0; col < N_PIXELS; col++)
        {
            const uint8_t pixel = (drawing_row & (1 << col));

            if (pixel)
            {
                pattern_row &= ~(0b1 << row_shift);  // Row Bit
                pattern_row |=   0b1 << col;         // Col Bit
            }
        }

        patterns[placement] = pattern_row;

        placement++;
    }
}


void LED_Display::_setOutputArray() 
{

    uint8_t is_hardwired_pin_and_rc_at_bit[N_BITS][3];

    _createPinRCTable(is_hardwired_pin_and_rc_at_bit);
  
    LED_rc_bits_t patterns[N_PIXELS * 2] = { 0 };

    _createPatternArray(patterns);

    for (uint8_t i = 0; i < N_PIXELS * 2; i++)
    {
        uint8_t shift  = 0b00000000; // This value gets sent to _sendByte for the shift register
        uint8_t port_b = 0b00000000; // This value gets sent to PORTB
        uint8_t port_d = 0b00000000; // This value gets sent to PORTD

        uint16_t  pattern;

        if (i % 2 == 0) pattern = patterns[(uint8_t)(i / 2)];
        else            pattern = 0b1111111100000000;  // Blank out every other row.
    
        for (uint8_t rc = 0; rc < 16; rc++)
        {
            const bool&    is_shift = !is_hardwired_pin_and_rc_at_bit[rc][0];
            const uint8_t& pin      =  is_hardwired_pin_and_rc_at_bit[rc][1];  
            const uint8_t& rc_val   =  is_hardwired_pin_and_rc_at_bit[rc][2];

            const bool bit  = 1 & (pattern >> rc_val);  // Get the bit at the row/column offset in the pattern.

            /* Place the bit in the correct position */
            if (is_shift)      shift  ^= (bit << (pin    ));
            else if (pin < 8)  port_d ^= (bit << (pin    ));
            else               port_b ^= (bit << (pin % 8));
        }

        _outputs[i][0] = port_b;
        _outputs[i][1] = port_d;
        _outputs[i][2] = shift ;
    }
}


void LED_Display::_setPinModes() const
{
    for (const uint8_t hard_wired_pin : _hard_wire_pins) 
    {
        pinMode(hard_wired_pin, OUTPUT);
    }

    pinMode(_RCLK_pin, OUTPUT);
    PICKPORTANDWRITE0(_RCLK_pin);

    pinMode(_SRCLK_pin, OUTPUT);
    PICKPORTANDWRITE0(_SRCLK_pin);

    pinMode(_DATA_pin, OUTPUT);
    PICKPORTANDWRITE0(_DATA_pin);
}


void LED_Display::_createPinRCTable(uint8_t table[N_BITS][3]) const
{
    for (uint8_t row = 0; row < N_BITS; row++)
    {
        for (uint8_t col = 0; col < N_HARD_WIRE_BITS; col++)
        {
            if (row == _hard_wire_rc[col])
            { 
                table[row][0] = true;
                table[row][1] = _hard_wire_pins[col];
                table[row][2] = row;
                break;
            }
            if (row == _shift_rc[col])
            {
                table[row][0] = false;
                table[row][1] = _shift_offsets[col];
                table[row][2] = row;
                break;
            }
        }
    }
}


void LED_Display::flipSetup()
{
    if (N_HARD_WIRE_BITS != N_SHIFT_BITS)
    {
        Serial.println("ARRAY SIZES DONT MATCH FOR FLIPSETUP!");
        return;
    }

    uint8_t back = N_HARD_WIRE_BITS - 1;

    for (uint8_t index = 0; index < N_HARD_WIRE_BITS; index++)
    {
        const uint8_t h_temp = _hard_wire_rc[index];
        const uint8_t s_temp = _shift_rc    [index];

        _hard_wire_rc[index]  = s_temp;
        _shift_rc    [index]  = h_temp;

        if(index < back)
        {
            const uint8_t o_temp = _shift_offsets[index];

            _shift_offsets[index] = _shift_offsets[back ];
            _shift_offsets[back ] = o_temp;
        }
 
        back--;
    }

    _setOutputArray();
}


void LED_Display::_sendByte(const uint8_t& data) const 
{
    for (uint8_t bit = 0; bit < 8; bit++) 
    {
        if (data & (1 << bit)) {PICKPORTANDWRITE1(_DATA_pin);}
        else                   {PICKPORTANDWRITE0(_DATA_pin);}

        CLOCKPULSE(_SRCLK_pin);
    }

    CLOCKPULSE(_RCLK_pin);
}


void LED_Display::drawDisplay() const
{
    for (uint8_t i = 0; i < N_PIXELS * 2; i++)
    {
        _sendByte(_outputs[i][2]);
        PORTD =   _outputs[i][1] ;
        PORTB =   _outputs[i][0] ;
    }
}


void LED_Display::drawDisplay(const uint32_t& microseconds) const
{
    drawDisplay();
    delayMicroseconds(microseconds);
}
