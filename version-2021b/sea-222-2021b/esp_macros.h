#ifndef ESP_MACROS_H
#define ESP_MACROS_H

/* esp_macros.h */
const int TRUE = 1;
const int FALSE = 0;

/* U4 HC259 */
const int RESET = 0;
const int IN_LATCH = 1;
const int LED_EN = 3;
const int DSP_LATCH = 4;
const int DAC_EN = 5;
const int PLL_PGM = 6;
const int MAIN_EN = 7;

const int MON = 5;

/* U13 HC74C185D in_latch */
const int TA = 2; 	//D0 - A - 11
const int AUX = 4; 	//D1 - B - 12
const int CHAN = 8;	//D2 - C - 13
const int IGN_SENSE = 16;	//D3 - D - 14
const int TEST = 32;	//D4 - E - 3
const int ANY_KEY = 64; //D5 - F - 4
const int MON_SCAN = 128; //D5 - G - 5


//for the front panel
const byte BLANK = 0;
const byte RNGE = 2;
const byte CALL = 8;
const byte TX = 64;
const byte ONE = 4;
const byte TWO = 1;
const byte THREE = 16;
const byte FOUR = 128;
byte FPstate = 0;

#endif
