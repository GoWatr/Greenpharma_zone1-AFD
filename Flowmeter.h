#include <SoftwareSerial.h>

const byte requestframe1[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x21, 0x85, 0xD2 };
const byte requestframe2[] = { 0x02, 0x03, 0x00, 0x00, 0x00, 0x21, 0x85, 0xE1 };
const byte requestframe3[] = { 0x03, 0x03, 0x00, 0x00, 0x00, 0x21, 0x84, 0x30 };

extern SoftwareSerial Serial_port;

int V8_BIT_1, V8_2BIT_1, V8_BIT_1_2, V8_2BIT_1_2, V8_BIT_1_3, V8_2BIT_1_3 = 0;
int V8_BIT_2, V8_2BIT_2, V8_BIT_2_2, V8_2BIT_2_2, V8_BIT_2_3, V8_2BIT_2_3 = 0;
int V8_BIT_3, V8_BIT_3_2, V8_BIT_3_3 = 0;
uint64_t V16_BIT_1, V16_BIT_2, V16_BIT_3 = 0;
float Flow, Flow2, Flow3 = 0.0;
String Tot, Tot2, Tot3;
uint8_t values[33], values2[33], values3[33];

void SUMP_01_ENTRY() {
  bool condition_met = false;
  bool condition_met1 = false;
  bool condition_met2 = false;
  for (int attempt = 0; attempt < 5; attempt++) {
    memset(values, 0, sizeof(values));
    if (Serial_port.write(requestframe1, sizeof(requestframe1)) == 8) {
      delay(1000);
      Serial_port.flush();
      for (uint8_t i = 0; i < 32; i++) {
        values[i] = Serial_port.read();
        delay(50);
      }
      Serial.println("---- Bore Flowmeter ----");
      Serial.print("Received Data: ");
      for (uint8_t i = 0; i < 32; i++) {
        Serial.print(values[i], HEX);
        Serial.print(' ');
      }
      Serial.println(' ');

      if ((values[0] == 1) && (values[1] == 3) && (values[2] == 0x42)) {
        condition_met = true;
        break;
      } else if ((values[1] == 1) && (values[2] == 3) && (values[3] == 0x42)) {
        condition_met1 = true;
        break;
      } else if ((values[2] == 1) && (values[3] == 3) && (values[4] == 0x42)) {
        condition_met2 = true;
        break;
      }
    } else {
      Serial.println("Failed to send request to Bore Flowmeter");
    }
    delay(1000);
  }

  if (condition_met) {
    if ((values[0] == 1) && (values[1] == 3)) {
      //FLOW
      V8_2BIT_1 = (values[5] << 8) | values[6];

      //Tot
      V8_BIT_1 = (values[19] << 8) | values[20];
      V8_BIT_2 = (values[21] << 8) | values[22];
      V16_BIT_1 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values[23] << 8) | values[24];

      //final
      Flow = V8_2BIT_1 * (1000 / 60);
      Flow = Flow * 0.001;
      Tot = String(static_cast<uint32_t>(V16_BIT_1)) + "." + String(V8_BIT_3);

      Serial.println("Flow =" + String(Flow));
      Serial.println("Tot  =" + String(Tot));
    }
  } else if (condition_met1) {
    if ((values[1] == 1) && (values[2] == 3)) {
      //FLOW
      V8_2BIT_1 = (values[6] << 8) | values[7];

      //Tot
      V8_BIT_1 = (values[20] << 8) | values[21];
      V8_BIT_2 = (values[22] << 8) | values[23];
      V16_BIT_1 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values[24] << 8) | values[25];

      //final
      Flow = V8_2BIT_1 * (1000 / 60);
      Flow = Flow * 0.001;
      Tot = String(static_cast<uint32_t>(V16_BIT_1)) + "." + String(V8_BIT_3);

      Serial.println("Flow =" + String(Flow));
      Serial.println("Tot  =" + String(Tot));
    }
  } else if (condition_met2) {
    if ((values[2] == 1) && (values[3] == 3)) {
      //FLOW
      V8_2BIT_1 = (values[7] << 8) | values[8];

      //Tot
      V8_BIT_1 = (values[21] << 8) | values[22];
      V8_BIT_2 = (values[23] << 8) | values[24];
      V16_BIT_1 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values[25] << 8) | values[26];

      //final
      Flow = V8_2BIT_1 * (1000 / 60);
      Flow = Flow * 0.001;
      Tot = String(static_cast<uint32_t>(V16_BIT_1)) + "." + String(V8_BIT_3);

      Serial.println("Flow =" + String(Flow));
      Serial.println("Tot  =" + String(Tot));
    }
  } else {
    Serial.println("Failed to get valid data from Bore Flowmeter after 5 attempts");
    Flow = -1;
    Tot = -1;
    Serial.println("Flow =" + String(Flow));
    Serial.println("Tot  =" + String(Tot));
  }
}

