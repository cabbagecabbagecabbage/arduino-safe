#include <Key.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 3;
const int led = 10;
const int redPin = 12;
const int greenPin = 10;
const int buttonPin = 2;
const int servoPin = 11;
int buttonState = 0;
bool isOpen = false;
bool isNew = false;
int cursorPos = 0;
String password = "0000";
String curInput = "";
Servo door;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3',},
  {'4', '5', '6',},
  {'7', '8', '9',},
  {'*', '0', '#',}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); //setting up the keypad using the Keypad Library
LiquidCrystal_I2C lcd(0x3f, 16, 2);  //setting up the display using the LiquidCrystal_I2C Library. lcd addresses may vary.

void closeDoor(){
  isOpen = false;
  door.write(0);
  analogWrite(redPin,255);
  analogWrite(greenPin,0);
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Enter Password");
}

void unlock(){
  lcd.setCursor(0,0); 
  lcd.print("Door Opened!");
  analogWrite(redPin,0);
  analogWrite(greenPin,255);
  isOpen = true;
  door.write(90);
}

void lock(){
  lcd.setCursor(0,0); 
  lcd.print("Incorrect");
  analogWrite(redPin,255);
  analogWrite(greenPin,0);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Enter Password");
}

void setup(){
  Serial.begin(9600);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  analogWrite(redPin,255);
  analogWrite(greenPin,0);
  lcd.setCursor(0,0); 
  lcd.print("Enter Password");
  door.attach(servoPin);
  door.write(0);
}
  
void loop(){
  buttonState = digitalRead(buttonPin);
  
  if (isOpen and buttonState == LOW){
    closeDoor();
  }
  if (isOpen) return;
  char customKey = customKeypad.getKey();
  
  if (customKey and customKey != '*' and customKey != '#' and cursorPos < 8){
    lcd.setCursor(cursorPos++,1); 
    lcd.print(customKey);
    curInput += customKey;
  }
  else if (customKey == '*' and curInput != ""){
  //check password
    lcd.clear();
    Serial.println(curInput);
    if (curInput == password) unlock();
    else lock();
    curInput = "";
    cursorPos = 0;
  }
  else if (customKey == '#'){
    if (isNew){
      password = curInput;
      curInput = "";
      cursorPos = 0;
      isNew = false;
      lcd.clear();
      lcd.setCursor(0,0); 
      lcd.print("Password Changed!");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0,0); 
      lcd.print("Enter Password");
      lcd.setCursor(0,1);
    }
    else if (curInput == password){
      //password change process
      curInput = "";
      cursorPos = 0;
      lcd.clear();
      lcd.setCursor(0,0); 
      lcd.print("New Password:");
      lcd.setCursor(0,1);
      isNew = true;
    }
  }
}
