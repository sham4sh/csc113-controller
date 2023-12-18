// example code based on: https://iotdesignpro.com/projects/how-to-connect-esp32-mqtt-broker
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ezButton.h>
//#include <driver/gpio.h>

using namespace std;


// Update these with values suitable for your network.
const char* ssid = "iPhone";
const char* password = "SHAMASH1";
const char* mqtt_server = "test.mosquitto.org";

#define mqtt_port 1883

#define MQTT_VELOCITY "csc113/controller/vel"
#define MQTT_STEER "csc113/controller/st"

#define MQTT_DISPLAY "csc113/controller/display"

#define MQTT_PROX "csc113/controller/prox"

#define MQTT_CTRL "csc113/controller/ctrl"

#define VRX_PIN  39 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  36 // ESP32 pin GPIO39 (ADC0) connected to VRY pin
#define SW_PIN   33 // ESP32 pin GPIO33 connected to SW  pin

#define BT_PIN   GPIO_NUM_25

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

ezButton button(SW_PIN);

int valueX = 0; // to store the X-axis value
int valueY = 0; // to store the Y-axis value
int bValue = 0; // To store value of the button

int sentDisplays = 0;
double prox = 0;
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
    client.publish(MQTT_CTRL, 0);
    
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-Shamash-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()) )
    {
      Serial.println("connected");
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
    messageTemp += (char)payload[i];
  }

  messageDoub = stod(messageTemp.c_str());

  prox = messageDoub;
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  Serial.setTimeout(5000);

  print_wakeup_reason();
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(BT_PIN, HIGH);

  //Serial.println("Going to sleep now");
  //delay(1000);
  //esp_deep_sleep_start();

  button.setDebounceTime(50); // set debounce time to 50 milliseconds
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

  client.publish(MQTT_STEER, to_string(valueX).c_str());
  client.publish(MQTT_VELOCITY, to_string(valueY).c_str());

  //client.publish(MQTT_PROX, to_string(proximity).c_str());

  textDisplay(lineDisplay(0, 0, 0, ((valueX / 100) + 21)));

  client.publish(MQTT_DISPLAY, (display[0] + display[1] + display[2] + display[3]).c_str());

  button.loop(); // MUST call the loop() function first

  // Read the button value
  bValue = button.getState();

  if (button.isPressed()) {
    client.publish(MQTT_CTRL, 0);
    client.publish(MQTT_VELOCITY, 0);
    Serial.println("Going to sleep now");
    delay(1000);
    esp_deep_sleep_start();
  }else{
    client.publish(MQTT_CTRL, "1");
  }
 }
