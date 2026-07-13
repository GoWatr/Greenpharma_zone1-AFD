//*******************************
//PROJECT NAME: Green Pharma
//IC USED:ESP 32
//DEVELOPED BY: Jithendra Vignesh
//LAST UPDATE DATE:26/06/2024
//MODIFIED BY: Jithendra vignesh
//******************************

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SensorModbusMaster.h>
#include <SoftwareSerial.h>
#include "Flowmeter.h"
#include "ethernet_function.h"
#include <ESPping.h>

#define CurrentVersion 5.0
#define LED_BUILTIN 2
#define valve1 25
#include "OTAupdate.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial Serial_port(16, 17);  // rx,tx

const byte Hydro_1[] = { 0x04, 0x03, 0x00, 0x00, 0x00, 0x0F, 0x05, 0x9B };
String ID1 = "fm1id:GPFM01";
String ID2 = ",fm1tot:";
String ID3 = ",fm1rate:";
String ID4 = ",fm2id:GPFM02";
String ID5 = ",fm2tot:";
String ID6 = ",fm2rate:";
String ID7 = ",fm3id:GPFM03";
String ID8 = ",fm3tot:";
String ID9 = ",fm3rate:";
String ID10 = ",z1ohtraw:";
String ID11 = ",z1ohtper:";
String ID12 = ",z1valvestatus:";

int ping_fail_counter = 0, i, eth_flag;
int16_t hydro1 = 0;      //raw data for percentage
float hydro1_per = 0.0;  //for decimal point
int16_t hydro1_mm = 0;   //raw data for millimeter
int hydro1_cm = 0;

unsigned long lastDisplayUpdateTime, lastInternetCheckTime, lastDataSendTime = 0;
const unsigned long displayUpdateInterval = 15000;  // 15 seconds
const unsigned long dataSendInterval = 60000;       // 2 min
const unsigned long checkverupdate = 300000;        // 5 minutes in milliseconds
int currentSection = 0;
static unsigned long lastUpdateCheck = 0;
#define maxConnectionAttempts 10

byte Hydro1_byte[15];
int Valve1_status = 0;

String datasend = "";
//String space = "  ";
String hydroSensor = "";

