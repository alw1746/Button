/** 
 *  Demonstration of how to use the button in query mode(STM32).
 */

#include <Button.h>

/* Pin, Mode
   - A PULL_DOWN resistor is tied to ground, and the button connects to HIGH on close.
   - A PULL_UP resistor is tied to Vcc, and the button connects to LOW on close.
   - An INTERNAL_PULLDOWN uses the internal resistor, and the button connects to HIGH on close.
     +---->PB6
     |
     \
     |
     +---->3.3V
*/

Button button(PB6, Button::INTERNAL_PULLDOWN);

#define Serial Serial1

void setup() {
  Serial.begin(115200);
  Serial.println("Button Event Demo");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  button.process();
  if (button.press()) {
    Serial.println("LOG: Press");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  if (button.held()) {
    Serial.println("LOG: Held");
  }
}
 
