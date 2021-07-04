# Lightning detector

## Libraries

- OneWire
- SparkFun AS3935
- Adafruit SSD1306

## Parts

- Oled 128x64
- AS3935
- Arduino nano Atmega 328P

## Pins

### Screen

- OLED GND <-> Arduino GND
- OLED VDD <-> Arduino 5V
- OLED SCK <-> Arduino A5
- OLED SDA <-> Arduino A4

### Detector

- AS3935 GND <-> Arduino GND
- AS3935 VDD <-> Arduino 5V
- AS3935 INT <-> Arduino D3
- AS3935 CS <-> ARDUINO D10
- AS3935 MOSI <-> ARDUINO D11
- AS3935 MISO <-> ARDUINO D12
- AS3935 SCL <-> ARDUINO D13
- AS3935 SI <-> Arduino CND (force spi)
- AS3935 EN_V <-> Arduino 5V
- AS3935 A0 <-> Arduino 5V
- AS3935 A1 <-> Arduino 5V