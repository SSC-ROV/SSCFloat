## Printing out serial port testing button


#if 0
#define BUTTON_PIN_1 13    // Bottom limit switch
#define BUTTON_PIN_2 14    // Top limit switch

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);
}

void loop() {
    // Read button states (pressed = LOW due to internal pull-ups)
    int buttonState1 = digitalRead(BUTTON_PIN_1);
    int buttonState2 = digitalRead(BUTTON_PIN_2);
    
    // Control LEDs based on button state
    digitalWrite(BUTTON_PIN_1, buttonState1 == LOW ? HIGH : LOW);
    digitalWrite(BUTTON_PIN_2, buttonState2 == LOW ? HIGH : LOW);
    
    // Optional: print button states to Serial for debugging
    Serial.print("D13: "); Serial.print(buttonState1);
    Serial.print(" | D14: "); Serial.println(buttonState2);
    
    delay(500);
}
#endif