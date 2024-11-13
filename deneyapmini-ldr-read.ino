#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Firebase Helper Files
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Firebase project credentials
#define API_KEY "YOUR_API_KEY"
#define DATABASE_URL "YOUR_URL"  // Make sure URL format is correct

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void setup() {
  // Start Serial communication and Wi-Fi connection
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure Firebase settings
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up to Firebase (only needed if user registration is required)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Signup Successful");
    signupOK = true;
  } else {
    Serial.printf("Signup Error: %s\n", config.signer.signupError.message.c_str());
  }

  // Set the token status callback
  config.token_status_callback = tokenStatusCallback;

  // Initialize Firebase and reconnect Wi-Fi if disconnected
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set LDR sensor pin
  pinMode(A0, INPUT);
}

void loop() {
  // LDR sensör değerini okuyun
  int ldrValue = analogRead(A0);
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  // Firebase'e her 15 saniyede bir veri gönderin
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // LDR değerini Firebase'e gönderin
    if (Firebase.RTDB.setInt(&fbdo, "sensor/ldr", ldrValue)) {
      Serial.println("LDR value sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.printf("Failed to send LDR value. Reason: %s\n", fbdo.errorReason().c_str());
    }
  }

  delay(100);  // Seri çıkışı aşırı yüklememek için kısa bir gecikme
}