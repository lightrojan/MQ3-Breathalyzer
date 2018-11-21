#include "U8glib.h"
#include <SoftwareSerial.h> //import software serial library 
#include <Array.h> //import array library for max 

#define MAIN_LOOP_MILLIS_DELAY 100 //in MS
#define mq3_analogPin A7 // connected to the output pin of MQ3
#define NANO_SERIAL_RX_PIN 10 //initialize pin 10 and 11 for softwareSerial
#define NANO_SERIAL_TX_PIN 11

SoftwareSerial nanoSerial(NANO_SERIAL_RX_PIN, NANO_SERIAL_TX_PIN); // RX, TX

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI

int mq3_value = 0;
long timeIn, timeOut;

int buttonState;
boolean buttonActive = false;
boolean longPressActive = false;
boolean btMode = true;

long buttonTimer = 0; //record time when button is first pressed

char tmp_string[6];
int yPos = 0;
int oled_height = 32;
int oled_width = 128;

const int arraySize = 200;
int i;
float sensorValue;
float BloodBAC;

unsigned long currentMillis;
unsigned long previousMillis = 0;
int oneSecondsPeriod = 1000; // interval of oneSecondsPeriod for 1s
int tenthOfSecondsPeriod = 0; // interval of tenthOfSecondsPeriod for 0.1s
char inByte;

int j = 0;
const byte size = 200;
float rawArray[size];
Array<float> array = Array<float>(rawArray, size);

boolean newData = false;
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

String bac;
String inputString = "  BAC";
/*****************************************************************/

int SecondsDisplay = 10; // time for seconds counter
const int buttonPin = 2; //input pin for push button
long longPressTime = 3000; // length of time in ms button held to activate long press function

/*****************************************************************/
void setup(void) {
  // assign color value
  pinMode(buttonPin, INPUT);
  u8g.setColorIndex(1);     // turn on pixel, 0-255
  Serial.begin(9600);
  nanoSerial.begin(9600); // open bluetooth serial at 9600 bps
  while (!nanoSerial.available()) //wait for wifi initiated signal from esp32 before going into main loop
  {
    waitForWifiModeDisplay(); //display waiting for wifi
  }
  connectedDisplay();
  delay(2000);
}
/*****************************************************************/
void loop(void) {
  void(* resetFunc) (void) = 0;//declare reset function at address
  homeDisplay();
  if (digitalRead(buttonPin) == HIGH)
  {
    if (buttonActive == false)
    {
      buttonActive = true;
      buttonTimer = millis(); //record button when first pressed
    }

    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) //activates wifi
    {
      longPressActive = true;

      Serial.println("WIFI LONG Button Pressed!");
      Serial.print("<");
      Serial.print("a");
      Serial.print(">");
      Serial.print("\n");

      nanoSerial.print("<");
      nanoSerial.print("a");
      nanoSerial.print(">");

      pleaseBlowCountDown();

      Serial.print(array.getMax());
      Serial.print("\n"); //newline for termination at the receiving end

      nanoSerial.print(array.getMax()); //get max value from array and send to esp32
      nanoSerial.print("\n"); //newline for termination at the receiving end

      buttonActive = false; //stop button from looping
      bacDisplay();
      delay(5000);
      resetFunc(); //hard reset arduino
    }

  } else
  {
    if (buttonActive == true)
    {
      if (longPressActive == true)
      {
        longPressActive = false;
      } else //activates BT
      {
        Serial.println("BT SHORT Button Pressed!");
        Serial.print("<");
        Serial.print("b");
        Serial.print(">");
        Serial.print("\n");

        nanoSerial.print("<");
        nanoSerial.print("b");
        nanoSerial.print(">");

        bluetoothModeDisplay();
        do
        {
          bluetoothMode();
          if (digitalRead(buttonPin) == HIGH) //press button again to stop BT mode
          {
            btMode = false;
            resetFunc(); //hard reset nano
          }
        } while (btMode == true);
        buttonActive = false;
      }
    }
  }
  btMode = true; //reset btMode
}
/*****************************************************************/
void calculateBAC(void)
{
  float VRL;
  float ratio;
  float RS_Air; //  Get the value of RS via in air with alcohol content
  float R0 = 0.22;  // Get the value of R0 via in Alcohol
  float BrBAC;

  VRL = sensorValue / 1024 * 5.0; // converting analog data into digital in voltage
  RS_Air = (5.0 - VRL) / VRL; // omit RL

  /*Getting value of R0 found above to be used in formula to get RS/R0 value*/
  ratio = RS_Air / R0; // ratio = RS/R0
  BrBAC = 0.5346 * pow(ratio, -0.67) ; //BrBAC in mg/L (need to work on this line i suppose)
  BloodBAC = BrBAC * 2100 / 46 * 0.0046 ; // convert BrBAC to Blood BAC in %BAC

} // end: calculateBAC(void)