void setup() {
  Serial.begin(115200);
  pinMode(valve1, OUTPUT);
  digitalWrite(valve1, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  lcd.init();  // initialize the lcd
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(7, 0);  // move cursor the first row
  lcd.print("GOWATR");
  lcd.setCursor(4, 1);  // move cursor the first row
  lcd.print("SMART WATER");
  lcd.setCursor(1, 2);
  lcd.print("MANAGEMENT  SYSTEM");  // print message at the first row
  lcd.setCursor(12, 3);             // move cursor to the second row
  lcd.print("VER-");                // print message at the second row
  lcd.setCursor(16, 3);             // move cursor to the second row
  lcd.print(CurrentVersion);
  delay(4000);

  WiFi.onEvent(WiFiEvent);
  ETH.begin(MISO_GPIO, MOSI_GPIO, SCLK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, SPI_HOST);
  delay(100);
  ETH.config(local_IP, gateway, subnet, dns);
  delay(100);

  while (!eth_connected) {
    i++;
    delay(1000);
    Serial.println("Waiting for Ethernet connection");
    // ETH.begin(MISO_GPIO, MOSI_GPIO, SCLK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, SPI_HOST);
    // delay(100);
    // ETH.config(local_IP, gateway, subnet, dns);
    delay(100);
    if (i == 10) {
      break;
    }
  }
  Serial.print("IP Address: ");
  Serial.println(ETH.localIP());
  Serial_port.begin(9600);
  lcd.clear();
  lcd.setCursor(3, 1);  // move cursor the first row
  lcd.print("SYSTEM INIT...");
}

void loop() {
  ping();
  // if (!eth_connected) {
  //   Serial.println("Ethernet connection lost. Trying to reconnect...");
  //   ETH.begin(MISO_GPIO, MOSI_GPIO, SCLK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, SPI_HOST);
  //   delay(100);
  //   ETH.config(local_IP, gateway, subnet, dns);
  //   delay(100);
  //   int attempts = 0;
  //   while (!eth_connected && attempts < maxConnectionAttempts) {
  //     delay(1000);  // Wait for connection
  //     Serial.print(".");
  //     attempts++;
  //   }
  //   //Serial_port.begin(9600);
  //   if (eth_connected) {
  //     Serial.println("\nConnected to Ethernet successfully.");
  //   } else {
  //     Serial.println("\nFailed to reconnect to Ethernet.");
  //   }
  // } else {
  //   ping_fail_counter = 0;
  //   Serial.println("Ethernet connection is already established.");
  // }
  // delay(2000);

  //********************tank_1********************
  hydro_25kl();
  delay(1000);
  Serial.print("25kl terrace tank in mm :  " + String(hydro1_mm));
  Serial.println("\t 25kl terrace tank in %  :  " + String(hydro1_per));
  Serial.println(" ");

  //********************valve control********************
  // Valve 1 control
  if (hydro1_per <= 75 && Hydro1_byte[0] == 4 && Hydro1_byte[1] == 3) {
    digitalWrite(valve1, HIGH);  // Open valve 1
    Valve1_status = 1;
    Serial.println("Valve status : " + String(Valve1_status) + " ON");
  } else if (hydro1_per >= 90 || hydro1 == -1) {
    digitalWrite(valve1, LOW);  // Close valve 1
    Valve1_status = 0;
    Serial.println("Valve status : " + String(Valve1_status) + " OFF");
  }

  delay(3000);
  SUMP_01_ENTRY();
  delay(3000);
  KL40_TO_12KL();
  delay(3000);
  KL40_TO_25KL();
  delay(3000);

  unsigned long currentTime_displaying = millis();
  if (currentTime_displaying - lastDisplayUpdateTime >= displayUpdateInterval) {
    lastDisplayUpdateTime = currentTime_displaying;
    displaySection(currentSection);
    currentSection++;
    if (currentSection >= 4) {
      currentSection = 0;
    }
  }
  unsigned long currentTime_dataSending = millis();
  if (currentTime_dataSending - lastDataSendTime >= dataSendInterval) {
    lastDataSendTime = currentTime_dataSending;
    datasend = ID1 + ID2 + String(Tot2) + ID3 + String(Flow2) + ID4 + ID5 + String(Tot3)
               + ID6 + String(Flow3) + ID7 + ID8 + String(Tot) + ID9 + String(Flow) + ID10
               + String(hydro1_mm) + ID11 + String(hydro1_per) + ID12 + String(Valve1_status);
    sendToServer(datasend);
    Serial.println(datasend);
  }

  unsigned long currentMillis_update = millis();
  if (currentMillis_update - lastUpdateCheck >= checkverupdate) {
    lastUpdateCheck = currentMillis_update;
    check_for_updates();  // Check for updates every hour
  }
  Serial.println(" ");
}


//********************************functions******************************/
void sendToServer(String data) {
  HTTPClient http;
  String url = String(serverUrl) + data;
  Serial.print("url: ");
  Serial.println(url);
  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("HTTP Request failed");
  }
  http.end();
}

void ping() {
  const IPAddress remote_ip(8, 8, 8, 8);
  Serial.print("Pinging ");
  Serial.print(remote_ip);
  if (Ping.ping(remote_ip) > 0) {
    Serial.print(" Ping success ");
    Serial.printf(" response time : %d/%.2f/%d ms\n", Ping.minTime(), Ping.averageTime(), Ping.maxTime());
    eth_flag = 1;
    delay(1000);
  } else {
    Serial.println("Server not reacheble - Ping failed");
    ping_fail_counter++;
    delay(1000);
    eth_flag = 0;
    Serial.println("ping_fail_counter = " + String(ping_fail_counter));
    if (ping_fail_counter >= 100) {
      ESP.restart();
    }
  }
  delay(100);
}
void eth_status_disp() {
  if (eth_flag == 1) {
    lcd.setCursor(14, 3);  // move cursor to the third row
    lcd.print("OKAY");
  } else {
    lcd.setCursor(14, 3);  // move cursor to the third row
    lcd.print("FAIL");
  }
}

