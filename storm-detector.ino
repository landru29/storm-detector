#include <Wire.h>
#include <SPI.h>
#include "SparkFun_AS3935.h"
#include "Adafruit_SSD1306.h"
#include "splash.h"

//#define DEBUG

//SPI chip select pin
#define CHIP_SELECT 10

// Detector mode
#define INDOOR 0x12 
#define OUTDOOR 0xE

// Type of signal
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04
#define NOISE_INT     0x01

// Interrupt pin for lightning detection
#define LIGHTNING_INTERRUPT_PIN 3

// OLED constants
#define SCREEN_WIDTH   128   // OLED display width, in pixels
#define SCREEN_HEIGHT  64    // OLED display height, in pixels
#define OLED_RESET     4     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

SparkFun_AS3935 lightning;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned int strikeCount = 0;
unsigned int noiseCount = 0;
unsigned int disturberCount = 0;

// Detector configuration
int noise           = 2; // Value between 1-7
int watchDogVal     = 2; // Value between 1-10
int spike           = 2; // Value between 1-11
int lightningThresh = 1; // Value in [1, 5, 9, 16]

// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector.
byte intVal      = 0;

void setup() {
    // When lightning is detected the interrupt pin goes HIGH.
    pinMode(LIGHTNING_INTERRUPT_PIN, INPUT);

    #ifdef DEBUG
    Serial.begin(115200);
    #endif // DEBUG

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        #ifdef DEBUG
        Serial.println("SSD1306 allocation failed, freezing!");
        #endif // DEBUG
        while (1);
    }

    display.clearDisplay();
    display.drawBitmap(
        (SCREEN_WIDTH - splash_width) / 2,
        (SCREEN_HEIGHT - splash_height) / 2,
        splash_data,
        splash_width,
        splash_height,
        1
    );

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();

    delay(2000); // Pause for 2 seconds

    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font

    display.writeFillRect(0, 0, SCREEN_WIDTH, 16, 0);
    display.setCursor(0, 0);
    display.println(" LIGHTNINGS - starting ");
    display.display();


    #ifdef DEBUG
    Serial.println("AS3935 Franklin Lightning Detector");
    #endif // DEBUG

    SPI.begin(); 

    if (!lightning.beginSPI(CHIP_SELECT)) {
        #ifdef DEBUG
        Serial.println("Lightning Detector did not start up, freezing!");
        #endif // DEBUG
        display.writeFillRect(0, 0, SCREEN_WIDTH, 16, 0);
        display.setCursor(0, 0);
        display.println(" LIGHTNINGS - ERROR ");
        display.writeFillRect(0, 16, SCREEN_WIDTH, SCREEN_HEIGHT-16, 0);
        display.setCursor(0, 16);
        display.println("Detector not found");
        display.display();
        while (1);
    }

    #ifdef DEBUG
    Serial.println("Lightning Detector Ready!");
    #endif // DEBUG
    display.writeFillRect(0, 0, SCREEN_WIDTH, 16, 0);
    display.setCursor(0, 0);
    display.println(" LIGHTNINGS - AS3935 ");
    display.display();

    // The lightning detector defaults to an indoor setting at
    // the cost of less sensitivity, if you plan on using this outdoors
    // uncomment the following line:
    lightning.setIndoorOutdoor(OUTDOOR);

    // Noise floor setting from 1-7, one being the lowest. Default setting is
    // 2. If you need to check the setting, the corresponding function for
    // reading the function follows. 
    lightning.setNoiseLevel(noise);

    // Watchdog threshold setting can be from 1-10, one being the lowest. Default setting is
    // 2. If you need to check the setting, the corresponding function for
    // reading the function follows.    
    lightning.watchdogThreshold(watchDogVal);

    // Spike Rejection setting from 1-11, one being the lowest. Default setting is
    // 2. If you need to check the setting, the corresponding function for
    // reading the function follows.    
    // The shape of the spike is analyzed during the chip's
    // validation routine. You can round this spike at the cost of sensitivity to
    // distant events. 
    lightning.spikeRejection(spike); 

    // This setting will change when the lightning detector issues an interrupt.
    // For example you will only get an interrupt after five lightning strikes
    // instead of one. Default is one, and it takes settings of 1, 5, 9 and 16.   
    // Followed by its corresponding read function. Default is zero. 
    lightning.lightningThreshold(lightningThresh); 

    // When signal is raising on interrupt pin (when a lighting occurs),
    // execute process
    attachInterrupt(digitalPinToInterrupt(LIGHTNING_INTERRUPT_PIN), process, RISING);
}

void loop() {
  /*if (digitalRead(LIGHTNING_INTERRUPT_PIN) == HIGH) {
      process();
  }*/

  delay(100); // Do nothing
}


void process() {
    noInterrupts();
    // Hardware has alerted us to an event, now we read the interrupt register
    // to see exactly what it is.
    intVal = lightning.readInterruptReg();
     switch (intVal) {
    case NOISE_INT:
        #ifdef DEBUG
        Serial.println("Noise.");
        #endif // DEBUG
        // Too much noise? Uncomment the code below, a higher number means better
        // noise rejection.
        // lightning.setNoiseLevel(setNoiseLevel);
        break;
        display.writeFillRect(0, 40, SCREEN_WIDTH, 8, 0);
        display.setCursor(0, 40);
        display.print("Noise ");
        display.println(++noiseCount);
        display.display();
    case DISTURBER_INT:
        #ifdef DEBUG
        Serial.println("Disturber.");
        #endif // DEBUG
        // Too many disturbers? Uncomment the code below, a higher number means better
        // disturber rejection.
        // lightning.watchdogThreshold(threshVal);
        break;
        display.writeFillRect(0, 56, SCREEN_WIDTH, 8, 0);
        display.setCursor(0, 56);
        display.print("Disturb ");
        display.println(++disturberCount);
        display.display();
    case LIGHTNING_INT:
        // Lightning! Now how far away is it? Distance estimation takes into
        // account any previously seen events in the last 15 seconds.
        byte distance = lightning.distanceToStorm();
        #ifdef DEBUG
        Serial.println("Lightning Strike Detected!");
        Serial.print("Approximately: ");
        Serial.print(distance);
        Serial.println("km away!");
        #endif // DEBUG
        display.writeFillRect(0, 16, SCREEN_WIDTH, 16, 0);
        display.setCursor(0, 16);
        display.print("Strike ");
        display.println(++strikeCount);
        display.setCursor(0, 24);
        display.print("Distance: ");
        display.print(distance);
        display.println("km");
        display.display();
        break;
    }
    interrupts();
}