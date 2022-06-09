#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//****************************
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
//*****************************
#define SS_PIN D8
#define RST_PIN D1
//****************************
const char* ssid = "MB11";
const char* password = "mustafa55";
const char* mqtt_server = "192.168.137.200";
//*********************************************
MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key;
//*********************************************
WiFiClient espClient;
PubSubClient client(espClient);
//********************************************
byte nuidPICC[4];
unsigned long lastMsg = 0;
unsigned long lastMsg2= 0;
unsigned long lastMsg3= 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
//*******************************
char msg2[MSG_BUFFER_SIZE];
String data;
char Buf[50];
//****************************
int value = 0;
int value2 = 0;
int yesil=D3;
int mavi=D2;
int kirmizi=D4;
int buzzer=D0;
int sensor = 9;
bool alarm_durumu=true;
bool servo_durum=true;
int kapi_durumu=0;
//******************
Servo servo;
WiFiServer server(80);
//**************************
void printHex(byte *buffer, byte bufferSize) 
{
  data.clear();
 for (byte i = 0; i < bufferSize; i++) 
 {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], HEX);   
   if(buffer[i] < 0x10)
   {      
     data += String("0");
   }
   data += String(buffer[i], HEX);   
 }    
  Serial.print("\nData value:");
  Serial.println(data);     
}
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

void callback(char* topic, byte* payload, unsigned int length) 
{
  unsigned long now3 = millis();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  int hangisi=0;

  if((String)topic=="Alarm")
  {
    digitalWrite(yesil, LOW);
    digitalWrite(mavi, LOW);
    digitalWrite(kirmizi, LOW);
    if((char)payload[0]=='A')
    {
      alarm_durumu=true;
    }
    else if((char)payload[0]=='K')
    {
      alarm_durumu=false;
    }
    if((char)payload[0]=='1' && now3 - lastMsg3 > 2000 && alarm_durumu )
    {
        lastMsg3 = now3;
        digitalWrite(yesil, LOW);
        digitalWrite(buzzer, HIGH);
        delay(150);
        digitalWrite(yesil, HIGH);        
        digitalWrite(buzzer, LOW);
        delay(100);
        digitalWrite(yesil, LOW);
        digitalWrite(buzzer, HIGH);
        delay(150);
        digitalWrite(yesil, HIGH);
        digitalWrite(buzzer, LOW);
        delay(100);
        digitalWrite(yesil, LOW);
        digitalWrite(buzzer, HIGH);
        delay(150);
        digitalWrite(yesil, HIGH);
        digitalWrite(buzzer, LOW);
        digitalWrite(yesil, LOW);
        delay(100);        
    }
    if((char)payload[0]=='0'&& now3 - lastMsg3 > 2000 && alarm_durumu )
    {
      digitalWrite(kirmizi, HIGH);
      digitalWrite(buzzer, HIGH);
      delay(2000);
      digitalWrite(kirmizi, LOW);
      digitalWrite(buzzer, LOW);
    }
  }
  else if((String)topic=="Servo")
  {
    if((char)payload[0]=='A')
    {
      servo_durum=true;
    }
    else if((char)payload[0]=='K')
    {
      servo_durum=false;
    }
    if((char)payload[0]=='0' && servo_durum )
    {
      kapi_durumu=0;
      servo.write(0);
      client.publish("Servo2","0");
    }
    else if((char)payload[0]=='9'&& servo_durum )
    {
      servo.write(90);
      client.publish("Servo2","90");
      kapi_durumu=1;      
    }
  }
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","1");
      // ... and resubscribe
      client.subscribe("inTopic");
      client.subscribe("espLed");
      client.subscribe("rfid");
      client.subscribe("Alarm");
      client.subscribe("Servo");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() 
{
  pinMode(sensor, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(yesil, OUTPUT);
  pinMode(mavi, OUTPUT);
  pinMode(kirmizi, OUTPUT);
  pinMode(buzzer, OUTPUT);
//**************************
  digitalWrite(buzzer, LOW);
  digitalWrite(yesil, LOW);
  digitalWrite(mavi, LOW);
  digitalWrite(kirmizi, LOW);
//*****************************
  Serial.begin(115200);
//*************************** 
   delay(10);
   servo.attach(5);
   SPI.begin(); // Init SPI bus
   rfid.PCD_Init(); // Init MFRC522
   Serial.println();
  
//**************************************
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address on serial monitor
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");    //URL IP to be typed in mobile/desktop browser
  Serial.print(WiFi.localIP());
  Serial.println("/");
}
  
void loop() 
{
    //****************************************
  
  unsigned long now2 = millis();
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
//************************************************
  if ( ! rfid.PICC_IsNewCardPresent())
   return;
 // Verify if the NUID has been readed
 if ( ! rfid.PICC_ReadCardSerial())
   return;
  
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);  
    
    
   if(now2 - lastMsg2 > 2000)
   {

      Serial.println();
      lastMsg2 = now2;
      if(data=="a38cf709")
      {
        client.publish("rfid","a38cf709");        
      }
      else if (data=="b3251497")
      {
        client.publish("rfid","b3251497");        
      }
      else 
      {
        client.publish("rfid","FFFFFFFF");
      }
   }
   Serial.println();
   if(kapi_durumu==1)
   {
      delay(300);
      servo.write(0);
      kapi_durumu=0;
   }

 rfid.PICC_HaltA();
 // Stop encryption on PCD
 rfid.PCD_StopCrypto1();
 //**************************************** 
}
