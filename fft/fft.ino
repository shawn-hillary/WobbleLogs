#include "arduinoFFT.h"
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

const uint16_t samples = 256; //This value MUST ALWAYS be a power of 2
const double signalFrequency = 2;
const double samplingFrequency = 10;
const uint8_t amplitude = 100;
double vReal[samples];
double vImag[samples];
double accelero_buffer[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03
Adafruit_MMA8451 mma = Adafruit_MMA8451();

void setup(void) {
  Serial.begin(9600); 
  Serial.println("Adafruit MMA8451 test!");
  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  mma.setRange(MMA8451_RANGE_2_G);




for(int s = 0;s<=samples;s++)
    {
    Serial.println("Getting Readings...");
    mma.read(); 
    sensors_event_t event; 
    mma.getEvent(&event);
    double x = event.acceleration.x;
    double y = event.acceleration.y;
    double z = event.acceleration.z;
    double wigglepower = (sqrt(x*x + y*y + z*z)-9.81); 
    accelero_buffer[s] = wigglepower;
    delay(10);
   // Serial.println(accelero_buffer[s]);
    }
} 

void loop()
{
  Serial.println("Readings Gotten");
  
  /* Build raw data */
  double cycles = (((samples-1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read
  for (uint16_t i = 0; i < samples; i++)
  {
   // vReal[i] = int8_t((amplitude * (sin((i * (twoPi * cycles)) / samples))) / 2.0);/* Build data with positive and negative values*/
    //vReal[i] = uint8_t((amplitude * (sin((i * (twoPi * cycles)) / samples) + 1.0)) / 2.0);/* Build data displaced on the Y axis to include only positive values*/
    vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }
  /* Print the results of the simulated sampling according to time */
 // Serial.println("Data:");
 // PrintVector(accelero_buffer, samples, SCL_TIME);
    FFT.Windowing(accelero_buffer, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
 // Serial.println("Weighed data:");
 // PrintVector(accelero_buffer, samples, SCL_TIME);
  FFT.Compute(accelero_buffer, vImag, samples, FFT_FORWARD); /* Compute FFT */
 // Serial.println("Computed Real values:");
 // PrintVector(accelero_buffer, samples, SCL_INDEX);
 // Serial.println("Computed Imaginary values:");
 // PrintVector(vImag, samples, SCL_INDEX);
  FFT.ComplexToMagnitude(accelero_buffer, vImag, samples); /* Compute magnitudes */
  Serial.println("Computed magnitudes:");
  PrintVector(accelero_buffer, (samples >> 1), SCL_FREQUENCY);
  double xx = FFT.MajorPeak(accelero_buffer, samples, samplingFrequency);
  Serial.println(xx, 6);
  while(1); /* Run Once */
  // delay(2000); /* Repeat after delay */
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
