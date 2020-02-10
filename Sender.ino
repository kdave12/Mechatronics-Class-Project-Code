String s = "000";
char mystr[4] = "000"; //String data
const int sampleWindow = 100; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() {
  // Begin the Serial at 9600 Baud
  Serial.begin(9600);
}

void loop() {
   unsigned long startMillis= millis();  // Start of sample window
   int peakToPeak = 0;   // peak-to-peak level

   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;

   // collect data for 100 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   //double volts = (peakToPeak * 5.0) / 1024.0;  // convert to volts
   s = String(peakToPeak);
   s.toCharArray(mystr,4);
   //Serial.println(s);
   Serial.write(mystr,4); //Write the serial data
}
