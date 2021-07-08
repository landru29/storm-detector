# Lightning detector

## Libraries

- OneWire
- SparkFun AS3935
- Adafruit SSD1306
- Piezzo (optional)

## Parts

- 1x Oled 128x64
- 1x AS3935
- 1x Arduino nano Atmega 328P
- 1x Rotary encoder
- 2x Capacitor 0.1µF

## Connections

### Screen OLED

- OLED GND <-> Arduino GND
- OLED VDD <-> Arduino 5V
- OLED SCK <-> Arduino A5
- OLED SDA <-> Arduino A4

### AS3935 Detector

- AS3935 VDD  <-> Arduino 5V
- AS3935 GND  <-> Arduino GND
- AS3935 SCL  <-> ARDUINO D13
- AS3935 MOSI <-> ARDUINO D11
- AS3935 MISO <-> ARDUINO D12
- AS3935 CS   <-> ARDUINO D10
- AS3935 SI   <-> Arduino GND (force spi)
- AS3935 IRQ  <-> Arduino D3
- AS3935 EN_V <-> Arduino 5V
- AS3935 A0   <-> Arduino 5V
- AS3935 A1   <-> Arduino 5V

### Rotary Encoder

``` text
    -----------
 A--|         |
    | Rotary  |--S1
 C--|         |
    | Encoder |--S2
 B--|         |
    -----------
```

- Encoder S1 <-> Arduino GND
- Encoder S2 <-> Arduino D4
- Encoder A  <-> Arduino D2
- Encoder B  <-> Arduino D5
- Encoder C  <-> Arduino GND

### Capacitors

- Cap1 + <-> Arduino D2
- Cap1 - <-> Arduino GND
- Cap2 + <-> Arduino D5
- Cap2 - <-> Arduino GND

### Piezzo (optional)

- Piezzo <-> Arduino D7
- Piezzo <-> Arduino GND