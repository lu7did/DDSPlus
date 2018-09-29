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
#define VFO_BAND_START          0

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
void ShiftUpdate();
void VFOUpdate();
void StepUpdate();

void showBand();
void showDDS();
void setWord(byte* SysWord,byte v, boolean val);
boolean getWord (byte SysWord, byte v);

MenuClass band(BandUpdate);
MenuClass vfo(VFOUpdate);
MenuClass stp(StepUpdate);
MenuClass shf(ShiftUpdate);
MenuClass lck(NULL);
MenuClass mod(NULL);


//*--------------------------------------------------------------------------------------------
//* readEEPROM
//* Read specific configuration
//*--------------------------------------------------------------------------------------------
void doSetGroup() {
  return;
}

void handleSerialCommand() {


//**************** To be Implemented !!
  
}
/*
byte step2byte(long int step) {

   if (step==VFO_STEP_100Hz)  {return 0;}
   if (step==VFO_STEP_1KHz)   {return 1;}
   if (step==VFO_STEP_10KHz)  {return 2;}
   if (step==VFO_STEP_100KHz) {return 3;}
   return 4;
   
  
}
//*--------------------------------------------------------------------------------------------
//* readEEPROM
//* Read specific configuration
//*--------------------------------------------------------------------------------------------
long int byte2step(byte s) {

   if (s==0)  {return VFO_STEP_100Hz;}
   if (s==1)  {return VFO_STEP_1KHz;}
   if (s==2)  {return VFO_STEP_10KHz;}
   if (s==3)  {return VFO_STEP_100KHz;}
   return VFO_STEP_1MHz;
   
  
}
*/