/*****************************************************************/
void blankScreen(void)
{
  u8g.firstPage();
  do {
    // Display blank page
  } while ( u8g.nextPage());
} // end: blankScreen(void)

/*****************************************************************/
void pleaseWaitCountDown(void) {
  while (SecondsDisplay != 0) {
    currentMillis = previousMillis = millis();
    while ((currentMillis - previousMillis) < oneSecondsPeriod) {
      // Display 'Please wait~' and timer
      u8g.firstPage();
      do {
        u8g.setFont(u8g_font_helvB08);
        u8g.drawStr(0, 10, "Please Wait~");
        itoa(SecondsDisplay, tmp_string, 10);
        u8g.drawStr(0, 28, tmp_string);
      } while ( u8g.nextPage());
      currentMillis = millis();
    } // end: while ((currentMillis - previousMillis) < oneSecondsPeriod)
    SecondsDisplay--;
  } // end: while (i != 0)
  if ( SecondsDisplay == 0 ) {
    blankScreen();
    SecondsDisplay = 10; //reset back to set value
    delay(500);
  }
} // end: pleaseWaitCountDown(void)
/*****************************************************************/
void pleaseBlowCountDown(void) {
  while (SecondsDisplay != 0) {
    currentMillis = previousMillis = millis();
    while ((currentMillis - previousMillis) < oneSecondsPeriod) {
      // Get BAC value
      if ((currentMillis - previousMillis) > tenthOfSecondsPeriod)
      {
        //unsigned long time1 = millis();
        sensorValue = analogRead(mq3_analogPin); // get MQ3 analog value
        calculateBAC(); // Calculate Blood BAC value from MQ3 analog value
        rawArray[j] = BloodBAC;
        Serial.print(BloodBAC);
        Serial.print("\n"); //newline for termination at the receiving end

        //Serial.println(currentMillis - previousMillis);
        tenthOfSecondsPeriod += 100 ;
        j++;
      } // end: if ((currentMillis - previousMillis) < tenthOfSecondsPeriod)
      // Display 'Please Blow~' and timer
      u8g.firstPage();
      do {
        u8g.setFont(u8g_font_helvB08);
        u8g.drawStr(0, 10, "Please Blow~");
        itoa(SecondsDisplay, tmp_string, 10);
        u8g.drawStr(0, 28, tmp_string);
      } while ( u8g.nextPage());

      currentMillis = millis();

    } // end: while ((currentMillis - previousMillis) < oneSecondsPeriod)
    SecondsDisplay--;
    tenthOfSecondsPeriod = 0;
  } // end: while (i != 0)
  if ( SecondsDisplay == 0 ) {
    blankScreen();
    SecondsDisplay = 10;
    //delay(500);
  }
} // end: pleaseBlowCountDown(void)
/*****************************************************************/
void bluetoothMode(void) {

  timeIn = millis();
  mq3_value = analogRead(mq3_analogPin); //read analog pin A0 with SP/36

  Serial.print(mq3_value);
  Serial.print("\n");

  nanoSerial.print(mq3_value);
  nanoSerial.print("\n");

  delay(50);
  timeOut = millis();

  delay(MAIN_LOOP_MILLIS_DELAY - (timeOut - timeIn)); //to ensure proper loop delay constraints

} // end: bluetoothMode(void)
/*****************************************************************/
void homeDisplay(void) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvB08);
    u8g.drawStr(0, 10, "Short Press for BT");
    u8g.drawStr(0, 28, "Hold for WiFi");
  } while ( u8g.nextPage());

}
/*****************************************************************/
void bluetoothModeDisplay(void) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvB08);
    u8g.drawStr(0, 10, "Please Blow~ BT MODE");
    u8g.drawStr(0, 28, "Press Again to Stop");
  } while ( u8g.nextPage());

}
/*****************************************************************/
void waitForWifiModeDisplay(void) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvB08);
    u8g.drawStr(0, 10, "Hello, checking WiFi");
    u8g.drawStr(0, 28, "Please wait... ");
  } while ( u8g.nextPage());

}
/*****************************************************************/
void bacDisplay(void) {
  bac = String(array.getMax()) + inputString;
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvB08);
    u8g.setScale2x2();
    u8g.setPrintPos(0, 10);
    u8g.print(bac);
    u8g.undoScale();
  } while ( u8g.nextPage());

}
/*****************************************************************/
void connectedDisplay(void) {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvB08);
    u8g.setScale2x2();
    u8g.drawStr(0, 10, "CONNECTED");
    u8g.undoScale();
  } while ( u8g.nextPage());

}
