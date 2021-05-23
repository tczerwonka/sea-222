/* alternative shiftIn for testing - not needed */
uint8_t my_shiftIn_msbFirst(uint8_t dataPin, uint8_t clockPin)
{
        uint8_t mask, value=0;
        for (mask=0x80; mask; mask >>= 1) {
                digitalWrite(clockPin, HIGH);
                delayMicroseconds(1);
                if (digitalRead(dataPin)) value |= mask;
                digitalWrite(clockPin, LOW);
                delayMicroseconds(1);
        }
        return value;
}