void displaySection(int section) {
  switch (section) {
    case 0:
      lcd.clear();
      eth_status_disp();
      lcd.setCursor(2, 0);
      lcd.print("FM:SUMP-01 ENTRY");
      lcd.setCursor(0, 1);
      lcd.print("F~ :");
      lcd.setCursor(6, 1);
      lcd.print(Flow);
      lcd.print(" L/min          ");

      lcd.setCursor(0, 2);
      lcd.print("T+ :");
      lcd.setCursor(6, 2);
      lcd.print(Tot);
      lcd.setCursor(2, 3);  // move cursor to the third row
      lcd.print("ETH STATUS: ");
      break;
    case 1:
      lcd.clear();
      eth_status_disp();
      lcd.setCursor(0, 0);
      lcd.print("FM:40KL TO 12KL(MF1)");
      lcd.setCursor(0, 1);
      lcd.print("F~ :");
      lcd.setCursor(6, 1);
      lcd.print(Flow2);
      lcd.print(" L/min");

      lcd.setCursor(0, 2);
      lcd.print("T+ :");
      lcd.setCursor(6, 2);
      lcd.print(Tot2);
      lcd.setCursor(2, 3);  // move cursor to the third row
      lcd.print("ETH STATUS: ");
      break;
    case 2:
      lcd.clear();
      eth_status_disp();
      lcd.setCursor(0, 0);
      lcd.print("FM:40KL TO 25KL(MF2)");
      lcd.setCursor(0, 1);
      lcd.print("F~ :");
      lcd.setCursor(6, 1);
      lcd.print(Flow3);
      lcd.print(" L/min  ");

      lcd.setCursor(0, 2);
      lcd.print("T+ :");
      lcd.setCursor(6, 2);
      lcd.print(Tot3);
      lcd.setCursor(2, 3);  // move cursor to the third row
      lcd.print("ETH STATUS: ");
      break;
    case 3:
      lcd.clear();
      eth_status_disp();
      lcd.setCursor(1, 0);
      lcd.print("TANK:RWST-25KL-MF2");
      lcd.setCursor(0, 1);
      lcd.print("LEVEL : ");
      lcd.setCursor(9, 1);
      lcd.print(hydro1_per);
      lcd.print(" %           ");

      lcd.setCursor(0, 2);
      lcd.print("VALVE : ");
      if (Valve1_status == 1) {
        lcd.setCursor(9, 2);
        lcd.print("ON ");
      } else if (Valve1_status == 0) {
        lcd.setCursor(9, 2);
        lcd.print("OFF");
      }
      lcd.setCursor(2, 3);  // move cursor to the third row
      lcd.print("ETH STATUS: ");
      delay(1000);
      break;
  }
}

void hydro_25kl(void)  //40kl
{
  bool condition_met = false;  // Flag to track if condition is met
  for (int attempt = 0; attempt < 5; attempt++) {
    if (Serial_port.write(Hydro_1, sizeof(Hydro_1)) == 8) {
      Serial_port.flush();
      delay(1000);
      for (byte i = 0; i <= 14; i++) {
        Hydro1_byte[i] = Serial_port.read();
        delay(50);
      }
      Serial.print("25kl Received BYTES: ");
      for (byte i = 0; i <= 14; i++) {
        Serial.print(Hydro1_byte[i], HEX);
        Serial.print(' ');
      }
      Serial.println(" ");

      if ((Hydro1_byte[0] == 4) && (Hydro1_byte[1] == 3)) {  // Check point
        hydro1 = (Hydro1_byte[7] << 8) | Hydro1_byte[8];
        hydro1_per = hydro1 * 0.1;
        hydro1_mm = (Hydro1_byte[3] << 8) | Hydro1_byte[4];  // Tank level in millimeter
        hydro1_cm = hydro1_mm * 0.1;
        condition_met = true;
        break;
      } else if ((Hydro1_byte[1] == 4) && (Hydro1_byte[2] == 3)) {  // Check point
        hydro1 = (Hydro1_byte[8] << 8) | Hydro1_byte[9];
        hydro1_per = hydro1 * 0.1;
        hydro1_mm = (Hydro1_byte[4] << 8) | Hydro1_byte[5];  // Tank level in millimeter
        hydro1_cm = hydro1_mm * 0.1;
        condition_met = true;
        break;
      } else if ((Hydro1_byte[2] == 4) && (Hydro1_byte[3] == 3)) {  // Check point
        hydro1 = (Hydro1_byte[9] << 8) | Hydro1_byte[10];
        hydro1_per = hydro1 * 0.1;
        hydro1_mm = (Hydro1_byte[5] << 8) | Hydro1_byte[6];  // Tank level in millimeter
        hydro1_cm = hydro1_mm * 0.1;
        condition_met = true;
        break;
      } else if ((Hydro1_byte[3] == 4) && (Hydro1_byte[4] == 3)) {  // Check point
        hydro1 = (Hydro1_byte[10] << 8) | Hydro1_byte[11];
        hydro1_per = hydro1 * 0.1;
        hydro1_mm = (Hydro1_byte[6] << 8) | Hydro1_byte[7];  // Tank level in millimeter
        hydro1_cm = hydro1_mm * 0.1;
        condition_met = true;
        break;
      }
    }
    delay(1000);

    if (condition_met)
      break;
  }
  if (!condition_met) {
    Serial.println("Failed to get valid data from tank 12kl sensor after 5 attempts");
    hydro1 = -1;
    hydro1_per = -1;
    hydro1_mm = -1;
    hydro1_cm = -1;
  }
}