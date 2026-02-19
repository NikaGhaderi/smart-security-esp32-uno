#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// --- 1. Network & Telegram Credentials ---
const char* ssid = "****";
const char* password = "****";
const String BOT_TOKEN = "****";
const String CHAT_ID = "****";
const String CF_WORKER = "****";

// --- 2. Camera Pin Definitions for AI-Thinker ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


// --- Custom Project Pins ---
const int pirPin = 13;      // The PIR Sensor
const int buzzerPin = 14;   // The local Siren
const int onboardLED = 33;  // The tiny red internal LED
const int normalLED = 4;    // External LED AND the massive white Flash LED
const int alertLED = 12;    // External Alert LED
const int commPin = 2;      // Armed/Disarmed Signal from Arduino Uno Pin 10

// --- 4. System States ---
bool hasCapturedPhoto = false; 

void setup() {
  Serial.begin(115200);
  
  // Set up the Input/Output pins
  pinMode(pirPin, INPUT_PULLDOWN); 
  pinMode(buzzerPin, OUTPUT);
  pinMode(onboardLED, OUTPUT);
  pinMode(normalLED, OUTPUT);
  pinMode(alertLED, OUTPUT);
  pinMode(commPin, INPUT_PULLDOWN); 
  
  digitalWrite(buzzerPin, LOW);
  digitalWrite(onboardLED, HIGH);
  digitalWrite(normalLED, HIGH);
  digitalWrite(alertLED, LOW);
  
  // --- Connect to Wi-Fi ---
  delay(5000);
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // --- Configure Camera ---
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Resolution
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  Serial.println("System Ready! Waiting for Arduino Uno to ARM the system...");
}

void loop() {
  // Check if the Arduino Uno keypad has ARMED the system
  int systemArmed = digitalRead(commPin);
  
  if (systemArmed == HIGH) {
    int motionState = digitalRead(pirPin);
    
    if (motionState == HIGH) {
      // 1. MOTION DETECTED: Immediately trigger the physical deterrents!
      digitalWrite(buzzerPin, HIGH); 
      digitalWrite(onboardLED, LOW); 
      digitalWrite(normalLED, LOW);
      digitalWrite(alertLED, HIGH);
      
      // 2. Take the photo ONLY if we haven't already taken one for this movement
      if (!hasCapturedPhoto) {
        Serial.println("INTRUDER DETECTED! Sounding alarm and snapping photo...");
        captureAndSendPhoto();
        hasCapturedPhoto = true; // Lock the camera out until motion stops
      }
      
    } else {
      // System is Armed, but no motion. Keep alarm quiet.
      digitalWrite(buzzerPin, LOW); 
      digitalWrite(onboardLED, HIGH); 
      digitalWrite(normalLED, HIGH);
      digitalWrite(alertLED, LOW);
      
      // Reset the photo flag so it is ready for the next intruder
      hasCapturedPhoto = false; 
    }
  } else {
    digitalWrite(buzzerPin, LOW); 
    digitalWrite(onboardLED, HIGH); 
    digitalWrite(normalLED, HIGH);
    digitalWrite(alertLED, LOW);
    
    hasCapturedPhoto = false; 
  }
  
  delay(100); 
}

// --- Telegram Photo Transmission Function ---
void captureAndSendPhoto() {
  camera_fb_t * fb = NULL;
  
  // Flush the current frame to get a fresh image of the movement
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); 
  fb = esp_camera_fb_get();  
  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  
  // Connect to the Cloudflare Worker instead of Telegram
  Serial.println("Connecting to Cloudflare Bridge...");
  
  if (client.connect(CF_WORKER.c_str(), 443)) {
    Serial.println("Connected! Sending image...");
    
    String head = "--SecurityBoundary\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--SecurityBoundary\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"intruder.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--SecurityBoundary--\r\n";
    
    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    // Send the HTTP POST headers to the Worker
    client.println("POST /bot" + BOT_TOKEN + "/sendPhoto HTTP/1.1");
    client.println("Host: " + CF_WORKER); // Update the Host header!
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=SecurityBoundary");
    client.println();
    
    // Send the body head
    client.print(head);

    // Send the image data in small chunks to prevent memory crashes
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n = n + 1024) {
      if (n + 1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      } else if (fbLen % 1024 > 0) {
        size_t remainder = fbLen % 1024;
        client.write(fbBuf, remainder);
      }
    }
    
    // Send the body tail
    client.print(tail);
    
    // Wait for the response to clear the buffer
    while (client.connected() && client.available() == 0) {
      delay(10);
    }
    while (client.available()) {
      client.read(); 
    }
    
    client.stop();
    Serial.println("Photo safely routed through Cloudflare to Telegram!");
  } else {
    Serial.println("Failed to connect to Cloudflare server.");
  }
  
  // Free up the camera memory
  esp_camera_fb_return(fb); 
}