#ifndef DISPLAY_PINDEFS_H
#define DISPLAY_PINDEFS_H

/* pindefs.h */
/* define the pins used in the SEA-DMI ESP504 conversion */

/* It may be reasonable to use the Arduino SPI library and       */
/* reassign MISO/MOSI and SCK pins to the hardware intended pins */

//test

const int A_0 = 4;        	//23 - PC5
const int A_1 = 3;        	//22 - PC6
const int A_2 = 2;        	//21 - PC7
//done to here
const int SpiEn = 5;     	//26
const int LockDet = 6;   	//4
const int AdcCsOut = 7;  	//8
const int PttIn = 10;    	//36
const int MOSILOCAL = 11;     	//32
const int MISOLOCAL = 12;     	//31
//const int SCKLOCAL = 13; 	//33
const int U3SW = 9;
const int SCKLOCAL = 8;      	//33 moved from 13 to 8 as 13 has the led
const int LED = 13;

const int TRUE = 1;
const int FALSE = 0;

const int BEEP = A0;     	//9
const int CW = A2;  		//clockwise switch -- 6
const int CCW = A1; 		//counterclockwise switch -- 5

#endif
