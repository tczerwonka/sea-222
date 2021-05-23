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
  foo = foo ^ 255;
  delay(100);
  U4_control(RESET);



    Serial.print("readFrontPanel: ");
    Serial.print(foo);
    Serial.print("\t");

  if (foo & 2) {
    Serial.print("HRN ");
    //squelch(1);
  }
  if (foo & 4) {
    Serial.print("PGM ");
  }
  if (foo & 8) {
    Serial.print("AUX ");
    //squelch(0);
  }
  if (foo & 128) {
    Serial.print("SCN ");
  }

  if (foo) Serial.println();
  

  return foo;
} //readFrontPanel
