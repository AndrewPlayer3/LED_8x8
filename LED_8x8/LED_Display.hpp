/*/
 * Library for Displaying Patterns on a 1588AS 8x8 LED Display
 * with a 74HC595 shift regesiter.
 * Andrew Player
 * 2022
/*/


#ifndef _LED_DISPLAY_HPP_
#define _LED_DISPLAY_HPP_

#ifndef Arduino_h 
#include <Arduino.h>
#endif 

#include <stdint-gcc.h>


#define N_SHIFT_PINS      3
#define N_SHIFT_BITS      8
#define N_HARD_WIRE_PINS  8
#define N_HARD_WIRE_BITS  8
#define N_PIXELS          8
#define N_BITS           16


/* Patterns are just 16-Bit Unsigned  */
typedef uint16_t LED_rc_bits_t;


enum : uint8_t
{ 
    // Cols: 0 ->  7
    C0 = 0, C1, C2, C3, C4, C5, C6, C7,

    // Rows: 8 -> 15
    R0 = 8, R1, R2, R3, R4, R5, R6, R7,
};


/*
 * This struct contains everything for drawing on a 1588AS 8x8 LED Dislpay with a 74HC595 shift register.
 * 
 * Notes:
 * ------
 *  By default it is assumed that the hard-wired side is the side with Row 4,
 *  if not, you need to call the flipSetup() method after initiallizing the object.
 *  You do not need to call it more than once.
 * 
 *  The top of the display (the zeroth row of _pixels) is the side of the display
 *  with the model numbers printed on.
 * 
 * Construction Parameters:
 * ------------------------
 *  shift_pins: uint8_t[#shift_register_pins (3)]
 *    - This array should contain the pin numbers in this order: DATA, SRCLK, RCHK.
 * 
 *  _hard_wire_pins: uint8_t[#hard_wired_pins (8)]
 *    - This array should contain the pin numbers as they correspond to this order: {R4, R6, C1, C2, R7, C4, R5, R2} if default.
 *                                                                                  {R0, C3, C5, R3, C0, R1, C6, C7} if using flipSetup().
 * 
 *  pixel_drawing: uint8_t[#pixels (8)]
 *    - This array contains an array of binary numbers, 1 8-bit number for each row.
 *      Each digit in the binary number represents the respective pixel. 0's are off, 1's are on.
 * 
 *      Diagonal Line Example: {
 *                              0b10000000,  ON OFF OFF OFF OFF OFF OFF OFF
 *                              0b01000000,  OFF ON ... ... ... ... ... OFF
 *                              0b00100000,  ...
 *                              0b00010000,  ...
 *                              0b00001000,  ...
 *                              0b00000100,  ...
 *                              0b00000010,  ...
 *                              0b00000001   OFF OFF OFF OFF OFF OFF OFF ON
 *                              }
 */
struct LED_Display 
{
    private:
        /* ---------------------------- */
        /* --- Private Data Members --- */
        /* ---------------------------- */

        uint8_t _DATA_pin ;  // Data Pin
        uint8_t _SRCLK_pin;  // Shift Pin 
        uint8_t _RCLK_pin ;  // Output Pin

        uint8_t _hard_wire_pins[N_HARD_WIRE_PINS];

        uint8_t _hard_wire_rc  [N_HARD_WIRE_BITS] = {R4, R6, C1, C2, R7, C4, R5, R2};
        uint8_t _shift_rc      [N_SHIFT_BITS    ] = {C7, C6, R1, C0, R3, C5, C3, R0};
        uint8_t _shift_offsets [N_SHIFT_BITS    ] = { 7,  6,  5,  4,  3,  2,  1,  0};

        uint8_t _pixels [N_PIXELS    ]   ;
        uint8_t _outputs[N_PIXELS * 2][3];

        /* --------------- */
        /* --- Setters --- */
        /* --------------- */

        // Set the values of the pins
        void _setPins(const uint8_t shift_pins[N_SHIFT_PINS], const uint8_t hard_wire_pins[N_HARD_WIRE_PINS]);

        // Constructs the output pieces from the pattern array.
        void _setOutputArray();

        // Sets the pin mode for all pins to OUTPUT
        void _setPinModes() const;

        /* ------------- */
        /* --- Misc. --- */
        /* ------------- */

        /*/
         * Creates table by reference for getting three things:
         * 1. If a bit position value goes to a hardware pin, or not.
         * 2. If hardware, the pin is given; else the left shift amount is given (position from the right in rc_s).
         * 3. The rc value for that bit position.
        /*/
        void _createPinRCTable(uint8_t table[N_BITS][3]) const;

        // Constructs the pattern array by reference from the pixel array.
        void _createPatternArray(LED_rc_bits_t patterns[N_PIXELS * 2]) const;

        // Send one byte of data to the shift register and output it.
        void _sendByte(const uint8_t& data) const;

    public:
        /* -------------------- */
        /* --- Constructors --- */
        /* -------------------- */

        // Default Constructor
        LED_Display();

        // Value Constructor 
        LED_Display(const uint8_t shift_pins   [N_SHIFT_PINS    ],
                    const uint8_t hardware_pins[N_HARD_WIRE_PINS],
                    const uint8_t pixel_drawing[N_PIXELS        ]);

        // Init method for use with default constructor
        void init(const uint8_t shift_pins   [N_SHIFT_PINS    ],
                  const uint8_t hardware_pins[N_HARD_WIRE_PINS],
                  const uint8_t pixel_drawing[N_PIXELS        ]);

        // Destructor just clears the display.
        ~LED_Display();

        /* ------------------- */
        /* --- Main Setter --- */
        /* ------------------- */

        // Set the values in the pixel array
        void setPixels(const uint8_t pixel_drawing[N_PIXELS]);

        /* ------------------ */
        /* --- Translator --- */
        /* ------------------ */

        // This flips the configuration so that it will work with the shift_reg on the Row 4 side.
        void flipSetup();

        /* ------------------------------- */
        /* --- Output Member Functions --- */
        /* ------------------------------- */
        
        // Draw the pixel array onto the LED Matrix.
        void drawDisplay  () const;

        // Draw the pixek array onto the LED Matrix with delay microseconds.
        void drawDisplay (const uint32_t& microseconds) const;
};

#endif
