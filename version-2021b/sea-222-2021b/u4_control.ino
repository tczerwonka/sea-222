////////////////////////////////////////////////////////////////////////////////
// U4_control()
//   code to control U4, a HC259 3-to-8 strobe and latch enable decoder
////////////////////////////////////////////////////////////////////////////////
void U4_control(int u4control) {
  switch (u4control) {
  case RESET:
    digitalWrite(SpiEn, LOW);
    digitalWrite(A_2, LOW);
    digitalWrite(A_1, LOW);
    digitalWrite(A_0, LOW);
    Serial.println("U4 RESET");
    break;
  //IN_LATCH is U13 74HC185D - this sets
  //  U3-D CD4066
  //  "in-latch" -- things like chan, aux, mon, TA, any_key, ign_sense, TEST
  case IN_LATCH:
    digitalWrite(A_0, HIGH);
    digitalWrite(A_1, LOW);
    digitalWrite(A_2, LOW);
    digitalWrite(SpiEn, LOW);
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
