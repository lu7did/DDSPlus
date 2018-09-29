//*--------------------------------------------------------------------------------------------------
//* VFOSystem VFO Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef VFOSystem_h
#define VFOSystem_h

//*--- Definition for VFO parameters and limits

#define VFOA    0
#define VFOB    1
#define VFOMAX  2

#define VFO_STEP_1Hz            1
#define VFO_STEP_10Hz          10
#define VFO_STEP_100Hz        100
#define VFO_STEP_1KHz        1000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_100KHz    100000
#define VFO_STEP_1MHz     1000000


#include "Arduino.h"
typedef void (*CALLBACK)();

typedef struct {
  byte ones;
  byte tens;
  byte hundreds;
  byte thousands;
  byte tenthousands;
  byte hundredthousands;
  byte millions;
 
} FSTR;

//*============================================================================================
//* Define band limits
//*============================================================================================
//*---- HF band definition

#define BANDMAX 6


//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS
//*---------------------------------------------------------------------------------------------------
class VFOSystem
{
  public: 
  
      VFOSystem(CALLBACK c,CALLBACK t);
      
      void set(byte VFO,long int f);
      long int get(byte VFO);
      void setVFO(byte VFO);
      void setVFOFreq(byte VFO,long int fVFO);
      void setVFOShift(byte VFO,long int shiftVFO);
      void setVFOStep(byte VFO,long int stepVFO);
      void setVFOLimit(byte VFO,long int fMIN, long int fMAX);
      void setVFOBand(byte VFO,byte band);

      long int VFOSystem::code2step(byte b);
      byte VFOSystem::step2code(long int s);
      
      boolean isVFOChanged(byte VFO);
      boolean isVFOLocked();

      void resetVFO(byte VFO);
      //void resetVFOFreq(byte VFO);
      void computeVFO(long int f, FSTR* v);

      void getStr(byte VFO);
      void updateVFO(byte VFO, long int vstep);
      //long int getVFOFreq(byte VFO);
      //void tx(boolean s);
      
      long int vfo[VFOMAX];
      boolean lock;
      
      
      long int vfoshift[VFOMAX];
      long int vfostep[VFOMAX];
      byte     vfoband[VFOMAX];
      
      long int vfomin[VFOMAX];
      long int vfomax[VFOMAX];
      byte     vforpt[VFOMAX];
      byte     vfoAB=VFOA;
      void     swapVFO();
      
      FSTR     vfostr[VFOMAX];
      
      CALLBACK changeVFO=NULL;
      CALLBACK changeTX=NULL;
 

      long int loFreq[BANDMAX+1]={100,1800,3500,7000,14000,21000,28000};
      long int hiFreq[BANDMAX+1]={60000,2000,3800,7300,14350,21450,29700};
      //long int bandvfo[VFOMAX][BANDMAX+1];
      

  
  private:
      boolean _boot;
      boolean _tx;
      long int _rxa[VFOMAX];

};

