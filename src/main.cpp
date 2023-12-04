// example code based on: https://iotdesignpro.com/projects/how-to-connect-esp32-mqtt-broker
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

using namespace std;


// Update these with values suitable for your network.
const char* ssid = "iPhone";
const char* password = "SHAMASH1";
const char* mqtt_server = "test.mosquitto.org";
#define mqtt_port 1883
//#define MQTT_USER "james"
//#define MQTT_PASSWORD "wesr431bghj"
#define MQTT_SERIAL_PUBLISH_CH "csc113/serialdata/tx"
#define MQTT_SERIAL_SUBSCRIBE_CH "csc113/serialdata/tx"

#define MQTT_VELOCITY "csc113/controller/vel"
#define MQTT_STEER "csc113/controller/st"

#define MQTT_DISPLAY "csc113/controller/display"

#define VRX_PIN  39 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  36 // ESP32 pin GPIO39 (ADC0) connected to VRY pin

int valueX = 0; // to store the X-axis value
int valueY = 0; // to store the Y-axis value

int sentDisplays = 0;
String display[4] = {"", "", "", ""};

WiFiClient wifiClient;

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network // Test commit
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-Shamash-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) 
    if (client.connect(clientId.c_str()) )
    {
      Serial.println("connected");
      // subscribe
      //client.subscribe(MQTT_SERIAL_SUBSCRIBE_CH);
      //client.subscribe(MQTT_VELOCITY);
      //client.subscribe(MQTT_STEER);
      //client.subscribe(MQTT_DISPLAY);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
    
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  double messageDoub;
  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }

  messageDoub = stod(messageTemp.c_str());
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  Serial.setTimeout(5000);
}

void publishSerialData(const char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  Serial.println(String("publishing: ") + serialData);

  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

/** Normalize joystick to range (-2048, 2048) and add buffer to zero */
void normalizeStick(){
  valueX *= -1;
  valueY *= - 1;

  valueX += 2048;
  valueY += 2048;

  if(abs(valueX) < 300){
    valueX = 0;
  }

  if(abs(valueY) < 300){
    valueY = 0;
  }
}

String lineDisplay(int veloc, int leftProx, int rightProx, int pos){
  String retStr = "|-----------------------------------------|\n";
  retStr[pos - leftProx] = '}';
  retStr[pos + rightProx] = '{';
  retStr[pos] = '8';
  return retStr;
}

void textDisplay(String curLine){
  display[0] = "|-----------------------------------------|\n";
  
  display[1] = display[2];
  display[2] = curLine;

  display[3] = "|-----------------------------------------|\n";
}

void loop() {
  client.loop();


  valueX = analogRead(VRX_PIN);
  valueY = analogRead(VRY_PIN);

  normalizeStick();

  if (Serial.available())
  {
    String str = Serial.readStringUntil('\n');
    publishSerialData(str.c_str());

    //client.publish(MQTT_STEER, to_string(valueX).c_str());
    //client.publish(MQTT_VELOCITY, to_string(valueY).c_str());
  }
  /**
  if(abs(valueX) < 300){
    valueX = 0;
  }

  if(abs(valueY - 2048) < 300){
    valueY = 0;
  }
  */
  client.publish(MQTT_STEER, to_string(valueX).c_str());
  client.publish(MQTT_VELOCITY, to_string(valueY).c_str());

   // read X and Y analog values


  textDisplay(lineDisplay(0, 0, 0, ((valueX / 100) + 21)));

  client.publish(MQTT_DISPLAY, (display[0] + display[1] + display[2] + display[3]).c_str());
  /**
  Serial.print(display[0]);
  Serial.print(display[1]);
  Serial.print(display[2]);
  Serial.print(display[3]);
  */
  // print data to 
  /**
  Serial.print("x = ");
  Serial.print(valueX);
  Serial.print(", y = ");
  Serial.println(valueY); 
  */
 }
