//*--------------------------------------------------------------------------------------------------
//* VFOSystem VFO Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseño de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef VFOSystem_h
#define VFOSystem_h

//*--- Definition for VFO parameters and limits

#define VFOA    0
#define VFOB    1
#define VFOMAX  2

#define VFO_SHIFT       600000
#define VFO_START    144000000
#define VFO_END      147999000
#define VFO_STEP_10KHz   10000
#define VFO_STEP_5KHz     5000

#include "Arduino.h"
typedef void (*CALLBACK)(byte VFO);



typedef struct {
  byte ones;
  byte tens;
  byte hundreds;
  byte thousands;
  byte tenthousands;
  byte hundredthousands;
  byte millions;
 
} FSTR;
//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS
//*---------------------------------------------------------------------------------------------------
class VFOSystem
{
  public: 
  
      VFOSystem(CALLBACK c,CALLBACK t);
      void setVFO(byte VFO);
      void setVFOFreq(byte VFO,long int fVFO,long int shiftVFO,long int stepVFO,long int fMIN, long int fMAX);
      void getStr(byte VFO);
      void updateVFO(boolean CW, boolean CCW);
      long int getVFOFreq(byte VFO);
      void tx(boolean s);
      
      long int vfo[VFOMAX];
      
      
      long int vfoshift[VFOMAX];
      long int vfostep[VFOMAX];
      long int vfomin[VFOMAX];
      long int vfomax[VFOMAX];
      byte     vforpt[VFOMAX];
      byte     vfoAB=VFOA;
      FSTR     vfostr[VFOMAX];
      CALLBACK changeVFO=NULL;
      CALLBACK changeTX=NULL;
  
  private:
      boolean _boot;
      boolean _tx;
      long int _rxa[VFOMAX];

};

#endif

