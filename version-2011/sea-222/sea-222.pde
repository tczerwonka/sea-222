////////////////////////////////////////////////////////////////////////////
// 222_mod_v002
// Code for an Arduino Pro to replace the processor in a SEA-DMI ESP504
// for the purposes of converting it to be a useable radio in the 1.25M
// Amateur Radio service as a weak-signal SSB radio
//
//   v001 -- can receive
//       Feb 2011 WO9U
//   v002 -- can transmit!
//   v003 -- code cleanup mostly
//

const int A_0 = 4;        //23
const int A_1 = 3;        //22
const int A_2 = 2;        //21
const int SpiEn = 5;     //26
const int LockDet = 6;   //4
const int AdcCsOut = 7;  //8
const int PttIn = 10;    //36
const int MOSILOCAL = 11;     //32
const int MISOLOCAL = 12;     //31
//const int SCKLOCAL = 13;      //33  
const int U3SW = 9;
const int SCKLOCAL = 8;      //33 moved from 13 to 8 as 13 has the led
const int LED = 13;
const int TRUE = 1;
const int FALSE = 0;

const int BEEP = A0;     //9
const int CW = A2;  //clockwise switch -- 6
const int CCW = A1; //counterclockwise switch -- 5


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

//for the U4 main latch
// A2 A1 A0
//0 -- resetU4
int InLatch[] = {
  0, 0, 1};
//2 NC
int FrontPanel[] = {
  0, 1, 1};
int DspLatch[] = {
  1, 0, 0};
int DacEn[] = {
  1, 0, 1};
int SynEn[] = {
  1, 1, 0};
int MainEn[] = {
  1, 1, 1};


void setup() {
  Serial.begin(9600);
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
  //turn on radio -- U3SW is ON if the radio is ON.
  //could be tied high with a resistor (I suppose) if
  //I run out of lines later
  digitalWrite(U3SW, 1);
  pinMode(BEEP, OUTPUT);
  pinMode(CW, INPUT);
  pinMode(CCW, INPUT);

  //set frequency to 222.100
  load_freq();
  delay(100);
  //set the DAC values

  //values appropriate for SN WA2285
  setDAC(60, 35, 38, 43);
  //setDAC(63, 63, 39, 44);

  delay(100);
  //wow this works!
  beep(1000,150);
  beep(1500,100);

}



//global definitions
int incomingByte;  
/////////////////////////////////////////////////////////////////////////
// main loop
//   everything gets done here
/////////////////////////////////////////////////////////////////////////
void loop() {


  Serial.print("Loop start\n");


  //what is the LockDet status?
  if (digitalRead(LockDet) != 1) {
    Serial.println("\tPLL UNLOCKED");
    load_freq();
  }


  delay(100);
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

  //time to transmit?
  //if (!digitalRead(PttIn)) {
  //  tx_mode();
  //}



  delay(500);


} //loop



///////////////////////////////////////////////////////////////
//load_freq
///////////////////////////////////////////////////////////////
void load_freq() {

  Serial.println("in load_freq");
  int lock_detect_status;
  //what is the LockDet status?
  lock_detect_status = digitalRead(LockDet);
  if (lock_detect_status == 1) {
    Serial.print("\tPLL locked\n");
  } 
  else {
    Serial.println("\tPLL UNLOCKED\n");
  }



  //start with 222.100
  //need to program up a MC145158 PLL
  //ref osc is 10.275mhz
  //step is 0.005
  //R range is 3 to 16383
  //N range is 3 to 1024
  //A range is 0 to 127

  //msb first:
  //R is 14 bits, put 1 for control at LSB
  //0000100000000111
  //shift left one
  //000100000000111X
  //AND a 1 to the control
  //0001000000001111

  //A and N are 17 bits together then add 1 for control -- 18 bits
  //N first, 10-bits
  //0000000011010101
  //then A, 7-bits
  //0000000001001011

  //so need to pass in 3 16-bit ints:
  //0123456789ABCDEF 0123456789ABCDEF 0123456789ABCDEF
  //
  //shift N right 8 
  //XXXXXXXX00000012 - 3456789A -- shift that out to PLL
  //shift N left 8
  //3456789AXXXXXXXX
  //shift A left 1
  //000000001234567X
  //and in a 0 at the end
  //0000000012345670
  //now AND that to the N shifted left 8
  //3456789AXXXXXXXX &
  //000000001234567X =
  //3456789A12345670 -- shift that out to the PLL

  //target frequency is 222 + 10.2755 = 232.2755
  //modulus control is /64 or /65
  //in a nutshell:
  // 10.275 / 0.005 = R = 2055
  // N * modulus + A = count
  // count * step = Fintermediat
  // Fintermediate - ref = Fout
  int R = 2055;     //5kc step
  int N = 725;
  int A = 75;

  //assert SpiEn
  Serial.println("writing out RNA to PLL");
  //SynEn set


  //shift in R, N, A on the SPI ports
  //quick and dirty shift out for 222.100, I think...
  //shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 0);
  digitalWrite(SpiEn, 1);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 0);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 16);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 15);
  Serial.println("BOUNCE");
  //delay(1000);
  digitalWrite(A_0, 0);
  digitalWrite(A_1, 1);
  digitalWrite(A_2, 1);
  //delay(100);
  digitalWrite(A_1, 0);
  digitalWrite(A_2, 0);
  //resetU4();
  //digitalWrite(SpiEn, 1);
  //delay(100);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 2);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 213);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 150);
  Serial.println("BOUNCE");
  //delay(100);
  digitalWrite(A_0, 0);
  digitalWrite(A_1, 1);
  digitalWrite(A_2, 1);
  //delay(100);
  digitalWrite(A_1, 0);
  digitalWrite(A_2, 0);

  Serial.println("leaving load_freq\n");
}



