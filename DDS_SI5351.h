//*--------------------------------------------------------------------------------------------------
//* DDS_SI5351 Version 1.0  Library to handle DDS SI5351
//*--------------------------------------------------------------------------------------------------
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#if SI5351

#include <Adafruit_SI5351.h>
Adafruit_SI5351 clkVFO = Adafruit_SI5351();

//*==============================================================================================================
//* Set DDS Frequency fur SI5351 
//* Receive frequency to set expressed in KHz
//*==============================================================================================================
void setDDSFreq () {
 
 unsigned long f2;
 unsigned long f3;
 unsigned long f4;
 unsigned long f5;
 unsigned long div2;
 unsigned int Divider2;
 unsigned int rdiv;

//*---- Resolve correct counter setup (recover FI shift from menu)

 unsigned long FI=0;

//*----- FI Displacement

 if (mod.get()>0) {
    FI=shf.get()*4;
 } 

//*---- Set DDS with new frequency
 long int fDDS=vx.get(vx.vfoAB)/1000;

#if DEBUG 
 sprintf(hi,"DDS frequency= %ld FI=%ld mod(%d) shf.get=%d",fDDS,FI,mod.get(),shf.get());
 Serial.println(hi);
#endif

 if (fDDS > 0) {
  
    f2=(fDDS-FI);
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
//-----------------------------------------------------------------------------------
//* SI5351 DDS  module initialization and setup
//----------------------------------------------------------------------------------- 
void DDSInit(){ 

  if (clkVFO.begin() != ERROR_NONE)
  {
     setWord(&USW,CONX,false);
  } else {
     setWord(&USW,CONX,true);    
  }
  
  clkVFO.enableOutputs(true);
  clkVFO.setupPLL(SI5351_PLL_A, 36, 0, 1000); //900 MHz 
  return;
  
}
#endif

