#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  // init the lights
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  // Get serial ready for debugging
  // This can probably go when it's time for Gemma
  Serial.begin(9600);
  
  // Check for the accelerometer
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  
}

void loop() {

  // Set some defaults
  strip.setBrightness(64); // 0-255
  int mode = 1; // 0 = rainbowSweep, 1 = compassColor, 2 = stepSplash, 3 = speedSweep, 4 = Serial debug
  /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);
  mag.getEvent(&event);
  
  // Stuff some variables we might call on later depending on the various modes
  double xAccel = event.acceleration.x;
  double yAccel = event.acceleration.y;
  double zAccel = event.acceleration.z;
  
  float Pi = 3.14159;
  float heading = (atan2(event.magnetic.y,event.magnetic.x) * 180) / Pi;
  
  // Handle the various modes
  switch (mode) {
  case 0:    // Rainbow cycle
    rainbowCycle(20, 2);
    break;
  case 1:    // Compass color
    compassColor(heading);
    break;
  case 2:    // Splash on step
    splashStep(zAccel, strip.Color(127, 127, 127));
    break;
  case 3:    // Sweep speed changes on skate speed
    // TODO
    break;
  case 4:
    sensorMonitor();
  } 

}

void compassColor (float heading) {
  // Normalize to 0-360
  if (heading < 0)
  {
    heading = 360 + heading;
  }
  Serial.print("Compass Heading: ");
  Serial.println(heading);
//  Serial.print("Byte Value: ");
//  Serial.println(ByteHeading(heading));
//  Serial.print("Wheel Value: ");
//  Serial.println(Wheel(ByteHeading(heading)));

    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(ByteHeading(heading)));
    }
    strip.show();
}

void splashStep(double zAccel, uint32_t color) {
  if (zAccel < 3) {
    fadeDown(64, 0, 64, color);
    Serial.println(" Fading!");
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
// Split specifies how many cycles are required to display the full rainbow
// i.e. 2 means 1/2 of the rainbow will display at a time
void rainbowCycle(uint8_t wait, int split) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / split / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Take a compass heading and return a 0-255 value. 
// Handy for sending to the Wheel() func to get back a color
uint32_t ByteHeading(uint32_t Heading) {
  float adj = 255.0/360.0;
  return (unsigned int) (Heading * adj);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void fadeDown(uint32_t highVal, uint32_t lowVal, uint8_t rate, uint32_t color) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.setBrightness(highVal);
  strip.show();
  for(int i = highVal; i > lowVal; i--) {
    strip.setBrightness(i);
    strip.show();
    delay(rate);
  }
}

void sensorMonitor () {
  sensors_event_t event; 
  accel.getEvent(&event);
  mag.getEvent(&event);
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
  Serial.print("Sum: "); Serial.print(event.acceleration.x + event.acceleration.y + event.acceleration.z); Serial.print("  ");
  Serial.println("m/s^2 ");
  delay(100);
}
