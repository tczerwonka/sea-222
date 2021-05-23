void setup() {

  Serial.begin(9600);

  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(1, INPUT); //miso
  pinMode(5, OUTPUT); //clock

  delay(1000);
  //turn on radio
  pinMode(25, OUTPUT);
  digitalWrite(25, 1);

}

void loop() {
  int foo;


  digitalWrite(8, LOW);
  digitalWrite(4, LOW);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
  Serial.println("U4 RESET");
  //delayMicroseconds(10000);

  digitalWrite(8, LOW);
  digitalWrite(4, HIGH);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
  Serial.println("U4 INLATCH");
  //delayMicroseconds(10000);


  foo = shiftIn(1, 5, MSBFIRST);
  //foo = my_shiftIn_msbFirst(1, 32);
  foo = foo ^ 255;

  Serial.println(foo);

  
  digitalWrite(8, LOW);
  digitalWrite(4, LOW);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
  Serial.println("U4 RESET");
  //delayMicroseconds(10000);


  delay(100);

}


uint8_t my_shiftIn_msbFirst(uint8_t dataPin, uint8_t clockPin)
{

  uint8_t mask, value = 0;

  for (mask = 0x80; mask; mask >>= 1) {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(10);
    if (digitalRead(dataPin)) value |= mask;
    digitalWrite(clockPin, LOW);
    delayMicroseconds(10);
  }
  return value;

}