///////////////////////////////////////////////////////////////////
//rx_mode
//  anything necessary to go into receive mode
///////////////////////////////////////////////////////////////////
void rx_mode() {
  Serial.println("rx_mode");
  //set the CD4066 in the DSP latch to receive
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 5);
  digitalWrite(A_0, 0);
  digitalWrite(A_1, 0);
  digitalWrite(A_2, 1);
  resetU4();

  return;
}



///////////////////////////////////////////////////////////////////
//tx_mode
//  anything necessary to go into TRANSMIT mode
///////////////////////////////////////////////////////////////////
void tx_mode() {
  Serial.println("TRANSMIT");

  //do I have PLL lock?
  //LED to transmit
  setFrontPanel(TX, 1);
  //set the DAC differently
  setDAC(39, 35, 38, 43);
  //setDAC(41, 63, 39, 44);
  //DSP latch to transmit
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 0);
  digitalWrite(A_0, 0);
  digitalWrite(A_1, 0);
  digitalWrite(A_2, 1);
  resetU4();
  //main latch to unsquelch (maybe)
  //main latch to TX
  digitalWrite(SpiEn, 1);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 128);
  digitalWrite(A_0, 1);
  digitalWrite(A_1, 1);
  digitalWrite(A_2, 1);
  resetU4();
  //maybe something else, I don't know yet
  //frobU406(0);
  //while PTT, loop
  while (!digitalRead(PttIn)) {
    //do nothing but wait here
    //maybe check PLL lock status?
    //...and it started smoking...
    //for (byte j = 0; j < 63; j++) {
    //Serial.println(j);
    //setDAC(63, 57, j, 46);
    //}
  }
  //before return:
  //main latch to RX
  digitalWrite(SpiEn, 1);
  shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 36);
  digitalWrite(A_0, 1);
  digitalWrite(A_1, 1);
  digitalWrite(A_2, 1);
  //un-illuminate the tX light
  //set the DAC values for RX
  setDAC(60, 35, 38, 43);
  //setDAC(63, 63, 39, 44);
  rx_mode();
  return;
}



/////////////////////////////////////////////////////////////////////////
// squelch
/////////////////////////////////////////////////////////////////////////
void squelch(int state) {
  Serial.println("in squelch");
  rx_mode();
  //the following three are "main enable"
  //which enables U407, main latch
  //U407 is a MC14094B

  resetU4();

  //1,1,1 goes to U407 main latch
  //send via spi d32 and d4 -- d36 is 0x24
  if (state==1) {
    Serial.println("\topen squelch\n");
    //open squelch, rx on
    //SPI.transfer(0x24);
    digitalWrite(SpiEn, 1);
    shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 32);
    //this a1, a2, a0 sequence chosen to not overlap with other
    //possible output states of U4 that don't require SpiEn
    //strange that the a1 a2 a0 ordering causes the radio to tx
    //immediately
    digitalWrite(A_0, 1);
    digitalWrite(A_1, 1);
    digitalWrite(A_2, 1);
  } 
  else {
    Serial.println("\tclose squelch\n");
    //close squelch, rx off
    //SPI.transfer(0x20);
    digitalWrite(SpiEn, 1);
    shiftOut(MOSILOCAL, SCKLOCAL, MSBFIRST, 36);
    digitalWrite(A_0, 1);
    digitalWrite(A_1, 1);
    digitalWrite(A_2, 1);
  }


  resetU4();
}



