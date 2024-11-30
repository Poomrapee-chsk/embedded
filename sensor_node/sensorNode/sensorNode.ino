#include <WiFi.h>
#include <PubSubClient.h>
#include <HCSR04.h>

//Wifi connection
#define ssid ""
#define password ""

//Netpie MSQTT connection
#define mqtt_server "broker.netpie.io"
#define mqtt_port 1883
//esp32sensornode
#define mqtt_Client ""
#define mqtt_username ""
#define mqtt_password ""
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

//IR sensor
#define detectingDistance 100
UltraSonicDistanceSensor distanceSensor(25, 26);
u_long lastIRevent = 0;

//SoundSensor
#define detectingSoundLevel 100
#define soundPin 36
u_long lastSoundevent = 0;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void checkIR() {
  u_long now = millis();

  if (now - lastIRevent > 5000 && distanceSensor.measureDistanceCm() < detectingDistance) {
    lastIRevent = now;
    Serial.println(distanceSensor.measureDistanceCm());
    client.publish("@msg/IR", "true");
    Serial.println("IR Triggered");
  }
  
}

void checkSound() {
  u_long now = millis();

  int sound_level = analogRead(soundPin);
  if (now - lastSoundevent > 5000 && sound_level >= detectingSoundLevel) {
    lastSoundevent = now;
    client.publish("@msg/light", "sound");
    Serial.println("Sound Triggered");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // long now = millis();
  // if (now - lastMsg > 2000) {
  //   lastMsg = now;
  //   client.publish("@msg/test", "Hello NETPIE2020");
  //   Serial.println("Hello NETPIE2020");
  // }
  
  checkIR();
  checkSound();
  delay(1);
}