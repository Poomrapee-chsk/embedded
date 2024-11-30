// Author: Mudassar Tamboli
#include "OV7670.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <driver/ledc.h>
#include <PubSubClient.h>
#include "DHT.h"

// Humidity and temperature sensor
#define DHTPIN 23
#define DHTTYPE DHT11

// Netpie
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "1428f47f-fe35-4010-8537-ac411a92ef51";
const char* mqtt_username = "PS42qjzn1vjEehJUAPMjqW1jCx65F5te";
const char* mqtt_password = "bGBwRE8JmkC4eBJfcDhn3PSUypjNzSmq";

const char *ssid1 = "wePresent1738";
const char *password1 = "itsgoogle!";

const int SIOD = 21; // SDA
const int SIOC = 22; // SCL
const int VSYNC = 34;
const int HREF = 35;
const int XCLK = 32;
const int PCLK = 33;

const int D0 = 27;
const int D1 = 26;
const int D2 = 25;
const int D3 = 15;
const int D4 = 14;
const int D5 = 13;
const int D6 = 12;
const int D7 = 4;

/* MQTT and Netpie*/
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
char msg[100];

// Reconnect to MQTT server
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}

// Initialize WiFi station
void initWifiStation() {

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid1, password1);    
    Serial.print("\nConnecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
       delay(5000);        
       Serial.print(".");
    }
    Serial.println(String("\nConnected to the WiFi network (") + ssid1 + ")" );

    Serial.print("\nStation IP address: ");
    Serial.println(WiFi.localIP()); 

}

/* Capture frame and send to computer*/
// Headers 
unsigned char start_flag = 0xAA;
unsigned char end_flag = 0xFF;
unsigned char ip_flag = 0x11;

OV7670 *camera;

void captureAndSend(void *pvParameters) {
    Serial.println("Connecting to PC");
    const char* serverIP = "192.168.153.153"; // Replace with your PC's IP address
    const uint16_t serverPort = 81; // Replace with your chosen port
    int blk_count = 0;
    camera->startBlock = 1;
    camera->endBlock = I2SCamera::blockSlice;

    if (espClient.connect(serverIP, serverPort)) {
        camera->oneFrame(); // Capture a frame from the camera
        
        // Write header
        uint8_t header[2] = {0xAA, 0x55};  // Example header
        espClient.write(header, 2);
        blk_count = camera->yres/I2SCamera::blockSlice;//30, 60, 120
        for (int i=0; i<blk_count; i++) {

          if (i == 0) {
              camera->startBlock = 1;
              camera->endBlock = I2SCamera::blockSlice;
              espClient.write(&start_flag, 1);
          }

          if (i == blk_count-1) {
            espClient.write(&end_flag, 1);
          }

          camera->oneFrame();
          espClient.write(camera->frame, camera->xres * I2SCamera::blockSlice * 2);
          camera->startBlock += I2SCamera::blockSlice;
          camera->endBlock   += I2SCamera::blockSlice;
      }
        // Write frame data to client
        espClient.stop(); // Close connection after sending
        Serial.println("Frame sent to PC");
    } else {
        Serial.println("Connection to PC failed");
    }
    delay(2000);
}

// Handle routine for humidity and temperature
void sensors(void *pvParameters) {
    delay(2000);
    
    double h = dht.readHumidity();
    double t = dht.readTemperature();

    if (!client.connected()) {
        reconnect();
    }

    // client.loop();
    String data = String("Humidity: ") + h + " %, Temperature: " + t + " C" + "\n";
    Serial.print(data);
    data.toCharArray(msg, data.length() + 1);
    client.publish("@shadow/data/update", msg);
}

// Setup function
void setup() {
    Serial.begin(115200);
    initWifiStation();
    Serial.println("Initializing camera");
    camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
    Serial.println("Camera initialized");
    client.setServer(mqtt_server, mqtt_port);
    dht.begin();

    // // Create a task for captureAndSend to run on core 1
    // xTaskCreatePinnedToCore(
    //     captureAndSend,   // Function to be called
    //     "CaptureAndSend", // Name of the task
    //     8192,             // Stack size (bytes)
    //     NULL,             // Parameter to pass
    //     1,                // Task priority
    //     NULL,             // Task handle
    //     1                 // Core to run the task on (1 = CPU1)
    // );

    // // Create a task for anotherFunction to run on core 0
    // xTaskCreatePinnedToCore(
    //     sensors,  // Function to be called
    //     "HumidityAndTemperature",// Name of the task
    //     4096,             // Stack size (bytes)
    //     NULL,             // Parameter to pass
    //     1,                // Task priority
    //     NULL,             // Task handle
    //     0                 // Core to run the task on (0 = CPU0)
    // );

    dht.begin();
}

void loop() {
    captureAndSend(NULL);
}