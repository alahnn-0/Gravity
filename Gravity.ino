#include <WiFi.h>
#include <Wire.h>
#include <MPU6050.h>
#include <WebSocketsServer.h>

const char* ssid = "RABBITSQUARE_4G";
const char* password = "rsq@9846";

MPU6050 mpu;
WebSocketsServer webSocket(81);

float roll = 0;
float pitch = 0;
float yaw = 0;

unsigned long lastSend = 0;

void setup() {
  Serial.begin(9600);

  // I2C
  Wire.begin(21, 22);

  // MPU6050
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  Serial.println("MPU6050 connected");

  // WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // WebSocket
  webSocket.begin();
  Serial.println("WebSocket started on port 81");
}

void loop() {

  webSocket.loop();

  int16_t ax, ay, az;
  int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Convert raw values
  float axf = (float)ax;
  float ayf = (float)ay;
  float azf = (float)az;

  // Roll
  roll = atan2(ayf, azf) * 180.0 / PI;

  // Pitch
  pitch = atan2(
            -axf,
            sqrt(ayf * ayf + azf * azf)
          ) * 180.0 / PI;

  // Yaw unavailable from accelerometer alone
  yaw = 0;

  // Send every 50ms
  if (millis() - lastSend > 50) {

    String json =
      "{\"roll\":" + String(roll, 2) +
      ",\"pitch\":" + String(pitch, 2) +
      ",\"yaw\":" + String(yaw, 2) +
      "}";

    webSocket.broadcastTXT(json);

    Serial.println(json);
    delay(2000);
    lastSend = millis();
  }
}