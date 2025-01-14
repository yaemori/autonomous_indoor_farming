#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>

// Wi-Fi and Firebase Configuration
#define WIFI_SSID "WIFI NAME"                                      // WiFi SSID
#define WIFI_PASSWORD "PASSWORD"                               // WiFi Password
#define FIREBASE_HOST "FIRABSE URL" // Firebase URL
#define FIREBASE_AUTH "SECRET/TOKEN" // Firebase Secret or Token

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// SoftwareSerial Configuration
#define SOFT_RX D5 // NodeMCU RX pin (connected to Arduino Mega TX pin)
#define SOFT_TX D6 // NodeMCU TX pin (connected to Arduino Mega RX pin)
SoftwareSerial nodeSerial(SOFT_RX, SOFT_TX); // RX, TX

void setup() {
  // Initialize SoftwareSerial and Serial Monitor
  nodeSerial.begin(9600); // Same baud rate as Arduino Mega
  Serial.begin(9600);   // NodeMCU USB Serial Monitor
  Serial.println("Initializing NodeMCU with SoftwareSerial...");

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Configure Firebase
  firebaseConfig.database_url = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Initialize Firebase
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("Connected to Firebase!");
}

void loop() {
  // Check if data is available from Arduino Mega
  if (nodeSerial.available()) {
    String payload = nodeSerial.readStringUntil('\n'); // Use '\n' as the payload terminator
    payload.trim(); // Remove any leading/trailing whitespace

    if (payload.startsWith("*")) { // Check for start marker
      payload.remove(0, 1); // Remove start marker

      // Log the received payload
      Serial.println("\nReceived Payload: " + payload);

      // Parse payload
      String dateTime = getValue(payload, '&', 0);
      String tds = getValue(payload, '&', 1);
      String ph = getValue(payload, '&', 2);
      String waterTemp = getValue(payload, '&', 3);
      String roomTemp = getValue(payload, '&', 4);
      String humidity = getValue(payload, '&', 5);
      String distance = getValue(payload, '&', 6);
      String greenFrequency = getValue(payload, '&', 7);

      // Log parsed values
      Serial.println("Parsed Sensor Values:");
      Serial.println("DateTime: " + dateTime);
      Serial.println("TDS: " + tds + " µS/cm");
      Serial.println("PH: " + ph);
      Serial.println("Water Temperature: " + waterTemp + "°C");
      Serial.println("Room Temperature: " + roomTemp + "°C");
      Serial.println("Humidity: " + humidity + "%");
      Serial.println("Distance: " + distance + " cm");
      Serial.println("Green Frequency: " + greenFrequency);

      // Prepare formatted variables for Firebase
      String fireTDS = tds + " µS/cm";
      String firePH = ph;
      String fireWaterTemp = waterTemp + "°C";
      String fireRoomTemp = roomTemp + "°C";
      String fireHumidity = humidity + "%";
      String fireDistance = distance + " cm";
      String fireGreenFrequency = greenFrequency;

      // Push data to Firebase
      Serial.println("Sending data to Firebase...");

      if (Firebase.pushString(firebaseData, "/SensorData/TDS", fireTDS)) {
        Serial.println("TDS sent successfully.");
      } else {
        Serial.println("Failed to send TDS: " + firebaseData.errorReason());
      }

      if (Firebase.pushString(firebaseData, "/SensorData/PH", firePH)) {
        Serial.println("PH sent successfully.");
      } else {
        Serial.println("Failed to send PH: " + firebaseData.errorReason());
      }

      if (Firebase.pushString(firebaseData, "/SensorData/WaterTemperature", fireWaterTemp)) {
        Serial.println("Water Temperature sent successfully.");
      } else {
        Serial.println("Failed to send Water Temperature: " + firebaseData.errorReason());
      }

      if (Firebase.pushString(firebaseData, "/SensorData/RoomTemperature", fireRoomTemp)) {
        Serial.println("Room Temperature sent successfully.");
      } else {
        Serial.println("Failed to send Room Temperature: " + firebaseData.errorReason());
      }

      if (Firebase.pushString(firebaseData, "/SensorData/Humidity", fireHumidity)) {
        Serial.println("Humidity sent successfully.");
      } else {
        Serial.println("Failed to send Humidity: " + firebaseData.errorReason());
      }

      if (Firebase.pushString(firebaseData, "/SensorData/Distance", fireDistance)) {
        Serial.println("Distance sent successfully.");
      } else {
        Serial.println("Failed to send Distance: " + firebaseData.errorReason());
      }

      if (Firebase.pushString(firebaseData, "/SensorData/GreenFrequency", fireGreenFrequency)) {
        Serial.println("Green Frequency sent successfully.");
      } else {
        Serial.println("Failed to send Green Frequency: " + firebaseData.errorReason());
      }

      Serial.println("All data sent to Firebase.");
    } else {
      Serial.println("Invalid payload format received.");
    }
  }

  delay(500); // Short delay for stability
}

// Function to parse payload values
String getValue(String data, char separator, int index) {
  int startIndex = 0, endIndex = -1, count = 0;

  while (count <= index) {
    startIndex = endIndex + 1;
    endIndex = data.indexOf(separator, startIndex);
    if (endIndex == -1) endIndex = data.length();
    if (count == index) return data.substring(startIndex, endIndex);
    count++;
  }
  return "";
}