void KL40_TO_12KL() {
  bool condition_met = false;
  bool condition_met1 = false;
  bool condition_met2 = false;
  for (int attempt = 0; attempt < 5; attempt++) {
    memset(values2, 0, sizeof(values2));
    if (Serial_port.write(requestframe2, sizeof(requestframe2)) == 8) {
      delay(1000);
      Serial_port.flush();
      for (uint8_t i = 0; i < 32; i++) {
        values2[i] = Serial_port.read();
        delay(50);
      }
      Serial.println("---- KL12 Flowmeter ----");
      Serial.print("Received Data: ");
      for (uint8_t i = 0; i < 32; i++) {
        Serial.print(values2[i], HEX);
        Serial.print(' ');
      }
      Serial.println(' ');

      if ((values2[0] == 2) && (values2[1] == 3) && (values2[2] == 0x42)) {
        condition_met = true;
        break;
      } else if ((values2[1] == 2) && (values2[2] == 3) && (values2[3] == 0x42)) {
        condition_met1 = true;
        break;
      } else if ((values2[2] == 2) && (values2[3] == 3) && (values2[4] == 0x42)) {
        condition_met2 = true;
        break;
      }
    } else {
      Serial.println("Failed to send request to KL12 Flowmeter");
    }
    delay(1000);
  }

  if (condition_met) {
    if ((values2[0] == 2) && (values2[1] == 3)) {
      //FLOW
      V8_2BIT_1 = (values2[5] << 8) | values2[6];

      //Tot
      V8_BIT_1 = (values2[19] << 8) | values2[20];
      V8_BIT_2 = (values2[21] << 8) | values2[22];
      V16_BIT_2 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values2[23] << 8) | values2[24];

      //final
      Flow2 = V8_2BIT_1 * (1000 / 60);
      Flow2 = Flow2 * 0.001;
      Tot2 = String(static_cast<uint32_t>(V16_BIT_2)) + "." + String(V8_BIT_3);

      Serial.println("Flow2 =" + String(Flow2));
      Serial.println("Tot2  =" + String(Tot2));
    }
  } else if (condition_met1) {
    if ((values2[1] == 2) && (values2[2] == 3)) {
      //FLOW
      V8_2BIT_1 = (values2[6] << 8) | values2[7];

      //Tot
      V8_BIT_1 = (values2[20] << 8) | values2[21];
      V8_BIT_2 = (values2[22] << 8) | values2[23];
      V16_BIT_2 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values2[24] << 8) | values2[25];

      //final
      Flow2 = V8_2BIT_1 * (1000 / 60);
      Flow2 = Flow2 * 0.001;
      Tot2 = String(static_cast<uint32_t>(V16_BIT_2)) + "." + String(V8_BIT_3);

      Serial.println("Flow2 =" + String(Flow2));
      Serial.println("Tot2  =" + String(Tot2));
    }
  } else if (condition_met2) {
    if ((values2[2] == 2) && (values2[3] == 3)) {
      //FLOW
      V8_2BIT_1 = (values2[7] << 8) | values2[8];

      //Tot
      V8_BIT_1 = (values2[21] << 8) | values2[22];
      V8_BIT_2 = (values2[23] << 8) | values2[24];
      V16_BIT_2 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values2[25] << 8) | values2[26];

      //final
      Flow2 = V8_2BIT_1 * (1000 / 60);
      Flow2 = Flow2 * 0.001;
      Tot2 = String(static_cast<uint32_t>(V16_BIT_2)) + "." + String(V8_BIT_3);

      Serial.println("Flow2 =" + String(Flow2));
      Serial.println("Tot2  =" + String(Tot2));
    }
  } else {
    Serial.println("Failed to get valid data from KL12 Flowmeter after 5 attempts");
    Flow2 = -1;
    Tot2 = -1;
    Serial.println("Flow2 =" + String(Flow2));
    Serial.println("Tot2  =" + String(Tot2));
  }
}

