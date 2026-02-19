#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 3; 

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};


byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const String password = "789"; 
String currentInput = "";
const int commPin = 10;
bool isArmed = false;

void setup() {
  Serial.begin(9600); 
  
  pinMode(commPin, OUTPUT);
  digitalWrite(commPin, LOW);
  
  Serial.println("Arduino Uno Access Panel Ready.");
  Serial.println("Enter PIN and press '5' to toggle the alarm.");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '5') { 
      Serial.println(); 
      
      if (currentInput == password) {
        isArmed = !isArmed; 
        
        if (isArmed) {
          digitalWrite(commPin, HIGH);
          Serial.println("SUCCESS: System ARMED.");
        } else {
          digitalWrite(commPin, LOW);
          Serial.println("SUCCESS: System DISARMED.");
        }
      } else {
        Serial.println("ACCESS DENIED: Wrong PIN.");
      }
      currentInput = ""; 
    } 
    else if (key == '*') { 
      currentInput = "";
      Serial.println("\nInput Cleared.");
    } 
    else {
      Serial.print("*"); 
      currentInput += key; 
    }
  }
}