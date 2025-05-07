#define CONVERSIONS_PER_PIN 20  // Define how many conversion per pin will happen and reading the data will be and average of all conversions
int sda = 8;
int scl = 9;

#include "Wire.h"
#include <Adafruit_PCF8574.h>
//#include <PCF8575.h>
#include <Adafruit_PCF8575.h>
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>  //used to enable the onboard RGB LED
#define RGB_PIN 48
Adafruit_NeoPixel pixels(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_PCF8574 pcf1;  // PCF8574 has 8 input ports
//PCF8575 pcf2(0x21);     // PCF8575 has 16 input ports
Adafruit_PCF8575 pcf2;
bool flag_[] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
bool flagOld_[] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
static int pcf1_flag = 1;
static int pcf2_flag = 1;
int last_Switch1 = 1; // Used to save the last position of Rotary Switch 4 positions
int last_Switch2 = 1; // Used to save the last position of Rotary Switch 3 positions
int last_Switch3 = 1; // Used to save the last position of Rotary Switch 8 positions
int Switch1 = 1; // Used to save the current position of Rotary Switch 4 positions
int Switch2 = 1; // Used to save the current position of Rotary Switch 3 positions
int Switch3 = 1; // Used to save the current position of Rotary Switch 8 positions

const int CONSOLES_DIMMER_Pin = 4;  // Potentiometer
const int INST_PNL_DIMMER_Pin = 5;  // Potentiometer
const int FLOOD_DIMMER_Pin = 6; // Potentiometer
const int CHART_DIMMER_Pin = 7; // Potentiometer
const int WARN_CAUTION_DIMMER_Pin = 3; // Potentiometer
const int KY58_Volume = 10;  // Potentiometer
const int LT_Test_Pin = 1;  // Two position Switch
const int MOD_NVG_PIN = 2;  // Three position Switch


uint8_t adc_pins[] = { CONSOLES_DIMMER_Pin, INST_PNL_DIMMER_Pin, FLOOD_DIMMER_Pin, CHART_DIMMER_Pin, WARN_CAUTION_DIMMER_Pin, KY58_Volume, LT_Test_Pin, MOD_NVG_PIN };  //ADC1 pins ffrom ESP32 - ONLY ADC1 pins are supported
uint8_t adc_pins_count = sizeof(adc_pins) / sizeof(uint8_t);  // Calculate how many pins are declared in the array - needed as input for the setup function of ADC Continuous
volatile bool adc_coversion_done = false; // Flag which will be set in ISR when conversion is done

int baselineValue1 = 0;  // Variable to store the baseline value
int baselineValue2 = 0;  // Variable to store the baseline value
int baselineValue3 = 0;  // Variable to store the baseline value
int baselineValue4 = 0;  // Variable to store the baseline value
int baselineValue5 = 0;  // Variable to store the baseline value
int baselineValue6 = 0;  // Variable to store the baseline value

int threshold1 = 0;
int threshold2 = 0;
int threshold3 = 0;
int threshold4 = 0;
int threshold5 = 0;
int threshold6 = 0;

bool LT_Test_flag = false;
bool LT_Test_flag_Old = false;
int MOD_NVG_flag = false;
int MOD_NVG_flag_Old = false;

// Result structure for ADC Continuous reading
adc_continuous_data_t *result = NULL;

// ISR Function that will be triggered when ADC conversion is done
void ARDUINO_ISR_ATTR adcComplete() {
  adc_coversion_done = true;
}

void setup() {
  Serial.begin(250000);
  Wire.begin(8, 9);                                                                      // SDA, SCL
  analogContinuousSetWidth(9);                                                           // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
  analogContinuousSetAtten(ADC_11db);                                                    // Maximum attenuation
  analogContinuous(adc_pins, adc_pins_count, CONVERSIONS_PER_PIN, 20000, &adcComplete);  // Setup ADC Continuous with following input: array of pins, count of the pins, how many conversions per pin in one cycle will happen, sampling frequency, callback function
  analogContinuousStart();                                                               // Start ADC Continuous conversions
  //analogReadResolution(10);               // 10Bit resolution

  // Initializing the PCF8574 4 ports
  if (!pcf1.begin(0x20, &Wire)) {
    Serial.println("Couldn't find PCF8574");
    pcf1_flag = 0;
  } else if (pcf1.begin(0x20, &Wire)) {
    Serial.println("found PCF8574 at address (0x20)");
    pcf1_flag = 1;
  }
  for (uint8_t p = 0; p < 8; p++) {
    pcf1.pinMode(p, INPUT_PULLUP);
    delay(20);
  }
  // end of initializing the PCF8574 4 ports
  // Initializing the PCF8575 8 ports
  if (!pcf2.begin(0x21, &Wire)) {
    Serial.println("Couldn't find PCF8575 (0x21)");
    pcf2_flag = 0;
  } else if (pcf2.begin(0x21, &Wire)) {
    Serial.println("found PCF8574 at address (0x21)");
    pcf2_flag = 1;
  }
  for (uint8_t p=0; p<16; p++) {
    pcf2.pinMode(p, INPUT_PULLUP);
    delay(20);
  }
  // End of initializing the PCF8575 8 ports 
}

void loop() {
// Begining of INTR LT Panel //
  // Begining of ADC potentiometers //
  if (adc_coversion_done == true)  // Check if conversion is done and try to read data
  {
    adc_coversion_done = false;            // Set ISR flag back to false
    if (analogContinuousRead(&result, 0))  // Read data from ADC1 ports
    {
      delay(20);
      baselineValue1 = result[0].avg_read_raw;
      float rate1 = 0.01 + (400 / (baselineValue1 + 1));
      threshold1 = baselineValue1 * rate1;

      baselineValue2 = result[1].avg_read_raw;
      float rate2 = 0.01 + (400 / (baselineValue2 + 1));
      threshold2 = baselineValue2 * rate2;

      baselineValue3 = result[2].avg_read_raw;
      float rate3 = 0.01 + (400 / (baselineValue3 + 1));
      threshold3 = baselineValue3 * rate3;

      baselineValue4 = result[3].avg_read_raw;
      float rate4 = 0.01 + (400 / (baselineValue4 + 1));
      threshold4 = baselineValue4 * rate4;

      baselineValue5 = result[4].avg_read_raw;
      float rate5 = 0.01 + (400 / (baselineValue5 + 1));
      threshold5 = baselineValue5 * rate5;

      baselineValue6 = result[5].avg_read_raw;
      float rate6 = 0.01 + (400 / (baselineValue6 + 1));
      threshold6 = baselineValue6 * rate6;
      //delay(50);

      analogContinuousRead(&result, 0);
      if (abs(result[0].avg_read_raw - baselineValue1) > threshold1) {
        if (result[0].avg_read_raw >= 100) {
          Serial.printf("CONSOLES_DIMMER %3d\r\n", map(result[0].avg_read_raw, 0, 4000, 0, 65536));
          pixels.setPixelColor(0, pixels.Color(map(result[0].avg_read_raw, 0, 4000, 0, 150), map(result[1].avg_read_raw, 0, 4000, 0, 150), map(result[2].avg_read_raw, 0, 4000, 0, 150)));
          pixels.show();
        }
      }
      if (abs(result[1].avg_read_raw - baselineValue2) > threshold2) {
        if (result[1].avg_read_raw >= 100) {
          Serial.printf("INST_PNL_DIMMER %3d\r\n", map(result[1].avg_read_raw, 0, 4000, 0, 65536));
          pixels.setPixelColor(0, pixels.Color(map(result[0].avg_read_raw, 0, 4000, 0, 150), map(result[1].avg_read_raw, 0, 4000, 0, 150), map(result[2].avg_read_raw, 0, 4000, 0, 150)));
          pixels.show();
        }
      }
      if (abs(result[2].avg_read_raw - baselineValue3) > threshold3) {
        if (result[2].avg_read_raw >= 100) {
          Serial.printf("FLOOD_DIMMER %3d\r\n", map(result[2].avg_read_raw, 0, 4000, 0, 65536));
          pixels.setPixelColor(0, pixels.Color(map(result[0].avg_read_raw, 0, 4000, 0, 150), map(result[1].avg_read_raw, 0, 4000, 0, 150), map(result[2].avg_read_raw, 0, 4000, 0, 150)));
          pixels.show();
        }
      }
      if (abs(result[3].avg_read_raw - baselineValue4) > threshold4) {
        if (result[3].avg_read_raw >= 100) Serial.printf("CHART_DIMMER %3d\r\n", map(result[3].avg_read_raw, 0, 4000, 0, 65536));
      }
      if (abs(result[4].avg_read_raw - baselineValue5) > threshold5) {
        if (result[4].avg_read_raw >= 100) Serial.printf("WARN_CAUTION_DIMMER %3d\r\n", map(result[4].avg_read_raw, 0, 4000, 0, 65536));
      }
      if (abs(result[5].avg_read_raw - baselineValue6) > threshold6) {
        if (result[5].avg_read_raw >= 100) Serial.printf("KY58_VOLUME %3d\r\n", map(result[5].avg_read_raw, 0, 4000, 0, 65536));
      }
    }
  }
    // End of ADC potentiometers //
    // Begining of Two positions Switch //
    if (result[6].avg_read_raw > 3000) {
      LT_Test_flag = true;
      if (LT_Test_flag != LT_Test_flag_Old) {
        Serial.println("LIGHTS_TEST_SW 1");
        LT_Test_flag_Old = LT_Test_flag;
      }
    }
    if (result[6].avg_read_raw < 1000) {
      LT_Test_flag = false;
      if (LT_Test_flag != LT_Test_flag_Old) {
        Serial.println("LIGHTS_TEST_SW 0");
        LT_Test_flag_Old = LT_Test_flag;
      }
    }
    // end of Two positions Switch //
    // Begining of threee positions Switch //
    if (result[7].avg_read_raw > 3000) {
      MOD_NVG_flag = 0;
      if (MOD_NVG_flag != MOD_NVG_flag_Old) {
        Serial.println("COCKKPIT_LIGHT_MODE_SW 0");
        MOD_NVG_flag_Old = MOD_NVG_flag;
      }
    }
    if (result[7].avg_read_raw > 1500 && result[7].avg_read_raw < 2500) {
      MOD_NVG_flag = 1;
      if (MOD_NVG_flag != MOD_NVG_flag_Old) {
        Serial.println("COCKKPIT_LIGHT_MODE_SW 1");
        MOD_NVG_flag_Old = MOD_NVG_flag;
      }
    }
    if (result[7].avg_read_raw < 1000) {
      MOD_NVG_flag = 2;
      if (MOD_NVG_flag != MOD_NVG_flag_Old) {
        Serial.println("COCKKPIT_LIGHT_MODE_SW 2");
        MOD_NVG_flag_Old = MOD_NVG_flag;
      }
    }
    // End of threee positions Switch //
  // End of INTR LT Panel //
  // Begining of ELEC Panel //
    if (pcf1_flag == 1) 
    {
    // Two position Switches //
      if (pcf1.digitalRead(0) == true) {
        flag_[0] = true;
        if (flagOld_[0] != flag_[0]) Serial.println("R_GEN_SW 0");
        flagOld_[0] = flag_[0];
      }
      if (pcf1.digitalRead(0) == false) {
        flag_[0] = false;
        if (flagOld_[0] != flag_[0]) Serial.println("R_GEN_SW 1");
        flagOld_[0] = flag_[0];
      }
    //------------//
      if (pcf1.digitalRead(3) == true) {
        flag_[3] = true;
        if (flagOld_[3] != flag_[3]) Serial.println("L_GEN_SW 0");
        flagOld_[3] = flag_[3];
      }
      if (pcf1.digitalRead(3) == false) {
        flag_[3] = false;
        if (flagOld_[3] != flag_[3]) Serial.println("L_GEN_SW 1");
        flagOld_[3] = flag_[3];
      }
    // End of two position Switches //
    // Start of Three position Switches //
    if (pcf1.digitalRead(1) == true) {
        flag_[1] = true;
        if (flagOld_[1] != flag_[1]) Serial.println("BATTERY_SW 1");
        delay(30);
        flagOld_[1] = flag_[1];
    }
    if (pcf1.digitalRead(1) == false) {
        flag_[1] = false;
        if (flagOld_[1] != flag_[1]) Serial.println("BATTERY_SW 0");
        delay(30);
        flagOld_[1] = flag_[1];
    }
    if (pcf1.digitalRead(2) == true) {
        flag_[2] = true;
        if (flagOld_[2] != flag_[2]) Serial.println("BATTERY_SW 1");
        delay(30);
        flagOld_[2] = flag_[2];
    }
    if (pcf1.digitalRead(2) == false) {
        flag_[2] = false;
        if (flagOld_[2] != flag_[2]) Serial.println("BATTERY_SW 2");
        delay(30);
        flagOld_[2] = flag_[2];
    }
    // End of Three position Switches //
  // End of ELEC Panel //
  }
  // Begining of KY 58 Panel //
    if (pcf2_flag == 1) 
    {
      // Start of Rotary Switch 4 positions //
      for (uint8_t p1 = 0; p1 <= 3; p1++)
      {
        if (!pcf2.digitalRead(p1))
        {
          if (p1 == 0) {
              Switch1 = 0;
              if (last_Switch1 != Switch1) Serial.println("KY58_MODE_SELECT 0");
              delay(30);
          }
          if (p1 == 1) {
              Switch1 = 1;
              if (last_Switch1 != Switch1) Serial.println("KY58_MODE_SELECT 1");
              delay(30);
          }
          if (p1 == 2) {
              Switch1 = 2;
              if (last_Switch1 != Switch1) Serial.println("KY58_MODE_SELECT 2"); 
              delay(30);          
          }
          if (p1 == 3) {
              Switch1 = 3;
              if (last_Switch1 != Switch1) Serial.println("KY58_MODE_SELECT 3");
              delay(30);           
          }          
        }
        last_Switch1 = Switch1;
      }
      // End of Rotary Switch 4 positions //
      // Start of Rotary Switch 3 positions //
      for (uint8_t p2 = 4; p2 <= 6; p2++)
      {
        if (!pcf2.digitalRead(p2))
        {
          if (p2 == 4) {
              Switch2 = 4;
              if (last_Switch2 != Switch2) Serial.println("KY58_POWER_SELECT 0");
              delay(30);
          }
          if (p2 == 5) {
              Switch2 = 5;
              if (last_Switch2 != Switch2) Serial.println("KY58_POWER_SELECT 1");
              delay(30);
          }
          if (p2 == 6) {
              Switch2 = 6;
              if (last_Switch2 != Switch2) Serial.println("KY58_POWER_SELECT 2"); 
              delay(30);          
          }
        }
        last_Switch2 = Switch2;
      }
      // End of Rotary Switch 3 positions //
      // Start of Rotary Switch 8 positions //
      for (uint8_t p2 = 7; p2 <= 14; p2++)
      {
        if (!pcf2.digitalRead(p2))
        {
          if (p2 == 7) {
              Switch3 = 7;
              if (last_Switch3 != Switch3) {
                //Serial.println("KY58_FILL_SELECT 0");
                Serial.println("KY58_FILL_SEL_PULL 1");
              }
              delay(30);
          }
          if (p2 == 8) {
              Switch3 = 8;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 0");
                Serial.println("KY58_FILL_SEL_PULL 0");
              }
              delay(30);
          }
          if (p2 == 9) {
              Switch3 = 9;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 1");
                Serial.println("KY58_FILL_SEL_PULL 0");
              }
              delay(30);          
          }
          if (p2 == 10) {
              Switch3 = 10;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 2");
                Serial.println("KY58_FILL_SEL_PULL 0");
              }
              delay(30);
          }
          if (p2 == 11) {
              Switch3 = 11;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 3");
               Serial.println("KY58_FILL_SEL_PULL 0");
              }
              delay(30);
          }
          if (p2 == 12) {
              Switch3 = 12;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 4"); 
                Serial.println("KY58_FILL_SEL_PULL 0");
              }
              delay(30);          
          }
          if (p2 == 13) {
              Switch3 = 13;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 5");
                Serial.println("KY58_FILL_SEL_PULL 0");
              }
              delay(30);
          }
          if (p2 == 14) {
              Switch3 = 14;
              if (last_Switch3 != Switch3) {
                Serial.println("KY58_FILL_SELECT 6");
                Serial.println("KY58_FILL_SEL_PULL 1");
              }
              delay(30);
          }
        }
        last_Switch3 = Switch3;
      }
      // End of Rotary Switch 3 positions //
    }
    // End of KY 58 Panel //
   }

  