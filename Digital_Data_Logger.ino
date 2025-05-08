/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */

#include <Arduino.h>
#include "soc/gpio_struct.h"  // Include the GPIO structure header
#include "SD.h"
#include "SPI.h"

#define MAX_NUMBER_OF_SAMPLES 100  // size of sample buffer
#define MAX_SAMPLE_LENGTH 4000      // druation of capture in ms

struct sample {
  unsigned long time;
  byte value;
} currentSample = { 0, 0x0 };

const int clkPin = 17;  //Sample Clock output, GPIO 01 = Nano A0
const int trigPin = 18;  //Trigger output, GPIO 02 = Nano A1
const int gpPin = 19;  //general purpose output, GPIO 03 = Nano A2
const uint gpioMaskH = (BIT18 + BIT17);
const uint gpioMaskL = (BIT10 + BIT9 + BIT8 + BIT7 + BIT6 + BIT5);
byte lastSample = 0;
bool triggered = false;
sample buffer[MAX_NUMBER_OF_SAMPLES];  //define the sample buffer
int bufferIdx = 0;
unsigned long trigMillis = 0;
unsigned long trigMicros = 0;
char message[40];



// function declarations
sample getSample();
void reportSerial();
void reportSD();

void setup() {
  delay(10000);  //wait for USB UART to load
  Serial.begin(115200);

  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(clkPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(gpPin, OUTPUT);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
}

void loop() {
  sample input = getSample();
  if (input.value != lastSample) {
    if (!triggered) {
      triggered = true;
      bufferIdx = 0;
      trigMillis = millis();
      trigMicros = input.time;
      digitalWrite(trigPin, HIGH);  // pulse the trigger output pin
      digitalWrite(trigPin, LOW); 
    }
    digitalWrite(gpPin, HIGH);  // pulse the GP output pin
    digitalWrite(gpPin, LOW); 
    if (bufferIdx < MAX_NUMBER_OF_SAMPLES) {
      buffer[bufferIdx] = input;  //store the sample in the buffer if there is room
      bufferIdx += 1;
    }
  }

  if (triggered && (millis() > (trigMillis + MAX_SAMPLE_LENGTH))) {  // check if the capture interval has expired
    reportSerial();
    reportSD();
    triggered = false;
  }
  lastSample = input.value;
}

// put function definitions here:
sample getSample() {
  sample thisSample;
  digitalWrite(clkPin, HIGH);               //Latch the parallel data
  thisSample.time = micros();               // capture the timestamp
  uint inputValue = REG_READ(GPIO_IN_REG);  // capture the GPIO port
  thisSample.value = (((inputValue & gpioMaskH) >> 11) | ((inputValue & gpioMaskL) >> 5));
  digitalWrite(clkPin, LOW);  //end the capure cycle
  return thisSample;
}

void binPrint(int var) {
  for (unsigned int test = 0x80; test; test >>= 1) {
    Serial.write(var & test ? '1' : '0');
  }
  Serial.println();
}


void reportSD() {
  File file = SD.open("/log.csv", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  file.println();
  for (int i = 0; i < bufferIdx; i++) {
    sprintf(message, "%4u, %7u, ", i, (buffer[i].time - trigMicros) );
    file.print(message);
    for (unsigned int test = 0x80; test; test >>= 1) {
      file.write(buffer[i].value & test ? '1' : '0');
    }
    file.println();
  }
  file.close();
}
void reportSerial() {
  Serial.println("#######################");
  for (int i = 0; i < bufferIdx; i++) {
    sprintf(message, "%4u, %7u, ", i, (buffer[i].time - trigMicros) );
    Serial.print(message);
    binPrint(buffer[i].value);
  }
}
