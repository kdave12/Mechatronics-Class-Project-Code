//generalized wave freq detection with 38.5kHz sampling rate and interrupts
//Shuming Teoh
//Code background by Amanda Ghassaei
//https://www.instructables.com/id/Arduino-Frequency-Detection/
//12/11/2018

#include <FastLED.h>
#define LED_PIN     5
#define NUM_LEDS    63
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
int num = 63; //number of LEDs used 
long startMillis = millis(); 
int timeDelay = 8; //delay variable used 
int micReading = 0; //mic reading variable
float flash = 0;
int r = 0; //red color number
int g = 0; //green color number
int b = 0; // blue color number

//////////////////////////////////////////

//clipping indicator variables
boolean clipping = 0;

//data storage variables
byte newData = 0;
byte prevData = 0;
unsigned int time = 0;//keeps time and sends vales to store in timer[] occasionally
int timer[10];//storage for timing of events
int slope[10];//storage for slope of events
unsigned int totalTimer;//used to calculate period
unsigned int period;//storage for period of wave
byte index = 0;//current storage index
float frequency;//storage for frequency calculations
int maxSlope = 0;//used to calculate max slope as trigger point
int newSlope;//storage for incoming slope data

//variables for decided whether you have a match
byte noMatch = 0;//counts how many non-matches you've received to reset variables if it's been too long
byte slopeTol = 4;//slope tolerance- adjust this if you need
int timerTol = 6;//timer tolerance- adjust this if you need
float oldPeriod = 0;
bool dir = true;
int current = millis(); //timer 
int prev = 0;

#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() {
  Serial.begin(9600);
  
  cli();//diable interrupts
  
  //set up continuous sampling of analog pin 0 at 38.5kHz
 
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  sei();//enable interrupts
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}


ISR(ADC_vect) {//when new ADC value ready
  
  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 53 && newData >=53){//if increasing and crossing midpoint (range modified to fit the readings of the arduino)
    newSlope = newData - prevData;//calculate slope
    if (abs(newSlope-maxSlope)<slopeTol){//if slopes are == record new data and reset time
      slope[index] = newSlope;
      timer[index] = time;
      time = 0;
      if (index == 0){//new max slope just reset
        noMatch = 0;
        index++;//increment index
      } else if (abs(timer[0]-timer[index])<timerTol && abs(slope[0]-newSlope)<slopeTol){//if timer duration and slopes match
        //sum timer values
        totalTimer = 0;
        for (byte i=0;i<index;i++){
          totalTimer+=timer[i];
        }
        period = totalTimer;//set period
        //reset new zero index values to compare with
        timer[0] = timer[index];
        slope[0] = slope[index];
        index = 1;//set index to 1
        noMatch = 0;
      }
      else{//crossing midpoint but not match
        index++;//increment index
        if (index > 9){
          reset();
        }
      }
    }
    else if (newSlope>maxSlope){//if new slope is much larger than max slope
      maxSlope = newSlope;
      time = 0;//reset clock
      noMatch = 0;
      index = 0;//reset index
    }
    else{//slope not steep enough
      //Serial.print("hi");
      noMatch++;//increment no match counter
      if (noMatch>9){
        reset();
      }
    }
  }
  
  if (newData == 0 || newData == 105){//if clipping
    clipping = 1;//currently clipping
  }
  
  time++;//increment timer at rate of 38.5kHz
}

void reset(){//clear out some variables
  index = 0;//reset index
  noMatch = 0;//reset match couner
  maxSlope = 0;//reset slope
}


void checkClipping(){//manage clipping indicator LED
  if (clipping){//if currently clipping
    clipping = 0;
  }
}

void loop() {
  checkClipping();
  frequency = 38462.f/float(period);//calculate frequency timer rate/period
  if(frequency>999) //if over 1000, ignore since range is too big
  {
    period = oldPeriod;
    frequency = 38462.f/float(period);
  }
  else{
    oldPeriod = period;
    //Serial.println(int(frequency));
    //Serial.println(" hz");
  }

//conditionals to set the LED colors
  if(frequency>0 && frequency<100){
    r=66;
    g=134;
    b =244; //blue
  }else if(frequency>=100 && frequency<200){
    r=41;
    g=216;
    b = 39; //green
  }else if(frequency>=200 && frequency<300){
    r=156;
    g=79;
    b = 239; //purple
  }else if(frequency>=300 && frequency<400){
    r=249;
    g=27;
    b =71; //red
  }else if(frequency>=400 && frequency<500){
    r=228;
    g=64;
    b = 237; //magenta
  }else if(frequency>=500 && frequency<600){
    r=255;
    g=145;
    b =28; //orange
  }else if(frequency>=600 && frequency<700){
    r=252;
    g=245;
    b = 30; //yellow
  }else if(frequency>=700 && frequency<800){
    r=174;
    g=242;
    b = 48; //lime
  }else if(frequency>=800 && frequency<900){
    r=43;
    g=242;
    b = 242; //cyan
  }else if(frequency>=900 && frequency<1000){
    r=140;
    g=104;
    b = 5; //brown
  }else{
    r=255;
    g=255;
    b =255;
  }
  Serial.println(frequency);

  current = millis();
  //conditionals to change delay within the range set below
  if(timeDelay == 7)
    {
      dir = true; //reaches bottom of range
    }
  else if(timeDelay == 10)
  {
    dir = false; //reaches top of the range
  }

//every 1.5 seconds change the delay
  if((current-prev)>1500)
  {
    if(dir)
    {
      timeDelay+=1;
    }
    else if(!dir)
    {
      timeDelay-=1;
    }
    prev = current;
  }
  //Serial.println(timeDelay);
  
  //Serial.println(frequency);
  for (int x =0; x<num;x++)
  {
     //leds[x].setRGB(30,245,30); 
     leds[x].setRGB(r, g, b); 
  }
   
  FastLED.show();//turn LEDs on

  delay(timeDelay);
  for (int y =0; y<num;y++)
  {
     leds[y] = CRGB::Black; 
  } 
    
  FastLED.show();//turn LEDS off
  delay(timeDelay);
}
