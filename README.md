# LED_8x8

Library for Displaying Patterns on a 1588AS 8x8 LED Display with a 74HC595 shift regesiter.

### Example:
This is a 'pixel array,' where each column of bits after 0b represents a column of led's on the led matrix. 
```c++
{
    0b11000011,
    0b00000000,
    0b01100110,
    0b01100110,
    0b00000000,
    0b01111110,
    0b01000010,
    0b00111100
}
```
Then we initialize the display struct:
```c++
#define SRCLK_PIN 2  // All pin values are customizable 😊.
#define RCLK_PIN  3
#define DATA_PIN  4

const uint8_t shift_pins    [N_SHIFT_PINS    ] = {DATA_PIN, SRCLK_PIN, RCLK_PIN}; 
const uint8_t hard_wire_pins[N_HARD_WIRE_PINS] = {12, 11, 10, 9, 8, 7, 6, 5    };  

LED_Display display;

void setup() 
{
  display.init(shift_pins, hard_wire_pins, pixel_drawing);
}
```
Finally, we can display the image:
```c++
void loop() 
{
  display.drawDisplay();
}
```
And the result is:
![edit](https://user-images.githubusercontent.com/19739107/162891844-0e13e8de-ce34-4bfc-a8f6-730fc5d4cef6.jpg)

