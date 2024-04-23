#include "Wire.h"

#define SDA 19
#define SCL 20

// I2C address of PCA9538
#define PCA9538_ADDR 0x73

// PCA9538 register addresses
#define PCA9538_INPUT_REG 0x00
#define PCA9538_OUTPUT_REG 0x01
#define PCA9538_POLARITY_REG 0x02
#define PCA9538_CONFIG_REG 0x03

// GPIO pins
#define GPIO5 0x10
#define GPIO6 0x20
#define GPIO7 0x40
#define GPIO8 0x80

#define NUM_OUTPUT_PINS 4  // Change this to the number of output pins you have

void setPinMode(uint8_t pin, uint8_t mode) {
  uint8_t config = readRegister(PCA9538_CONFIG_REG);
  if (mode == INPUT) {
    config |= pin;
  } else {
    config &= ~pin;
  }
  writeRegister(PCA9538_CONFIG_REG, config);
}

void writeOutput(uint8_t pin, uint8_t value) {
  uint8_t output = readRegister(PCA9538_OUTPUT_REG);
  if (value == LOW) {
    output &= ~pin;
  } else {
    output |= pin;
  }
  writeRegister(PCA9538_OUTPUT_REG, output);
}
uint8_t readInput() {
  return readRegister(PCA9538_INPUT_REG);
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(PCA9538_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(PCA9538_ADDR, 1);
  return Wire.read();
}

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(PCA9538_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void setup() {
  Wire.begin(SDA, SCL);
  setPinMode(GPIO5, OUTPUT);
  setPinMode(GPIO6, OUTPUT);
  setPinMode(GPIO7, OUTPUT);
  setPinMode(GPIO8, OUTPUT);
}

void loop() {
  writeOutput(GPIO5, 0);
  writeOutput(GPIO6, 0);
  writeOutput(GPIO7, 0);
  writeOutput(GPIO8, 0);
  delay(300);

  writeOutput(GPIO5, 1);
  writeOutput(GPIO6, 0);
  writeOutput(GPIO7, 0);
  writeOutput(GPIO8, 0);
  delay(300);

  writeOutput(GPIO5, 0);
  writeOutput(GPIO6, 1);
  writeOutput(GPIO7, 0);
  writeOutput(GPIO8, 0);
  delay(300);

  writeOutput(GPIO5, 0);
  writeOutput(GPIO6, 0);
  writeOutput(GPIO7, 1);
  writeOutput(GPIO8, 0);
  delay(300);

  writeOutput(GPIO5, 0);
  writeOutput(GPIO6, 0);
  writeOutput(GPIO7, 0);
  writeOutput(GPIO8, 1);
  delay(300);
}