//read the state of the ptt line
int readPtt() {
  //read the state of the ptt line
  if (digitalRead(PttIn)) {
    //PTT engaged
    Serial.println("PTT ON\n");
    return (TRUE);
  } 
  else {
    // no ptt
    Serial.println("ptt off\n");
    return(FALSE);
  }
}


///////////////////////////////////////////////////////////////////
//readFrontPanel
//  read the state of the buttons on the front panel
///////////////////////////////////////////////////////////////////
//read the state of the front panel buttons
int readFrontPanel () {
  int foo;
  resetU4();
  //enable the in-latch, U13 which is a 74HC165
  //read the data on MISOLOCAL on each clock
  //this is basically the latch
  digitalWrite(A_0, 1);
  digitalWrite(A_1, 0);
  digitalWrite(A_2, 0);
  digitalWrite(SpiEn, 0);

  foo = shiftIn(MISOLOCAL, SCKLOCAL, MSBFIRST);
  resetU4();
  foo = foo ^ 255;

  Serial.print("readFrontPanel: ");
  Serial.print(foo);
  Serial.print("\n\t");
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

  Serial.print("\n");
  return foo;

}



///////////////////////////////////////////////////////////////////
//setFrontPanel
//    improved function to write to a UCN5821
//    setFrontPanel(INDICATOR, state);
//    FPstate holds the current state of the indicator lights
//    INDICATOR is the light to be activated on
//    state is 1 for on 0 for off.  Easy.
//	const byte BLANK = 0;
//	const byte RNGE = 2;
//	const byte CALL = 8;
//	const byte TX = 64
//	const byte ONE = 4;
//	const byte TWO = 1;
//	const byte THREE = 16;
//	const byte FOUR = 128;
//	byte FPstate = 0;
//
///////////////////////////////////////////////////////////////////
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
  setU4(FrontPanel);
  resetU4();

  return;
}	



///////////////////////////////////////////////////////////////////
// beep(frequency, duration in ms)
//    make square waves!
//    a potential problem here is this function waits
///////////////////////////////////////////////////////////////////
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



////////////////////////////////////////////////////////////////////
//resetU4
//  write zeros to all of the U4 inputs
////////////////////////////////////////////////////////////////////
void resetU4() {
  //Serial.print("resetU4\n");
  digitalWrite(SpiEn, 0);
  //i think it should be a0 first
  digitalWrite(A_2, 0);
  digitalWrite(A_1, 0);
  digitalWrite(A_0, 0);
  return;
}


////////////////////////////////////////////////////////////////////
// single place to write control out to U4
////////////////////////////////////////////////////////////////////
void setU4(int control[2]) {
  digitalWrite(A_1, control[1]);
  digitalWrite(A_0, control[0]);
  digitalWrite(A_2, control[2]);
}



////////////////////////////////////////////////////////////////////
//frobU406
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
////////////////////////////////////////////////////////////////////
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
  resetU4();
  delay(100);
  //} //for
  //Serial.print("\n");
  return;
}







void foo() {
  //just do some math
  //given four values that represent 6-bit ints
  //A, B, C, D
  //do some math to come out with three 8-bit ints
  //X, Y, Z
  byte A = 12;
  byte B = 31;
  byte C = 23;
  byte D = 3;
  Serial.println("values:");
  Serial.println(A, BIN);
  Serial.println(B, BIN);
  Serial.println(C, BIN);
  Serial.println(D, BIN);
  //shift A left two
  byte Ashiftl = A << 2;
  Serial.println("Ashiftl 2");
  Serial.println(Ashiftl, BIN);
  //shift B right 4
  byte Bshiftr = B >> 4;
  Serial.println("Bshiftr 4");
  Serial.println(Bshiftr, BIN);
  //X gets Ashiftl and BshiftR
  byte X = Ashiftl | Bshiftr;
  Serial.println("X is Ashiftl | Bshiftr");
  Serial.println(X, BIN);
  //Y gets B shift left 4 
  byte Bshiftl = B << 4;
  Serial.println("B shift left 4");
  Serial.println(Bshiftl, BIN);
  Serial.println("C shift rt 2");
  byte Cshiftr = C >> 2;
  Serial.println(Cshiftr, BIN);
  Serial.println("Y is Bshiftl | C shiftr");
  byte Y = Bshiftl | Cshiftr;
  Serial.println(Y, BIN);
  Serial.println("Cshiftl 6");
  byte Cshiftl = C << 6;
  Serial.println(Cshiftl, BIN);
  Serial.println("Z is Cshiftl | D");
  byte Z = Cshiftl | D;
  Serial.println(Z, BIN);



  return;


}


