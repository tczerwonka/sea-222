#ifndef ESP_MACROS_H
#define ESP_MACROS_H

/* esp_macros.h */

/* U4 HC259 */
const int RESET = 0;
const int IN_LATCH = 1;
const int LED_EN = 3;
const int DSP_LATCH = 4;
const int DAC_EN = 5;
const int PLL_PGM = 6
const int MAIN_EN = 7;

/* U13 HC74C185D in_latch */
const int TA = 2; 	//D0 - A
const int AUX = 4; 	//D1 - B
const int CHAN = 8;	//D2 - C
const int IGN_SENSE = 16;	//D3 - D
const int TEST = 32;	//D4 - E
const int ANY_KEY = 64; //D5 - F
const int MON_SCAN = 128; //D5 - G

#endif
