#if SINPLEA
//*--------------------------------------------------------------------------------------------------
//* Specific Headers for sinpleA implementation
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//*--- Program & version identification


#define PROGRAMID "sinpleA"
#define PROG_VERSION   "1.0"

//*-----------------------------------------------------------------------------------------------
//* Control lines and VFO Definition [Project dependent]
//*-----------------------------------------------------------------------------------------------

#define VFO_SHIFT            1000
#define VFO_START          100000
#define VFO_END          60000000

#define VFO_STEP_100Hz        100
#define VFO_STEP_1KHz        1000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_100KHz    100000
#define VFO_STEP_1MHz     1000000

//#define VFO_FI_SHIFT           12
#define VFO_FI_SHIFT            0
#define VFO_PLL_LOWER        1000

//*=======================================================================================================================================================
//* SI5351 Library
//*=======================================================================================================================================================
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_SI5351.h>

Adafruit_SI5351 clkVFO = Adafruit_SI5351();

//*------------------------------------------------------------------------------------------------
//* Set here SINPLEA Menu definitions
//*------------------------------------------------------------------------------------------------

void BandUpdate();
MenuClass band(BandUpdate);
MenuClass vfo(NULL);
MenuClass stp(NULL);

//*============================================================================================
//* Define band limits
//*============================================================================================
//*---- HF band definition

#define BANDMAX 11

  long int loFreq[]={100,1800,3500,5350,7000,10000,14000,18068,21000,24890,28000,50000};
  long int hiFreq[]={60000,2000,3800,5370,7300,10150,14350,18168,21450,24990,29700,54000};
  long int bandvfo[BANDMAX+1];

void definesinpleAmenu(){
//*============================================================================================
//* Define master menu and lower level tree for simpleA
//*============================================================================================
  
  menuRoot.add((char*)"Band",&band);
  menuRoot.add((char*)"VFO",&vfo);
  menuRoot.add((char*)"Step",&stp);

  band.add((char*)"Off      ",NULL);
  band.set(0);

  vfo.add((char*)"A",NULL);
  vfo.add((char*)"B",NULL);

  stp.add((char*)"100  Hz",NULL);
  stp.add((char*)" 1  KHz",NULL);
  stp.add((char*)"10  KHz",NULL);
  stp.add((char*)"100 KHz",NULL);
  stp.add((char*)"  1 MHz",NULL);

  for (int i=0; i <= BANDMAX; i++){
      bandvfo[i]=loFreq[i];
  }

}


//*==============================================================================================================
//* Set DDS Frequency fur SI5351 
//* Receive frequency to set expressed in KHz
//*==============================================================================================================
void setDDSfreq (unsigned long freq)
{

 //*---- trace code make into DEBUG later

#if DEBUG 
 sprintf(hi,"DDS frequency= %ld",freq);
 Serial.println(hi);
#endif
 
 
 unsigned long f2;
 unsigned long f3;
 unsigned long f4;
 unsigned long f5;
 unsigned long div2;
 unsigned int Divider2;
 unsigned int rdiv;

 //*---- Resolve correct counter setup (assumes a 12 KHz FI)
 if (freq > 0) {
    f2=(freq-VFO_FI_SHIFT)*4;
    if (f2<VFO_PLL_LOWER) {
       rdiv = 16;
       f2 = f2 * 16;
    }  else {
       rdiv = 1;
    }

 //*---- Set DDS divisor
 
 div2 = 900000000/f2;
 f4 = div2/1000;
 f5=div2-(f4*1000);
 clkVFO.setupMultisynth(1, SI5351_PLL_A, f4, f5,VFO_PLL_LOWER);

//*--- Set additional divisor
 
 if (rdiv == 16) {
    clkVFO.setupRdiv(1, SI5351_R_DIV_16);
 }
 if (rdiv == 1) {
    clkVFO.setupRdiv(1, SI5351_R_DIV_1);
 }
}
}
#endif
#if SINPLEA
//*--------------------------------------------------------------------------------------------
//* savesinpleA
//* save specifics of sinpleA
//*--------------------------------------------------------------------------------------------
void savesinpleA() {

     switch(stp.get()) {
         case 0:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_100Hz;break;}
         case 1:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_1KHz;break;}
         case 2:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_10KHz;break;}
         case 3:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_100KHz;break;}
         default:                  {vx.vfostep[vx.vfoAB]=VFO_STEP_1MHz;break;}
     }    
  
}
#endif

