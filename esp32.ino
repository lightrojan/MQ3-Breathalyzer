#include <WiFi.h>
#include "BluetoothSerial.h"

#define RXD2 16 //for UART2
#define TXD2 17

HardwareSerial mySerial(2);
BluetoothSerial SerialBT;

int status = WL_IDLE_STATUS;
WiFiClient client;

boolean newData = false;
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

boolean newData2 = false;
const byte numChars2 = 32;
char receivedChars2[numChars2];   // an array to store the received data
char receivedChar;

String value;

boolean btMode = true;
boolean wifiMode = true;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

/*****************************************************************/
// EDIT: Change the 'ssid' and 'password' to match your network
//char ssid[] = "ASUS_TIONG_2.4G";  // wireless network name
//char password[] = "tiong67466333"; // wireless password
char ssid[] = "zc";  // wireless network name
char password[] = "lightrojan124"; // wireless password
// EDIT: 'Server' address to match your domain
char server[] = "18.191.104.220"; // This could also be 192.168.1.18/~me if you are running a server on your computer on a local network.
String yourdatacolumn = "value="; //change according to column name set in database
/*****************************************************************/
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //SerialBT.begin("ESP32"); // open bluetooth serial at flexible baud rate
  inputString.reserve(200);
  connectWifi();
  printWifiStatus();
}
void(* resetFunc) (void) = 0;//declare reset function at address
void loop() {

  mySerial.flush();
  recvWithStartEndMarkers();
  Serial.println(receivedChars2);

  if (String(receivedChars2) == "a") //start wifi
  {
    Serial.println("WIFI Calculating BAC.....: ");
    do {
      serialEvent();
      if (stringComplete)
      {
        Serial.print(inputString);
        postData();
        delay(5000);
        if (!mySerial.available())
        {
          wifiMode = false;
          Serial.println("break...");
          resetFunc(); //hard reset arduino
        }
        inputString = "";
        stringComplete = false;
      }
    } while (wifiMode == true);
  }
  else if (String(receivedChars2) == "b") //start BT
  {
    //WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    SerialBT.begin("ESP32"); // open bluetooth serial at flexible baud rate
    Serial.println("BT Received character: ");
    do
    {
      serialEvent();
      if (stringComplete)
      {
        Serial.print(inputString);
        bluetoothSend();
        // clear the string:

        if (!mySerial.available())
        {
          btMode = false;
          Serial.println("break...");
          resetFunc(); //hard reset esp32
        }
        inputString = "";
        stringComplete = false;

      }
    } while (btMode == true);
  }

  else
  {
    Serial.println("Waiting for Start Signal...");
  }

  btMode = true; //reset btMode
  wifiMode = true; //reset wifiMode
  delay(1000);
}


/*****************************************************************/
void connectWifi() {
  // Attempt to connect to wifi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    // Wait 10 seconds for connection
    delay(10000);
  }
}

/*****************************************************************/
void printWifiStatus() {
  // Print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  if (WiFi.status() == WL_CONNECTED) //send char to nano to start
  {
    mySerial.print("1");
    mySerial.print("\n");
  }
}

/*****************************************************************/
// This method makes a HTTP connection to the server and POSTs data
void postData() {
  // Combine yourdatacolumn header (yourdata=) with the data recorded from your arduino
  // (yourarduinodata) and package them into the String yourdata which is what will be
  // sent in your POST request

  value = yourdatacolumn + inputString;

  // If there's a successful connection, send the HTTP POST request
  if (client.connect(server, 80)) {
    Serial.println("connecting...");

    // EDIT: The POST 'URL' to the location of your insert_mysql.php on your web-host
    client.println("POST /mysql_connect.php HTTP/1.1");

    // EDIT: 'Host' to match your domain
    client.println("Host: 18.191.104.220");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");

    client.println(value.length());
    client.println();
    client.println(value);

    Serial.println(inputString);
    Serial.println();
    Serial.println(value.length());
    Serial.println();
    Serial.println(value);
  }

  else {
    // If you couldn't make a connection:
    Serial.println("Connection failed");
    Serial.println("Disconnecting.");
    client.stop();
  }
}

/*****************************************************************/
void recvWithEndMarker() {  //receiving BAC characters
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

/*****************************************************************/
void bluetoothSend() { //bluetooth send to pc

  SerialBT.print(inputString);
  delay(100);

}
/*****************************************************************/
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars2[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars2[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}
/*****************************************************************/
void serialEvent() {
  while (mySerial.available()) {
    // get the new byte:
    char inChar = (char)mySerial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
