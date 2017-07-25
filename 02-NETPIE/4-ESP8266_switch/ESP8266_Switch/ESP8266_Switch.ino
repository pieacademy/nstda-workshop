#include <ESP8266WiFi.h>
#include <MicroGear.h>

const char* ssid     = "YOUR_WIFI_SSID";          // ชื่อ ssid
const char* password = "YOUR_WIFI_PASSWORD";      // รหัสผ่าน wifi

#define APPID   "APPID"                     // ให้แทนที่ด้วย AppID รวม
#define KEY     "KEY"                       // ให้แทนที่ด้วย Key รวม
#define SECRET  "SECRET"                    // ให้แทนที่ด้วย Secret รวม

#define ALIAS   "YOUR_UNIQUE_ALIAS"         // แทนที่ด้วยหมายเลขของท่าน เช่น "A01"
#define NEIGHBOR "NEIGHBOR_ALIAS"           // ชื่ออุปกรณ์ของเพื่อน เช่น "A02"

#define BUTTONPIN  D3               // ต่อกับปุ่ม Flash บนบอร์ด NodeMCU
#define LEDPIN     LED_BUILTIN      // ต่อกับไฟ LED บนบอร์ด NodeMCU

int currentLEDState = 0;
int lastLEDState = 1;
int currentButtonState = 0;
int lastButtonState = 0;

WiFiClient client;

int timer = 0;
MicroGear microgear(client);

void updateLED(int state) {
    // ไฟ LED บน NodeMCU จะติดก็ต่อเมื่อส่งค่า LOW ไปให้ LEDPIN 
    if(state==1 && currentLEDState == 0){
        currentLEDState = 1;
        digitalWrite(LEDPIN, LOW); // LED ON
    }
    else if (state==0 && currentLEDState == 1) {
        currentLEDState = 0;
        digitalWrite(LEDPIN, HIGH); // LED OFF
    }
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    Serial.println((char *)msg);

    if (*(char *)msg == '0') updateLED(0);
    else if (*(char *)msg == '1') updateLED(1);
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    microgear.setAlias(ALIAS);
}

void setup() {
    microgear.on(MESSAGE,onMsghandler);
    microgear.on(CONNECTED,onConnected);

    Serial.begin(115200);
    Serial.println("Starting...");

    // กำหนดชนิดของ PIN (ขาI/O) เช่น INPUT, OUTPUT เป็นต้น
    pinMode(LEDPIN, OUTPUT);          // LED pin mode กำหนดค่า
    pinMode(BUTTONPIN, INPUT);        // Button pin mode รับค่า
    updateLED(currentLEDState);

    if (WiFi.begin(ssid, password)) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.print(".");
        }
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    microgear.init(KEY,SECRET,ALIAS);   // กำหนดค่าตันแปรเริ่มต้นให้กับ microgear
    microgear.connect(APPID);           // ฟังก์ชั่นสำหรับเชื่อมต่อ NETPIE
}

void loop() {
    if (microgear.connected()) {
        microgear.loop();

        if(currentLEDState != lastLEDState){
          microgear.publish("/LEDstate", currentLEDState);
          lastLEDState = currentLEDState;
        }

        currentButtonState = digitalRead(BUTTONPIN);
        if(currentButtonState != lastButtonState){
          microgear.chat(NEIGHBOR, currentButtonState);
          lastButtonState = currentButtonState;
        }
    }
    else {
        Serial.println("connection lost, reconnect...");
        microgear.connect(APPID); 
    }
}
