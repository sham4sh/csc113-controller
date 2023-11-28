// example code based on: https://iotdesignpro.com/projects/how-to-connect-esp32-mqtt-broker
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


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

#define VRX_PIN  36 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
#define VRY_PIN  39 // ESP32 pin GPIO39 (ADC0) connected to VRY pin

String valueX = ""; // to store the X-axis value
String valueY = ""; // to store the Y-axis value

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
      client.subscribe(MQTT_SERIAL_SUBSCRIBE_CH);
      client.subscribe(MQTT_VELOCITY);
      client.subscribe(MQTT_STEER);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
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
void loop() {
  client.loop();


  valueX = std::to_string(analogRead(VRX_PIN));
  valueY = analogRead(VRY_PIN);


  if (Serial.available())
  {
    String str = Serial.readStringUntil('\n');
    publishSerialData(str.c_str());

    client.publish(MQTT_STEER, valueX.c_str());
    client.publish(MQTT_VELOCITY, valueY.c_str());
  }

   // read X and Y analog values
  

  // print data to 
  /**
  Serial.print("x = ");
  Serial.print(valueX);
  Serial.print(", y = ");
  Serial.println(valueY); 
  */
 }