void KL40_TO_25KL() {
  bool condition_met = false;
  bool condition_met1 = false;
  bool condition_met2 = false;
  for (int attempt = 0; attempt < 5; attempt++) {
    memset(values3, 0, sizeof(values3));
    if (Serial_port.write(requestframe3, sizeof(requestframe3))) {
      delay(1000);
      Serial_port.flush();
      for (uint8_t i = 0; i < 32; i++) {
        values3[i] = Serial_port.read();
        delay(50);
      }
      Serial.println("---- KL25 Flowmeter ----");
      Serial.print("Received Data: ");
      for (uint8_t i = 0; i < 32; i++) {
        Serial.print(values3[i], HEX);
        Serial.print(' ');
      }
      Serial.println(' ');

      if ((values3[0] == 3) && (values3[1] == 3) && (values3[2] == 0x42)) {
        condition_met = true;
        break;
      } else if ((values3[1] == 3) && (values3[2] == 3) && (values3[3] == 0x42)) {
        condition_met1 = true;
        break;
      } else if ((values3[2] == 3) && (values3[3] == 3) && (values3[4] == 0x42)) {
        condition_met2 = true;
        break;
      }
    } else {
      Serial.println("Failed to send request to KL25 Flowmeter");
    }
    delay(1000);
  }

  if (condition_met) {
    if ((values3[0] == 3) && (values3[1] == 3)) {
      //FLOW
      V8_2BIT_1 = (values3[5] << 8) | values3[6];

      //Tot
      V8_BIT_1 = (values3[19] << 8) | values3[20];
      V8_BIT_2 = (values3[21] << 8) | values3[22];
      V16_BIT_3 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values3[23] << 8) | values3[24];

      //final
      Flow3 = V8_2BIT_1 * (1000 / 60);
      Flow3 = Flow3 * 0.001;
      Tot3 = String(static_cast<uint32_t>(V16_BIT_3)) + "." + String(V8_BIT_3);

      Serial.println("Flow3 =" + String(Flow3));
      Serial.println("Tot3  =" + String(Tot3));
    }
  } else if (condition_met1) {
    if ((values3[1] == 3) && (values3[2] == 3)) {
      //FLOW
      V8_2BIT_1 = (values3[6] << 8) | values3[7];

      //Tot
      V8_BIT_1 = (values3[20] << 8) | values3[21];
      V8_BIT_2 = (values3[22] << 8) | values3[23];
      V16_BIT_3 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values3[24] << 8) | values3[25];

      //final
      Flow3 = V8_2BIT_1 * (1000 / 60);
      Flow3 = Flow3 * 0.001;
      Tot3 = String(static_cast<uint32_t>(V16_BIT_3)) + "." + String(V8_BIT_3);

      Serial.println("Flow3 =" + String(Flow3));
      Serial.println("Tot3  =" + String(Tot3));
    }
  } else if (condition_met2) {
    if ((values3[2] == 3) && (values3[3] == 3)) {
      //FLOW
      V8_2BIT_1 = (values3[5] << 8) | values3[6];

      //Tot
      V8_BIT_1 = (values3[21] << 8) | values3[22];
      V8_BIT_2 = (values3[23] << 8) | values3[24];
      V16_BIT_3 = static_cast<uint64_t>(V8_BIT_1) << 16 | V8_BIT_2;
      V8_BIT_3 = (values3[25] << 8) | values3[26];

      //final
      Flow3 = V8_2BIT_1 * (1000 / 60);
      Flow3 = Flow3 * 0.001;
      Tot3 = String(static_cast<uint32_t>(V16_BIT_3)) + "." + String(V8_BIT_3);

      Serial.println("Flow3 =" + String(Flow3));
      Serial.println("Tot3  =" + String(Tot3));
    }
  } else {
    Serial.println("Failed to get valid data from KL25 Flowmeter after 5 attempts");
    Flow3 = -1;
    Tot3 = -1;
    Serial.println("Flow3 =" + String(Flow3));
    Serial.println("Tot3  =" + String(Tot3));
  }
}
