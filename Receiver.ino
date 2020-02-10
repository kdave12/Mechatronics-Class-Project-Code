#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    20
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
int num = 20;
long startMillis = millis();
int timeDelay = 25;
int micReading = 0;
float flash = 0;

#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

char mystr[4]; //Initialized variable to store recieved data

void setup() {
  //delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  Serial.begin(9600);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}

void loop() {
  if(Serial.available()>0)
  {
    Serial.readBytes(mystr,4); //Read the serial data and store in var
    micReading = atoi(mystr);
    Serial.println(micReading); //Print data on Serial Monitor
  }
  flash = float(micReading)/600.0*100.0;
  //Serial.println(micReading);

//  static uint8_t startIndex = 0;
//  startIndex = startIndex + 1; /* motion speed */
//
//
//  //Serial.println(timeDelay);
//  //FillLEDsFromPaletteColors(startIndex);
  for (int x =0; x<num;x++)
  {
     leds[x] = CRGB::White; 
  }
   
  FastLED.show();

  delay(flash);
  for (int y =0; y<num;y++)
  {
     leds[y] = CRGB::Black; 
  } 
    
  FastLED.show();
  delay(flash);
}
