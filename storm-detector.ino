#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include "SparkFun_AS3935.h"
#include "Adafruit_SSD1306.h"
#include "splash.h"

//#define DEBUG

// Button
#define BUTTON 4
#define ROTARY_INTERRUPT 2
#define STEP   5

//SPI chip select pin
#define CHIP_SELECT 10

// Detector mode
#define INDOOR 0x12 
#define OUTDOOR 0xE

// Type of signal
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04
#define NOISE_INT     0x01

#define PAGE_MAIN        1
#define PAGE_MODE        2
#define PAGE_NOISE       3
#define PAGE_WATCHDOG    4
#define PAGE_REJECTIONS  5
#define PAGE_LIGHTNING   6

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
bool modeOut             = true;
byte noise                = 2; // Value between 1-7
byte watchDogVal          = 2; // Value between 1-10
byte spike                = 2; // Value between 1-11
byte lightningThresh[4]   = {1, 5, 9, 16}; // Value in [1, 5, 9, 16]
byte lightningThreshIndex = 0;

// This variable holds the number representing the lightning or non-lightning
// event issued by the lightning detector.
byte intVal      = 0;

byte distance;

byte page = 1;

bool buttonUp = true;

void setup() {
    loadConfig();
    
    Wire.begin();

    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(ROTARY_INTERRUPT, INPUT_PULLUP);
    pinMode(STEP, INPUT_PULLUP);

    // When lightning is detected the interrupt pin goes HIGH.
    //pinMode(LIGHTNING_INTERRUPT_PIN, INPUT);

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
    lightning.setIndoorOutdoor(modeOut ? OUTDOOR: INDOOR);

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
    lightning.lightningThreshold(lightningThresh[lightningThreshIndex]); 

    // When signal is raising on interrupt pin (when a lighting occurs),
    // execute process
    attachInterrupt(digitalPinToInterrupt(LIGHTNING_INTERRUPT_PIN), process, RISING);

    attachInterrupt(digitalPinToInterrupt(ROTARY_INTERRUPT), rotation, FALLING);

    page = 0;
}

void loop() {
    /*if (digitalRead(LIGHTNING_INTERRUPT_PIN) == HIGH) {
        process();
    }*/

    if ((digitalRead(BUTTON) == HIGH) && buttonUp) {
        buttonUp = false;
        page++;
        if (page>6) {
            saveConfig();
            page=1;
        }
    }

    if (digitalRead(BUTTON) == LOW) {
        buttonUp = true;
    }

    switch(page) {
    default:
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LIGHTNINGS - AS3935 ");
        display.setCursor(0, 16);
        display.print("Strike: ");
        display.println(strikeCount);
        display.setCursor(0, 24);
        display.print("Distance: ");
        display.print(distance);
        display.println("km");

        display.setCursor(0, 40);
        display.print("Noise: ");
        display.println(noiseCount);

        display.setCursor(0, 56);
        display.print("Disturb: ");
        display.println(disturberCount);
        display.display();
        break;

    case PAGE_MODE:
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LIGHTNINGS - MODE ");
        display.setCursor(0, 16);
        display.print("Mode: ");
        if (modeOut) {
            display.println("Outdoor");
        } else {
            display.println("Indoor");
        }
        display.display();
        break;

    case PAGE_NOISE:
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LIGHTNINGS - NOISE ");
        display.setCursor(0, 16);
        display.print("Noise: ");
        display.println(noise);
        display.display();
        break;

    case PAGE_WATCHDOG:
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LIGHTNINGS - WATCHDOG ");
        display.setCursor(0, 16);
        display.print("Watchdog: ");
        display.println(watchDogVal);
        display.display();
        break;

    case PAGE_REJECTIONS:
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LIGHTNINGS - REJECT ");
        display.setCursor(0, 16);
        display.print("Spike reject: ");
        display.println(spike);
        display.display();
        break;

    case PAGE_LIGHTNING:
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LIGHTNINGS - THRESHOL ");
        display.setCursor(0, 16);
        display.print("Threshold: ");
        display.println(lightningThresh[lightningThreshIndex]);
        display.display();
        break;
    }

    delay(100);
}

void rotation() {
    noInterrupts();
    bool dir = (digitalRead(STEP) == HIGH);

    switch(page) {
    case PAGE_MODE:
        modeOut = !modeOut;
        lightning.setIndoorOutdoor(modeOut ? OUTDOOR: INDOOR);
        break;

    case PAGE_NOISE:
        if (dir) {
            noise++;
            if (noise>7) {
                noise=1;
            }
        } else {
            noise--;
            if (noise<1) {
                noise = 7;
            }
        }
        lightning.setNoiseLevel(noise);
        break;

    case PAGE_WATCHDOG:
        if (dir) {
            watchDogVal++;
            if (watchDogVal>10) {
                watchDogVal = 1;
            }
        } else {
            watchDogVal--;
            if (watchDogVal<1) {
                watchDogVal = 10;
            }
        }
        lightning.watchdogThreshold(watchDogVal);
        break;

    case PAGE_REJECTIONS:
        if (dir) {
            spike++;
            if (spike>11) {
                spike = 1;
            }
        } else {
            spike--;
            if (spike<1) {
                spike = 11;
            }
        }
        lightning.spikeRejection(spike);
        break;

    case PAGE_LIGHTNING:
        if (dir) {
            lightningThreshIndex++;
            if (lightningThreshIndex>3) {
                lightningThreshIndex = 0;
            }
        } else {
            lightningThreshIndex--;
            if (lightningThreshIndex<0) {
                lightningThreshIndex = 3;
            }
        }
        lightning.lightningThreshold(lightningThresh[lightningThreshIndex]); 
        break;
    }
    delay(100);
    interrupts();
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
        noiseCount++;
    case DISTURBER_INT:
        #ifdef DEBUG
        Serial.println("Disturber.");
        #endif // DEBUG
        // Too many disturbers? Uncomment the code below, a higher number means better
        // disturber rejection.
        // lightning.watchdogThreshold(threshVal);
        break;
        disturberCount++;
    case LIGHTNING_INT:
        // Lightning! Now how far away is it? Distance estimation takes into
        // account any previously seen events in the last 15 seconds.
        distance = lightning.distanceToStorm();
        #ifdef DEBUG
        Serial.println("Lightning Strike Detected!");
        Serial.print("Approximately: ");
        Serial.print(distance);
        Serial.println("km away!");
        #endif // DEBUG
        strikeCount++;
        break;
    }
    interrupts();
}

void saveConfig() {
    EEPROM.write(0x00, modeOut ? 1:0);
    EEPROM.write(0x01, noise);
    EEPROM.write(0x02, watchDogVal);
    EEPROM.write(0x03, spike);
    EEPROM.write(0x04, lightningThreshIndex);
}

void loadConfig() {
    modeOut = (EEPROM.read(0x00)==1);

    noise = EEPROM.read(0x01);
    if ((noise<1) || (noise>7)) {
        noise = 2;
    }

    watchDogVal = EEPROM.read(0x02);
    if ((watchDogVal<1) || (watchDogVal>10)) {
        watchDogVal = 2;
    }

    spike = EEPROM.read(0x03);
    if ((spike<1) || (spike>11)) {
        spike = 2;
    }

    lightningThreshIndex = EEPROM.read(0x04);
    if (lightningThreshIndex>3) {
        lightningThreshIndex = 0;
    }
}