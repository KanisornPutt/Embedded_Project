#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <SoftwareSerial.h>
EspSoftwareSerial::UART StmSerial;
int ddht = 14; //D5
int relay = 12; //D6
String autopump = "off";
float AP = 0.0f;
String data = "";
#define DHTTYPE DHT22
#define D0 16
#define LED_PIN D0
#define RELAY_PIN 12
#define BUTTON_PIN 0

DHT dht(ddht, DHTTYPE);

// WiFi Parameters
const char* ssid = "LeGalaxy";
const char* password = "LeMaxGyarados";

// NetPie Parameters
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "a9738c8a-4c31-41d3-8ef3-e98b47c291ad";
const char* mqtt_username = "ANZt9V5EhmDaWaLQyW3B5tWuJNwvCfns";
const char* mqtt_password = "3RQ8nJFGtCoyLyxbTQLMeaHPsGGHqEr2";

bool ledState = false; // Variable to store the LED state
bool lastButtonState = HIGH; // Variable to store the last button state
unsigned long lastDebounceTime = 0; // Debounce timer
unsigned long debounceDelay = 5;

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100],msg2[100];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/#");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  tpc = getMsg(topic, message);
}

void setStatus(String status) {
  if (status == "on") {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH); //relay เปิด
    autopump = "on";
    AP = 1.0f;
    Serial.println("ON");
    StmSerial.write("ON");
  }
  else if (status == "off") {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW); //relay ปิด

    autopump = "off";
    AP = 0.0f;
    Serial.println("OFF");
  }
}

void toggleStatus() {
  if (autopump == "on")
    setStatus("off");
  else 
    setStatus("on");
}

void ReadSTM() {
  if(StmSerial.available() > 0){
    data = "";
    uint8_t state=0;
    while(StmSerial.available()){
      char c = StmSerial.read();
      data += c;
    }
    toggleStatus();
  }
  // Serial.println(data);
}

void ReadButton() {
  int reading = digitalRead(BUTTON_PIN); // Read the state of the button

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed and is stable
    if (reading == LOW) { // Button is pressed
      toggleStatus();
    }
  }
  lastButtonState = reading;
}

void setup() {
  pinMode(relay, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(relay, LOW); // relay ปิด
  Serial.begin(115200);
  StmSerial.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, D7, D8, false, 100, 100);
  dht.begin();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  setStatus("off");
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // if (autopump == "on") {
  //   if (h > 80) {
  //     digitalWrite(relay, LOW); //เปิด
  //   } else {
  //     digitalWrite(relay, HIGH); //ปิด
  //   }
  // }

  ReadSTM();
  ReadButton();

  // Dust Density Sensor
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (3.3 / 1023.0);
  float dustDensity = (voltage - 0.0356) / 0.000377;
  dustDensity = max(dustDensity, 0.0f);
  Serial.println("===========================================================");
  Serial.println("--------------------- Dust Density ------------------------");
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print(", Voltage: ");
  Serial.print(voltage);
  Serial.print(", Dust Density: ");
  Serial.print(dustDensity);
  Serial.println(" mg/m3");
  // Serial.println("----------------------------------------------------------");

  float r = dustDensity; // r = Dust Percentage

  String data = "{\"data\":{\"Humidity\": " + String(h) + ",\"Temperature\": " + String(t) + ",\"Dust Density\": " + String(r) + ",\"Status\": " + String(AP) + "}}";
  String msgData =  String(t) + "," + String(h) + "," + String(r) + "," + String(AP); 
  Serial.println("--------------------- Data ---------------------------------");
  Serial.println(data);
  Serial.println(msgData);
  Serial.println("----------------------------------------------------------");
  msgData.toCharArray(msg2,(msgData.length() + 1));
  data.toCharArray(msg , (data.length() + 1));
  client.publish("@shadow/data/update", msg);
  client.publish("@msg/test",msg2);
  AP = 0.5f;
  client.loop();
  delay(1000);
}

String getMsg(String topic_, String message_) {     
  if (topic_ == "@msg/on") {
    setStatus(message_);                          //netpie["???"].publish("@msg/relay","on")
  }
  return autopump;
}