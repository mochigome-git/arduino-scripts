#include <ezButton.h>

/// constants won't change
const int BUTTON_PIN = 2; // the number of the pushbutton pin
const int LED_PIN    = 13; // the number of the LED pin

ezButton button(BUTTON_PIN);  // create ezButton object that attach to pin 7;

// variables will change:
int ledState = LOW;   // the current state of LED

void setup() {
  Serial.begin(9600);         // initialize serial
  pinMode(LED_PIN, OUTPUT);   // set arduino pin to output mode
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
}

void loop() {
  button.loop(); // MUST call the loop() function first

  if(button.isPressed()) {

  if(ledState == LOW)
    Serial.println("The button is pressed");
  if(ledState == HIGH)
    Serial.println("The button is released");

    // toggle state of LED
    ledState = !ledState;

    // control LED arccoding to the toggleed sate
    digitalWrite(LED_PIN, ledState); 
  }
}
