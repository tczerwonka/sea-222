/*
   sea-222-2021.pde
   T Czerwonka WO9U
   February 2021
   https://github.com/tczerwonka/sea-222

   Project to take the ESP504 radio move it to the US 222 amateur
   band.  The 2022 code (see elsewhere in the project) was minimally
   working but really needs to be completely re-written.
   Additionally decent functions for the PLL needed to be written
   then and that's still true today.
   Plus the A register for the PLL is 63/64 and that's it.

*/

#include "pindefs.h"

char szStr[20];

//initial values for the PLL -- all values in Hz
unsigned long l_frequency = 222100000; 
unsigned long l_step = 5000;
unsigned long l_reference_oscillator = 10275000;
int modulus = 64; //fixed on this PLL

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


  load_frequency(l_frequency);
  
  //turn on radio -- U3SW is ON if the radio is ON.
  radio_enable(1);

  //set frequency to 222.100
  load_frequency(l_frequency);
  //load_freq();
  //delay(100);
  //set the DAC values

  //values appropriate for SN WA2285
  //setDAC(60, 35, 38, 43);

  //delay(100);
  //wow this works!
  //beep(1000, 150);
  //beep(1500, 100);


}

void loop() {
  // put your main code here, to run repeatedly:
  delay(50);
  l_frequency += 5000;
  load_frequency(l_frequency);

  Serial.println("X");

}



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
  
  Serial.print("desired frequency: ");
  sprintf( szStr, "%09lu", l_frequency );
  Serial.println( szStr );

  //calculate target frequency
  unsigned long l_target = l_frequency + l_reference_oscillator;
  Serial.print("target frequency: ");
  sprintf( szStr, "%09lu", l_target );
  Serial.println( szStr );

  //R register is reference oscillator divided by desired step
  int R = l_reference_oscillator / l_step;
  Serial.print("R: ");
  Serial.println(R);

  //need to find the N and A
  //l_target / step = some large integer
  //large integer divided by modulus is some whole and some remainder
  //whole is N, remainder is A
  unsigned long l_divisor = l_frequency / l_step;
  unsigned long N = l_divisor / modulus;
  Serial.print("N: ");
  Serial.println(N);

  unsigned long A = l_divisor - (N * modulus);
  Serial.print("A: ");
  Serial.println(A);

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

  //At this point we have R, N, A -- need to load into the PLL


  
} //load_frequency
