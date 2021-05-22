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
  } else {
    digitalWrite(LED, 0);
    LEDstate = 0;
  }
	
  frontPanelByte = readFrontPanel();
  if (frontPanelByte == 128) {
    //scn
    setFrontPanel(RNGE, 1);
  }
  else {
    setFrontPanel(RNGE, 0);
  }

  if (frontPanelByte == 2) {
    //hrn
    //testfn();
    squelch(1);
  }
  else {
    squelch(0);
  }


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
// load_frequency()
//    load a frequency into the MC145158 PLL
//    Frequency input is in hz
////////////////////////////////////////////////////////////////////////////////
void load_frequency(unsigned long frequency) {
  Serial.print("load_frequency: ");
  Serial.println(frequency);

  //Serial.print("desired frequency: ");
  //sprintf( szStr, "%09lu", l_frequency );
  //Serial.println( szStr );

  //calculate target frequency
  unsigned long l_target = l_frequency + l_reference_oscillator;
  //Serial.print("target frequency: ");
  //sprintf( szStr, "%09lu", l_target );
  //Serial.println( szStr );

  //R register is reference oscillator divided by desired step
  int R = l_reference_oscillator / l_step;
  //Serial.print("R: ");
  //Serial.println(R);

  //need to find the N and A
  //l_target / step = some large integer
  //large integer divided by modulus is some whole and some remainder
  //whole is N, remainder is A
  unsigned long l_divisor = l_frequency / l_step;
  unsigned long N = l_divisor / modulus;
  //Serial.print("N: ");
  //Serial.println(N);

  unsigned long A = l_divisor - (N * modulus);
  //Serial.print("A: ");
  //Serial.println(A);

  //R range -- 3 to 16383
  //N range -- 3 to 1023
  //if outside that set an error 
  //  (likely the step is too low)
  if (R > 16383) {
    Serial.println("ERROR: R exceeds 16383 - step too low");
    delay(1000); //raise some sort of general error in future
  }
  if (N > 1023) {
    Serial.println("ERROR: N exceeds 1023");
    delay(1000); //raise some sort of general error in future
  }

  //e.g.
  //desired frequency: 222100000
  //target frequency: 232375000
  //R: 2055
  //N: 694
  //A: 4

  //this could all be condensed in a few statements and potentially
  //simplified but when I look at this (again) in ten years it's less
  //likely that I'll curse the long description than a summary
  //my view is the MSB is on the "left"

  //At this point we have R, N, A -- need to load into the PLL
  //R is 14 bits -- put one for control at LSB -- total of 15 bits 
  //A and N are 17 bits together, 7 bits for A, 10 bits for N
  //  -- then put one for control at LSB position -- 18 bits total
  //now shift out in 3 16-bit ints -- shift must go MSB first
  //shiftOut works on bytes -- something like this needs to be done
  //shiftOut(dataPin, clock, MSBFIRST, (data >> 8));
  
  //ideal
  //RRRRRRRRRRRRRRC NNNNNNNNNNAAAAAAAC
  //M            L1 M        LM     L0
  // S            S  S        SS     S
  //  B            B  B        BB     B
 
  //OH I SUPPOSE
  //RRRRRRRRRRRRRR1 EN 000000NN NNNNNNNNAAAAAAA0 EN
  //-------R-------    ---MM--- -------NN-------

  //Serial.print("R: 0x");
  //sprintf( szStr, "%X", R );
  //Serial.println( szStr );
  R = R << 1; //shift left
  //Serial.print("R bsl: 0x");
  //sprintf( szStr, "%X", R );
  //Serial.println( szStr );
  //2055d -> 0x807 -> 0x100E
  //0b100000000111 -> 0b1000000001110
  R = R | 1; // OR the one at the LSB?
  //Serial.print("R OR 1: 0x");
  //sprintf( szStr, "%X", R );
  //Serial.println( szStr );
  //0x100E -> 0x100f
  //0b1000000001110 -> 0b1000000001111 ---- looks right


  //need two MSB bits of a 10-bit N in LSB-most position of a unsigned int
  // e.g. 000000NNNNNNNNNN -> 000000NN + NNNNNNNN00000000
  // NNNNNNNN--------
  
  //   -- make a copy of N, shift down 8 bits as N is only 10 bits
  unsigned long N_PART;
  N_PART = N >> 8;
  //now shift N up 8 bits -- that will roll off the first two and leave the last 8 as zero
  N = N << 8;
  //shift A up one bit - LSB becomes control zero
  A = A << 1;
  //OR A on lower part of N
  N = N | A;

  //Serial.print("N_PART: 0x");
  //sprintf( szStr, "%X", N_PART );
  //Serial.println( szStr );

  //Serial.print("N: 0x");
  //sprintf( szStr, "%X", N );
  //Serial.println( szStr );

  //desired frequency: 222100000
  //target frequency: 232375000
  //R: 2055
  //N: 694 -- 0x2B6 -- 0b00000010 10110110
  //A: 4
  //R: 0x807
  //R bsl: 0x100E
  //R OR 1: 0x100F
  //N_PART: 0x2 -- right -- that's just 0b10
  //N: 0xB608 -- 0b10110110 0000100 0

  digitalWrite(SpiEn, 1);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, R);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, (R >> 8));
  //now toggle ENABLE
  U4_control(PLL_PGM);
  U4_control(RESET);

  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, N_PART);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, N);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, (N >> 8));
  //now toggle ENABLE
  U4_control(PLL_PGM);
  U4_control(RESET);
  digitalWrite(SpiEn, 0);
  
  Serial.println("finished load_frequency");
} //load_frequency



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
// U4_control()
//   code to control U4, a HC259 3-to-8 strobe and latch enable decoder
////////////////////////////////////////////////////////////////////////////////
void U4_control(int u4control) {
  switch (u4control) {
  case RESET:
    digitalWrite(SpiEn, 0);
    digitalWrite(A_2, 0);
    digitalWrite(A_1, 0);
    digitalWrite(A_0, 0);
    Serial.println("U4 RESET");
    break;
  //IN_LATCH is U13 74HC185D - this sets
  //  U3-D CD4066
  //  "in-latch" -- things like chan, aux, mon, TA, any_key, ign_sense, TEST
  case IN_LATCH:
    digitalWrite(A_0, 1);
    digitalWrite(A_1, 0);
    digitalWrite(A_2, 0);
    digitalWrite(SpiEn, 0);
    Serial.println("U4 IN_LATCH");
    break;
  //LED_EN
  //   LED and DTMF enable
  case LED_EN:
    digitalWrite(SpiEn, 0);
    digitalWrite(A_2, 0);
    digitalWrite(A_1, 1);
    digitalWrite(A_0, 1);
    Serial.println("U4 LED_EN");
    break;
  //DSP_LATCH CD4066
  //  
  case DSP_LATCH:
    digitalWrite(SpiEn, 0);
    digitalWrite(A_2, 1);
    digitalWrite(A_1, 0);
    digitalWrite(A_0, 0);
    Serial.println("U4 DSP_LATCH");
    break;
  case DAC_EN:
    digitalWrite(SpiEn, 1);
    digitalWrite(A_2, 1);
    digitalWrite(A_1, 0);
    digitalWrite(A_0, 1);
    Serial.println("U4 DAC_EN");
    break;
  case PLL_PGM:
    digitalWrite(SpiEn, 1);
    digitalWrite(A_2, 1);
    digitalWrite(A_1, 1);
    digitalWrite(A_0, 0);
    Serial.println("U4 PLL_PGM");
    break;
  case MAIN_EN:
    //enables U407, main latch -- U407 is a MC14094B
    //digitalWrite(SpiEn, 1);
    digitalWrite(A_0, 1);
    digitalWrite(A_1, 1);
    digitalWrite(A_2, 1);
    Serial.println("U4 MAIN_EN");
    break;
  default:
    Serial.println("U4 ...lol wut?");
    break;
  }
} //U4_control



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
// readFrontPanel
//   read the state of the buttons on the front panel
////////////////////////////////////////////////////////////////////////////////
int readFrontPanel() {
  int foo;
  //enable the in-latch, U13 which is a 74HC165
  //read the data on MISOLOCAL on each clock
  U4_control(RESET);
  U4_control(IN_LATCH);
  foo = shiftIn(MISOLOCAL, SCKLOCAL, MSBFIRST);
  U4_control(RESET);
  foo = foo ^ 255;


  Serial.print("readFrontPanel: ");
  Serial.print(foo);
  Serial.print("\t");
  if (foo & 2) {
    Serial.print("HRN ");
  }
  if (foo & 4) {
    Serial.print("PGM ");
  }
  if (foo & 8) {
    Serial.print("AUX ");
  }
  if (foo & 128) {
    Serial.print("SCN ");
  }

  Serial.println();
  return foo;
} //readFrontPanel



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
  }
  else {
    Serial.println("\tclose squelch\n");
    //close squelch, rx off
    //SPI.transfer(0x20);
    digitalWrite(SpiEn, 1);
    shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 36);
    U4_control(MAIN_EN);
  }
  U4_control(RESET);
} //squelch