//*--------------------------------------------------------------------------------------------
//* defineMenu
//* Define devide specific menu configuration
//*--------------------------------------------------------------------------------------------
void defineMenu(){
//*============================================================================================
//* Define master menu and lower level tree for simpleA
//*============================================================================================
  
  menuRoot.add((char*)"Band",&band);
  menuRoot.add((char*)"VFO",&vfo);
  menuRoot.add((char*)"Step",&stp);
  menuRoot.add((char*)"IF Shift",&shf);
  menuRoot.add((char*)"Lock",&lck);
  menuRoot.add((char*)"Mode",&mod);

  band.add((char*)"Off      ",NULL);
  band.set(0);

  shf.add((char*)" 0 Hz",NULL);
  shf.set(FI_LOW);

  vfo.add((char*)" A",NULL);
  vfo.set(VFOA);

//*---- Establish different and default step
  stp.add((char*)"  1 KHz",NULL);
  stp.set(3);

  lck.add((char*)"Off",NULL);
  lck.add((char*)"On ",NULL);

  mod.add((char*)"DDS",NULL);
  mod.add((char*)"VFO",NULL);
/*
  for (int i=0; i <= BANDMAX; i++){
      vx.bandvfo[VFOA][i]=vx.loFreq[i]*1000;
      vx.bandvfo[VFOB][i]=vx.loFreq[i]*1000;
  }
^*/
}
//*--------------------------------------------------------------------------------------------
//* pinSetup
//* Setup pin
//*--------------------------------------------------------------------------------------------
void pinSetup() {
  return;
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

 //*---- Resolve correct counter setup (recover FI shift from menu)
 if (freq > 0) {
    f2=(freq-shf.get())*4;
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
//*---------------------------------------------------------------------------------------------
//*
//*---------------------------------------------------------------------------------------------
void debugPrint(char* m){


#if DEBUG

   sprintf(hi,"<%s> vfoAB=%u vfo.get()=%u",m,vx.vfoAB,vfo.mItem);
   Serial.println(hi);

   sprintf(hi,"<%s> vfoband->[A]=%u  [B]=%u band.get()=%u %s %s",m,vx.vfoband[VFOA],vx.vfoband[VFOB],band.mItem,band.getText(0),band.getCurrentText());
   Serial.println(hi);

   sprintf(hi,"<%s> vfostep->[A]=%u  [B]=%u  stp.get()=%u",m,vx.vfostep[VFOA],vx.vfostep[VFOB],vx.step2code(stp.mItem));
   Serial.println(hi);

   sprintf(hi,"<%s> VFOA=%ld VFOB=%ld",m,vx.vfo[VFOA],vx.vfo[VFOB]);
   Serial.println(hi);

   sprintf(hi,"<%s> bandvfo[%u][%u]=%ld",m,vx.vfoAB,band.get(),vx.bandvfo[vx.vfoAB][vx.vfoband[vx.vfoAB]]);
   Serial.println(hi);

   Serial.println("--<eof>--");

#endif
   
   return;
  
}
//*--------------------------------------------------------------------------------------------
//* savesinpleA
//* save specifics of sinpleA
//*--------------------------------------------------------------------------------------------
void saveMenu() {

   if (vx.vfoAB != vfo.mItem) {   //Switch from VFO A to B or viceversa
      debugPrint("VFO Change Entry");
      
      vx.vfoAB=vfo.mItem;
      
      band.mItem=(vx.vfoband[vx.vfoAB]);
      BandUpdate();
      
      stp.mItem=(vx.step2code(vx.vfostep[vx.vfoAB]));
      StepUpdate();
      
      vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);

      debugPrint("VFO Change Exit");
      //vx.vfo[vx.vfoAB]=vx.loFreq[band.mItem]*1000;  //---> Revisar luego si debe volver al comienzo o poner un valor en memoria
       
   } else {

      if (vx.vfoband[vx.vfoAB]!=band.mItem) { //Switch Band
         debugPrint("BAND Change Entry"); 
         //vx.bandvfo[vx.vfoAB][vx.vfoband[vx.vfoAB]]=vx.vfo[vx.vfoAB];
         vx.vfoband[vx.vfoAB]=band.mItem;
         vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);
       
            if ((vx.vfo[vx.vfoAB]>=vx.loFreq[band.mItem]*1000) && (vx.vfo[vx.vfoAB]<=vx.hiFreq[band.mItem]*1000)) {

            } else {
              vx.vfo[vx.vfoAB]=vx.loFreq[band.mItem]*1000;  //* solo altera la frecuencia si resulta que es distinto
            }
          vx.vfo[vx.vfoAB]=vx.loFreq[vx.vfoband[vx.vfoAB]]*1000;
          debugPrint("BAND Change Exit"); 
   
    }

         if (vx.vfostep[vx.vfoAB]!=vx.code2step(stp.mItem)) { //Change tuning step
            debugPrint("STEP Change Entry");
            vx.vfostep[vx.vfoAB]=vx.code2step(stp.mItem);
            debugPrint("STEP Change Exit"); 
         }
   }
   

}
//*--------------------------------------------------------------------------------------------
//* Band Update   (CALLBACK from Menu System)
//* Set band label and limits
//*--------------------------------------------------------------------------------------------
void BandUpdate() {

  char* s=(char*)"                  ";

  //sprintf(hi,"BandUpdate ANTES band.mItem=%u %s",band.mItem,band.l.get(0)->mText);
  //Serial.println(hi);
  
  if (band.mItem < BANDMAX && band.CW == true) {
      band.mItem++;
  }
  if (band.mItem > 0 && band.CCW == true) {
      band.mItem--;
  }
   
  
  switch(band.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"160m";break;};
    case 2:                          {s=(char*)"80m";break;}; 
    case 3:                          {s=(char*)"40m";break;}; 
    case 4:                          {s=(char*)"20m";break;}; 
    case 5:                          {s=(char*)"15m";break;}; 
    default:                         {s=(char*)"10m";break;}; 
  }

  //vx.vfo[vx.vfoAB]=vx.loFreq[band.mItem]*1000; 
  //vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);

  band.l.get(0)->mText=s;
  band.CW=false;
  band.CCW=false;
  
  //showPanel();
  //sprintf(hi,"BandUpdate AFTER band.mItem=%u %s",band.mItem,band.l.get(0)->mText);
  //Serial.println(hi);
  
  return;
  
}
//*--------------------------------------------------------------------------------------------
//* FI Shift Update   (CALLBACK from Menu System)
//* Set FI shift label and limits
//*--------------------------------------------------------------------------------------------
void ShiftUpdate() {
  
  if (shf.mItem < 15 && shf.CW == true) {
      shf.mItem++;
  }
  if (shf.mItem > 0 && shf.CCW == true) {
      shf.mItem--;
  }
  char* s=(char*)"                  "; 


  sprintf(s,"%i KHz",shf.mItem);
      
  shf.l.get(0)->mText=s;
  shf.CW=false;
  shf.CCW=false;
  
  //showPanel();
  return;
}
//*--------------------------------------------------------------------------------------------
//* FI Shift Update   (CALLBACK from Menu System)
//* Set FI shift label and limits
//*--------------------------------------------------------------------------------------------
void VFOUpdate() {

  char* s=(char*)"  "; 

  if (vfo.mItem < VFOB && vfo.CW == true) {
      vfo.mItem++;
  }
  if (vfo.mItem > VFOA && vfo.CCW == true) {
      vfo.mItem--;
  }

  if (vfo.mItem==VFOA){s="A";}
  if (vfo.mItem==VFOB){s="B";}
  
  vfo.l.get(0)->mText=s;
  
  vfo.CW=false;
  vfo.CCW=true;
  
  return;
}
void LckUpdate(){
  char* s=(char*)"    "; 

  if (lck.mItem < 1 && lck.CW == true) {
      lck.mItem++;
  }
  if (lck.mItem > 0 && lck.CCW == true) {
      lck.mItem--;
  }
  switch(lck.mItem) {
    case 0:                          {s=(char*)"Off";vx.lock=false;break;};     
    default:                         {s=(char*)"On";vx.lock=true;break;};  
  }
  
  lck.l.get(0)->mText=s;
  lck.CW=false;
  lck.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* FI Shift Update   (CALLBACK from Menu System)
//* Set FI shift label and limits
//*--------------------------------------------------------------------------------------------
void StepUpdate() {

  char* s=(char*)"            "; 

  if (stp.mItem < 6 && stp.CW == true) {
      stp.mItem++;
  }
  if (stp.mItem > 0 && stp.CCW == true) {
      stp.mItem--;
  }
  switch(stp.mItem) {
    case 0:                          {s=(char*)"   1 Hz";break;};                            
    case 1:                          {s=(char*)"  10 Hz";break;};
    case 2:                          {s=(char*)" 100 Hz";break;}; 
    case 3:                          {s=(char*)"  1 KHz";break;}; 
    case 4:                          {s=(char*)" 10 KHz";break;}; 
    case 5:                          {s=(char*)"100 KHz";break;}; 
    default:                         {s=(char*)"  1 MHz";break;}; 
  }

  stp.l.get(0)->mText=s;
  stp.CW=false;
  stp.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* readEEPROM
//* Read specific configuration
//*--------------------------------------------------------------------------------------------
void readEEPROM(){

#if DEBUG 
     sprintf(hi,"EEPROM initialization Band(%i)",EEPROM.read(31));
     Serial.println(hi);
#endif

 
     return;
}    
//*--------------------------------------------------------------------------------------------
//* showBand
//* show Band setting at the display
//*--------------------------------------------------------------------------------------------
void showBand() {

  lcd.setCursor(6,0);
  lcd.print(String(band.getCurrentText()));
 
};      
//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------

void writeEEPROM() {
  
  
  EEPROM.write(18,shf.get());
  
  
  return;
}
//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------
void showGUI(){
      showBand();
      showDDS();
      return;
}
//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------
void checkBandLimit(){

#if DEBUG
  sprintf(hi,"setup band=%i",band.get());
  Serial.println(hi);
  
  sprintf(hi,"entry with VFOA=%ld VFOB=%ld",vx.vfo[VFOA],vx.vfo[VFOB]);
  Serial.println(hi);
#endif

  band.mItem=vx.vfoband[vx.vfoAB];
  stp.mItem=vx.step2code(vx.vfostep[vx.vfoAB]);
  
  if (vx.get(VFOA)<vx.loFreq[band.get()]*1000 || vx.get(VFOA)>vx.hiFreq[band.get()]*1000) {vx.set(VFOA,vx.loFreq[band.get()]*1000);}
  if (vx.vfo[VFOB]<vx.loFreq[band.get()]*1000 || vx.vfo[VFOB]>vx.hiFreq[band.get()]*1000) {vx.set(VFOB,vx.loFreq[band.get()]*1000);}

  BandUpdate();
  StepUpdate();

#if DEBUG
  sprintf(hi,"exit checkBandLimit with VFOA=%ld VFOB=%ld",vx.vfo[VFOA],vx.vfo[VFOB]);
  Serial.println(hi);
#endif
  
  return;
}

//*-----------------------------------------------------------------------------------------------------------------------------------
//* setFrequencyHook
//* Device specific frequency display
//*-----------------------------------------------------------------------------------------------------------------------------------
void setFrequencyHook(long int f,FSTR* v) {

    long int fDDS=f/1000;

#if DEBUG  
  sprintf(hi,"Frequency %ld",f);
  Serial.println(hi);
  sprintf(hi,"FrequencyDDS %ld",fDDS);
  Serial.println(hi);
#endif
    
//*******************************
//* Setup DDS Frequency         *
//*******************************
  //*---- Set DDS with new frequency
  setDDSfreq(fDDS);

  if (stp.get()<3) {
     lcd.print(" ");
     lcd.print((*v).hundreds);
     lcd.print((*v).tens);
  }

  if (stp.get()==0){
     lcd.print((*v).ones);
  }

  //*---- In current band and vfo store last used frequency
  //vx.bandvfo[vx.vfoAB][band.get()]=f/1000;
  
}

void setSysOM(){
  
  setWord(&USW,CONX,false);

//===================================================================================
//* SI5351 DDS  module initialization and setup
//=================================================================================== 
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

void handleTimerHook(){

}

void showVFOHook(){
  lcd.setCursor(0,0);
  lcd.print("Vfo");
  if (vfo.get()==VFOA){lcd.print("A");} else {lcd.print("B");}
  //lcd.print("");
  return;
}


#endif

