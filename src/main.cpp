#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "synxhronous";                                   // wifi name
const char *password = "";                                          // wifi pass
const char *mqtt_server = "ec2-**********.compute-1.amazonaws.com"; // fill your mqtt server

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
#define sensorSoilMoisture A0
#define relayModule D1
char msg[MSG_BUFFER_SIZE];
int value = 0;
int nilaiSensor;

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  if (String(topic) == "SPC/Siram")
  {
    if (messageTemp == "siramtanaman")
    {
      digitalWrite(relayModule, LOW);
      delay(3500);
      digitalWrite(relayModule, HIGH);
      client.publish("SPC/Notif", "Berhasil Disiram");
    }
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      client.subscribe("SPC/Siram");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  pinMode(relayModule, OUTPUT);
  digitalWrite(relayModule, HIGH);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
    nilaiSensor = analogRead(sensorSoilMoisture);
    nilaiSensor = map(nilaiSensor, 1023, 165, 0, 100);
    static char soilMoisture[7];
    dtostrf(nilaiSensor, 4, 2, soilMoisture);
    Serial.print("soilMoisture: ");
    Serial.println(soilMoisture);
    client.publish("SPC/Kelembaban", soilMoisture);
  }
}