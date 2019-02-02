/** 
 *  Demonstration of how to use the button in query mode(Arduino).
 */

#include <Button.h>

/* Pin, Mode
   - A PULL_DOWN resistor is tied to ground, and the button connects to HIGH on close.
   - A PULL_UP resistor is tied to Vcc, and the button connects to LOW on close.
   - An INTERNAL_PULLUP uses the internal resistor, and the button connects to LOW on close.
*/
Button button(8, Button::INTERNAL_PULLUP);

void setup() {
  Serial.begin(19200);
  Serial.println("Button Event Demo");
  pinMode(13, OUTPUT);
}

void loop() {
  button.process();

  if (button.press()) {
    Serial.println("LOG: Press");
  }
  if (button.held()) {
    Serial.println("LOG: Held");
  }
  digitalWrite(13, button.isDown());
}
 
