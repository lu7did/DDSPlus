//*--------------------------------------------------------------------------------------------------
//* VFOSystem VFO Management Class   (CODE)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseño de VFO para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#include "VFOSystem.h"
VFOSystem::VFOSystem(CALLBACK c,CALLBACK t)
{
  changeVFO=NULL;
  _tx=false;
  
  if (c!=NULL) {changeVFO=c;}  //* Callback of change VFO frequency
  if (t!=NULL) {changeTX=t;}   //* Callback of TX mode change
 
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::setVFOFreq(byte VFO,long int fVFO,long int shiftVFO,long int stepVFO,long int fMIN,long int fMAX) {
  
  if (VFO<VFOA || VFO>VFOB) { return;}
  
  vfo[VFO]=fVFO;
  _rxa[VFO]=fVFO;
  
  vfoshift[VFO]=shiftVFO;
  vfostep[VFO]=stepVFO;
  vfomin[VFO]=fMIN;
  vfomax[VFO]=fMAX;
  getStr(VFO);
  
  return;
  
}
//*---------------------------------------------------------------------------------------------------
//* Set the TX mode
//*---------------------------------------------------------------------------------------------------
void VFOSystem::tx(boolean s) {
  if (s!=_tx) {
     _tx=s;
     if (changeTX!=NULL) {changeTX(vfoAB);}
  }   
  
}
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

  vfostr[VFO].millions =       int(fx / 1000000);
  vfostr[VFO].hundredthousands = ((fx / 100000) % 10);
  vfostr[VFO].tenthousands =     ((fx / 10000) % 10);
  vfostr[VFO].thousands =        ((fx / 1000) % 10);
  vfostr[VFO].hundreds =         ((fx / 100) % 10);
  vfostr[VFO].tens =             ((fx / 10) % 10);
  vfostr[VFO].ones =             ((fx / 1) % 10);
  
}
//*---------------------------------------------------------------------------------------------------
//* Get the frequency applying for the rpt and tx mode
//*---------------------------------------------------------------------------------------------------
long int VFOSystem::getVFOFreq(byte VFO) {
    if (VFO<VFOA || VFO>VFOB) { return vfo[vfoAB];}
    
    switch(vforpt[VFO]) {

    case 0 : {
              return vfo[VFO];
              break;
             }
    case 1 : {
              return vfo[VFO]+vfoshift[VFO];
              break;
             }
    case 2 : {
              return vfo[VFO]-vfoshift[VFO];
              break;
             }
    
  }
  return vfo[VFO];
}
//*---------------------------------------------------------------------------------------------------
//* Set the parameters of a given VFO
//*---------------------------------------------------------------------------------------------------
void VFOSystem::updateVFO(boolean CW, boolean CCW) {

  
  if (CW==true) {
       vfo[vfoAB] = vfo[vfoAB] + vfostep[vfoAB];
   }
   
   if (CCW==true) {    
       vfo[vfoAB] = vfo[vfoAB] - vfostep[vfoAB];
   }  
       
   if (vfo[vfoAB] > vfomax[vfoAB]) {
       vfo[vfoAB] = vfomax[vfoAB];
   } // UPPER VFO LIMIT
       
   if (vfo[vfoAB] < vfomin[vfoAB]) {
       vfo[vfoAB] = vfomin[vfoAB];
   } // LOWER VFO LIMIT

   if (vfo[vfoAB]!=_rxa[vfoAB]) {
       _rxa[vfoAB]=vfo[vfoAB];
       if (changeVFO!=NULL) {changeVFO(vfoAB);}

   }

}