#endif
//*---------------------------------------------------------------------------------------------------
//* VFOSystem CLASS Implementation
//*---------------------------------------------------------------------------------------------------
//*--------------------------------------------------------------------------------------------------
//* VFOSystem VFO Management Class   (CODE)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//*#include "VFOSystem.h"
VFOSystem::VFOSystem(CALLBACK c,CALLBACK t)
{
  changeVFO=NULL;
  _tx=false;
  lock=false;
  
  if (c!=NULL) {changeVFO=c;}  //* Callback of change VFO frequency
  if (t!=NULL) {changeTX=t;}   //* Callback of TX mode change
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO step
//*---------------------------------------------------------------------------------------------------
long int VFOSystem::get(byte b){
  
  if ((b<VFOA) && (b>VFOB)) {
     return vfo[VFOA];
  }
 
  return vfo[b];   
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO step
//*---------------------------------------------------------------------------------------------------
long int VFOSystem::code2step(byte b) {

       switch(b) {
         case 0:                   {return VFO_STEP_1Hz;}
         case 1:                   {return VFO_STEP_10Hz;}
         case 2:                   {return VFO_STEP_100Hz;}
         case 3:                   {return VFO_STEP_1KHz;}
         case 4:                   {return VFO_STEP_10KHz;}
         case 5:                   {return VFO_STEP_100KHz;}
         default:                  {return VFO_STEP_1MHz;}
     }    
  return VFO_STEP_1KHz;
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO step
//*---------------------------------------------------------------------------------------------------
byte VFOSystem::step2code(long int s) {

       switch(s) {
         case 1:                   {return 0;}
         case 10:                  {return 1;}
         case 100:                 {return 2;}
         case 1000:                {return 3;}
         case 10000:               {return 4;}
         case 100000:              {return 5;}
         default:                  {return 6;}
     }    
  return 3;
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Shift
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOShift(byte VFO,long int shiftVFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}

  vfoshift[VFO]=shiftVFO;
  return;
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Step
//*---------------------------------------------------------------------------------------------------
void VFOSystem::swapVFO() {
   if (vfoAB==VFOA) {vfoAB=VFOB;} else {vfoAB=VFOA;}
   return;
}

//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Step
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOStep(byte VFO,long int stepVFO) {
   
   if (VFO<VFOA || VFO>VFOB) { return;}

   vfostep[VFO]=stepVFO;
   return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Band
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOBand(byte VFO,byte band) {
   
   if (VFO<VFOA || VFO>VFOB) { return;}

   vfoband[VFO]=band;
   return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Band
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOLimit(byte VFO,long int fMIN,long int fMAX) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}

  vfomin[VFO]=fMIN;
  vfomax[VFO]=fMAX;
  return;
  
}
/*
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Frequency
//*---------------------------------------------------------------------------------------------------
void VFOSystem::resetVFOFreq(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  vfo[VFO]=_rxa[VFO];
  getStr(VFO);
  
  return;
}
*/
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Frequency
//*---------------------------------------------------------------------------------------------------
void VFOSystem::set(byte VFO,long int f) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  
  vfo[VFO]=f;
  _rxa[VFO]=f;
  
  if (changeVFO!=NULL) {changeVFO();}
  getStr(VFO);
  
  return;
  
}
boolean VFOSystem::isVFOLocked(){

  return lock;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO Frequency
//*---------------------------------------------------------------------------------------------------
void VFOSystem::resetVFO(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  
  _rxa[VFO]=vfo[VFO];
  
  return;
  
}
/*
//*---------------------------------------------------------------------------------------------------
//* Set the TX mode
//*---------------------------------------------------------------------------------------------------
void VFOSystem::tx(boolean s) {
  if (s!=_tx) {
     _tx=s;
     if (changeTX!=NULL) {changeTX();}
  }   
  
}
*/
//*---------------------------------------------------------------------------------------------------
//* Set the focus VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFO(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  vfoAB = VFO;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::getStr(byte VFO) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  
  long int fx=vfo[VFO];
  computeVFO(fx,&vfostr[vfoAB]);
  return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::computeVFO(long int f, FSTR* v) {
  
  (*v).millions =       int(f / 1000000);
  (*v).hundredthousands = ((f / 100000) % 10);
  (*v).tenthousands =     ((f / 10000) % 10);
  (*v).thousands =        ((f / 1000) % 10);
  (*v).hundreds =         ((f / 100) % 10);
  (*v).tens =             ((f / 10) % 10);
  (*v).ones =             ((f / 1) % 10);
  return; 
   
}
//*---------------------------------------------------------------------------------------------------
//* check if the VFO had changed
//*---------------------------------------------------------------------------------------------------
boolean VFOSystem::isVFOChanged(byte VFO) {

  if (_rxa[VFO] == vfo[VFO]) {return false;}
  return true;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::updateVFO(byte VFO,long int vstep) {

   vfo[VFO]=vfo[VFO]+vstep;

   if (vfo[VFO] > vfomax[VFO]) {
       vfo[VFO] = vfomax[VFO];
   } // UPPER VFO LIMIT
       
   if (vfo[VFO] < vfomin[VFO]) {
       vfo[VFO] = vfomin[VFO];
   } // LOWER VFO LIMIT

   getStr(VFO);

   if (vfo[VFO]!=_rxa[VFO]) {
       //_rxa[vfoAB]=vfo[vfoAB];
       if (changeVFO!=NULL) {
          changeVFO();}

   }

}



