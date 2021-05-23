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
//
//setDAC
//10101110|1101000|11111100
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