////////////////////////////////////////////////////////////////////////////////
//setDAC
//  U406 is an MC144111 6-bit DAC
//
// the way this works is to set the latch, write out three 8-bit 
// values -- 24 bits -- which is four 6-bit values -- 24-bits.
//
// A is Q1, TP410, AGC sense thresh/pwr fine
//  range is 0-8v
// B is Q2, TP409, AFC reference
//  range is 0-8v
// C is Q3, TP408, FB phase control
//  range is 0-8v
// D is Q4, TP107, AGC fine, RF pwr level
//  range is 0-8v
//
// that's about 0.127 per int for Q1, Q2, Q3, Q4
// Measured values on ESP504 S/N WA2285
//       VOLTS       calc INT
// Q1  7.54  4.91    60    39
// Q2  4.19  4.19    35    35
// Q3  4.62  4.62    38    38
// Q4  5.27  5.29    43    43
//      RX    TX     RX    TX
////////////////////////////////////////////////////////////////////////////////
void setDAC(byte D, byte C, byte B, byte A) {



  Serial.println("setDAC");
  //for (int j=0; j < 63; j++) {
  //byte A = j; //Q1 TP410 -- agc sense thresh/pwr fine
  //byte B = j; //Q2 TP408? -- AFC ref
  //byte C = j; //Q3 TP408 -- FB phase control
  //byte D = j; //Q4 TP107 -- agc fine, rf pwr level
  //shift A left two
  byte Ashiftl = A << 2;
  //shift B right 4
  byte Bshiftr = B >> 4;
  //X gets Ashiftl and BshiftR
  byte X = Ashiftl | Bshiftr;
  //Y gets B shift left 4 
  byte Bshiftl = B << 4;
  byte Cshiftr = C >> 2;
  byte Y = Bshiftl | Cshiftr;
  byte Cshiftl = C << 6;
  byte Z = Cshiftl | D;

  Serial.print(X, BIN);
  Serial.print("|");
  Serial.print(Y, BIN);
  Serial.print("|");
  Serial.print(Z, BIN);
  Serial.print("\n ");


  digitalWrite(A_2, 1);
  digitalWrite(A_1, 0);
  digitalWrite(A_0, 1);
  delay(100);
  digitalWrite(SpiEn, 1);
  delay(100);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, X);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, Y);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, Z);
  delay(100);
  U4_control(RESET);
  delay(100);
  //} //for
  //Serial.print("\n");
  return;
}
