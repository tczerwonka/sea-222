/*
   sea-222-2021.pde
   T Czerwonka WO9U
   February 2021
   https://github.com/tczerwonka/sea-222

   Project to take the ESP504 radio move it to the US 222 amateur
   band.  The 2011 code (see elsewhere in the project) was minimally
   working but really needs to be completely re-written.

   Additionally decent functions for the PLL needed to be written
   then and that's still true today.
   Plus the A register for the PLL is 63/64 and that's it.

*/

#include "pindefs.h"
#include "esp_macros.h"




////////////////////////////////////////////////////////////////////////////////
//function prototypes
////////////////////////////////////////////////////////////////////////////////
void radio_enable(int power_state);
void load_frequency(unsigned long frequency);
void U4_control(int u4control);
int readFrontPanel(void);
void beep (int freq, int duration);
void setFrontPanel(byte light, int state);
void squelch(int state);
void setDAC(byte D, byte C, byte B, byte A);



////////////////////////////////////////////////////////////////////////////////
//initial values for the PLL -- all values in Hz
////////////////////////////////////////////////////////////////////////////////
unsigned long l_frequency = 222100000; 
unsigned long l_step = 5000;
unsigned long l_reference_oscillator = 10275000;
int modulus = 64; //fixed on this PLL


////////////////////////////////////////////////////////////////////////////////
// other globals
////////////////////////////////////////////////////////////////////////////////
char szStr[20]; 	//for debug printing
int frontPanelByte;
int LEDstate = 0;
//int squelch_state = CLOSED;



////////////////////////////////////////////////////////////////////////////////
// setup
////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("========================================");
  pinMode(A_0, OUTPUT);
  pinMode(A_1, OUTPUT);
  pinMode(A_2, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(SpiEn, OUTPUT);
  pinMode(LockDet, INPUT);
  pinMode(AdcCsOut, OUTPUT);
  pinMode(PttIn, INPUT);
  pinMode(SCKLOCAL, OUTPUT);
  pinMode(MOSILOCAL, OUTPUT);
  pinMode(MISOLOCAL, INPUT);
  pinMode(U3SW, OUTPUT);
  pinMode(BEEP, OUTPUT);
  pinMode(CW, INPUT);
  pinMode(CCW, INPUT);
  
  //turn on radio -- U3SW is ON if the radio is ON.
  radio_enable(1);

  //set frequency to 222.100
  load_frequency(l_frequency);
  delay(100);
  //set the DAC values

  //values appropriate for SN WA2285 RX
  setDAC(60, 35, 38, 43);

  delay(100);
  beep(1000, 150);
  beep(1500, 100);


}



////////////////////////////////////////////////////////////////////////////////
// loop
////////////////////////////////////////////////////////////////////////////////
void loop() {
  Serial.println("loop start\n==========");
  //toggle D13 (LED) on every loop
  if (LEDstate == 0) {
    digitalWrite(LED, 1);
    LEDstate = 1;
    Serial.print(".");
  } else {
    digitalWrite(LED, 0);
    LEDstate = 0;
  }

  frontPanelByte = readFrontPanel();

  delay(100);

  if (frontPanelByte & 128) {
    //scn
    setFrontPanel(RNGE, 1);
  }
  else {
    setFrontPanel(RNGE, 0);
  }


/*
  incomingByte = readFrontPanel();

  if (incomingByte == 128) {
    //scn
    setFrontPanel(RNGE, 1);
  } 
  else {
    setFrontPanel(RNGE, 0);
  }

  if (incomingByte == 2) {
    //hrn
    //testfn();
    squelch(1);
  } 
  else {
    squelch(0);
  }
*/

  delay(1000);
} //loop



////////////////////////////////////////////////////////////////////////////////
// radio_enable()
//  turn on the radio
////////////////////////////////////////////////////////////////////////////////
void radio_enable(int power_state) {
  if (power_state == 1) {
    digitalWrite(U3SW, 1);
    Serial.println("radio power on");
  } else {
    digitalWrite(U3SW, 0);
    Serial.println("radio power off");
  }
} //radio_enable






////////////////////////////////////////////////////////////////////////////////
// rx_mode
//   anything needed to receive
////////////////////////////////////////////////////////////////////////////////
void rx_mode() {
  Serial.println("rx_mode");
  //set the CD4066 (U3-D) in the DSP latch to receive
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 5);
  U4_control(DSP_LATCH);
  U4_control(RESET);

  return;
}




////////////////////////////////////////////////////////////////////////////////
// in_latch
//   in_latch is U13, a 74HC165D - controlled by U4_control(IN_LATCH)
//   74HC165D is a 8-bit parallel in / serial-out shift register
////////////////////////////////////////////////////////////////////////////////
void in_latch(int in_latchvar) {
  switch (in_latchvar) {
    case MON:
      U4_control(IN_LATCH);
      U4_control(RESET);
  } //switch
} //in_latch



////////////////////////////////////////////////////////////////////////////////
// beep(frequency, duration in ms)
//   make square waves!
////////////////////////////////////////////////////////////////////////////////
void beep (int freq, int duration) {
  //1000000 divided by two divided by the frequency is the
  //number of periods, or so I have convinced myself.
  int period = 500000 / freq;
  for (int i = 0; i < duration; i++) {
    digitalWrite(BEEP, HIGH);
    delayMicroseconds(period);
    digitalWrite(BEEP, LOW);
    delayMicroseconds(period);
  }

  return;
}



////////////////////////////////////////////////////////////////////////////////
//setFrontPanel
//    improved function to write to a UCN5821
//    setFrontPanel(INDICATOR, state);
//    FPstate holds the current state of the indicator lights
//    INDICATOR is the light to be activated on
//    state is 1 for on 0 for off.  Easy.
//      const byte BLANK = 0;
//      const byte RNGE = 2;
//      const byte CALL = 8;
//      const byte TX = 64
//      const byte ONE = 4;
//      const byte TWO = 1;
//      const byte THREE = 16;
//      const byte FOUR = 128;
//      byte FPstate = 0;
//
//	this needs work
//
////////////////////////////////////////////////////////////////////////////////
void setFrontPanel(byte light, int state) {
  Serial.print("setFrontPanel: ");
  if (state == TRUE) {
    Serial.print(" on ");
    //bitwise or
    FPstate |= light;
  }
  else {
    Serial.print(" off ");
    //bitwise and of the not of light
    FPstate &= ~light;
  } //false
  Serial.println(FPstate,BIN);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, FPstate);
  U4_control(LED_EN);
  U4_control(RESET);

  return;
} //setFrontPanel



////////////////////////////////////////////////////////////////////////////////
// squelch
////////////////////////////////////////////////////////////////////////////////
void squelch(int state) {
  Serial.println("in squelch");
  rx_mode();
  U4_control(RESET);

  //enable U407 main latch
  //send via spi d32 and d4 -- d36 is 0x24
  if (state==1) {
    Serial.println("\topen squelch\n");
    //open squelch, rx on
    //SPI.transfer(0x24);
    digitalWrite(SpiEn, 1);
    shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 32);
    U4_control(MAIN_EN);
  } else {
    Serial.println("\tclose squelch\n");
    //close squelch, rx off
    //SPI.transfer(0x20);
    digitalWrite(SpiEn, 1);
    shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 36);
    U4_control(MAIN_EN);
  }
  U4_control(RESET);
} //squelch
