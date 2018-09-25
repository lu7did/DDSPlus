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
//#define VFO_FI_SHIFT            0
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
void showBand();
void showDDS();
void setWord(byte* SysWord,byte v, boolean val);
boolean getWord (byte SysWord, byte v);

MenuClass band(BandUpdate);
MenuClass vfo(NULL);
MenuClass stp(NULL);
MenuClass shf(ShiftUpdate);


void doSetGroup() {
  return;
}

void handleSerialCommand() {


//**************** To be Implemented !!
  
}

byte step2byte(long int step) {

   if (step==VFO_STEP_100Hz)  {return 0;}
   if (step==VFO_STEP_1KHz)   {return 1;}
   if (step==VFO_STEP_10KHz)  {return 2;}
   if (step==VFO_STEP_100KHz) {return 3;}
   return 4;
   
  
}

long int byte2step(byte s) {

   if (s==0)  {return VFO_STEP_100Hz;}
   if (s==1)  {return VFO_STEP_1KHz;}
   if (s==2)  {return VFO_STEP_10KHz;}
   if (s==3)  {return VFO_STEP_100KHz;}
   return VFO_STEP_1MHz;
   
  
}
void defineMenu(){
//*============================================================================================
//* Define master menu and lower level tree for simpleA
//*============================================================================================
  
  menuRoot.add((char*)"Band",&band);
  menuRoot.add((char*)"VFO",&vfo);
  menuRoot.add((char*)"Step",&stp);
  menuRoot.add((char*)"IF Shift",&shf);

  band.add((char*)"Off      ",NULL);
  band.set(0);

  shf.add((char*)" 0 Hz",NULL);
  shf.set(0);

  vfo.add((char*)"A",NULL);
  vfo.add((char*)"B",NULL);

  stp.add((char*)"100  Hz",NULL);
  stp.add((char*)" 1  KHz",NULL);
  stp.add((char*)"10  KHz",NULL);
  stp.add((char*)"100 KHz",NULL);
  stp.add((char*)"  1 MHz",NULL);

  for (int i=0; i <= BANDMAX; i++){
      vx.bandvfo[VFOA][i]=vx.loFreq[i];
      vx.bandvfo[VFOB][i]=vx.loFreq[i];
  }

}

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

//*--------------------------------------------------------------------------------------------
//* savesinpleA
//* save specifics of sinpleA
//*--------------------------------------------------------------------------------------------
void saveMenu() {

     switch(stp.get()) {
         case 0:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_100Hz;break;}
         case 1:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_1KHz;break;}
         case 2:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_10KHz;break;}
         case 3:                   {vx.vfostep[vx.vfoAB]=VFO_STEP_100KHz;break;}
         default:                  {vx.vfostep[vx.vfoAB]=VFO_STEP_1MHz;break;}
     }    
  
}
//*--------------------------------------------------------------------------------------------
//* Band Update   (CALLBACK from Menu System)
//* Set band label and limits
//*--------------------------------------------------------------------------------------------
void BandUpdate() {

  if (band.mItem < BANDMAX && band.CW == true) {
      band.mItem++;
  }
  if (band.mItem > 0 && band.CCW == true) {
      band.mItem--;
  }
  char* s=(char*)"                  "; 

#if DEBUG
  sprintf(hi,"BandUpdate callback mItem= %i",band.mItem);
  Serial.println(hi);
  sprintf(hi,"BandUpdate get()=%i",band.get());
  Serial.println(hi);
#endif  
  
  switch(band.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"160m";break;};
    case 2:                          {s=(char*)"80m";break;}; 
    case 3:                          {s=(char*)"40m";break;}; 
    case 4:                          {s=(char*)"20m";break;}; 
    case 5:                          {s=(char*)"15m";break;}; 
    default:                         {s=(char*)"10m";break;}; 
  }

  vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.get()]*1000,vx.hiFreq[band.get()]*1000);
  
  if ((vx.vfo[vx.vfoAB]>=vx.vfoAB,vx.loFreq[band.get()]*1000) && (vx.vfo[vx.vfoAB]>=vx.vfoAB,vx.loFreq[band.get()]*1000)) {
    
    //* do nothing, already inband
    
  } else {  
    vx.vfo[vx.vfoAB]=vx.bandvfo[vx.vfoAB][band.get()]*1000;
  }


#if DEBUG  
  sprintf(hi,"BandUpdate bandvfo[%i][%i]=%ld",vx.vfoAB,band.get(),vx.bandvfo[vx.vfoAB][band.get()]);
  Serial.println(hi);

  sprintf(hi,"BandUpdate callback loFreq= %ld hiFreq= %ld",vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);
  Serial.println(hi);

  sprintf(hi,"BandUpdate VFO A= %ld VFO B= %ld",vx.vfo[VFOA],vx.vfo[VFOB]);
  Serial.println(hi);

  sprintf(hi,"BandUpdate callback text %s",s);
  Serial.println(hi);
#endif
    
  band.l.get(0)->mText=s;
  //showPanel();
  
  return;
  
}
//*--------------------------------------------------------------------------------------------
//* Shift Update   (CALLBACK from Menu System)
//* Set shift label and limits
//*--------------------------------------------------------------------------------------------
void ShiftUpdate() {
  
  if (shf.mItem < 15 && shf.CW == true) {
      shf.mItem++;
  }
  if (shf.mItem > 0 && shf.CCW == true) {
      shf.mItem--;
  }
  char* s=(char*)"                  "; 

#if DEBUG 
  sprintf(hi,"Shift Update callback mItem= %i",shf.mItem);
  Serial.println(hi);
#endif


  sprintf(s,"%i KHz",shf.mItem);
  
#if DEBUG  
  sprintf(hi,"BandUpdate callback loFreq= %ld hiFreq= %ld",loFreq[band.mItem]*1000,hiFreq[band.mItem]*1000);
  Serial.println(hi);
  
  sprintf(hi,"BandUpdate callback text %s",s);
  Serial.println(hi);
#endif
    
  shf.l.get(0)->mText=s;
  //showPanel();
  return;
}


void readEEPROM(){
  

#if DEBUG 
     sprintf(hi,"EEPROM initialization Band(%i)",EEPROM.read(31));
     Serial.println(hi);
#endif

     if (vx.vfoAB == VFOA) {                    
        if ((EEPROM.read(30) >= 0) && (EEPROM.read(30)<= BANDMAX)) {
           band.set(EEPROM.read(30));
        } else {
           band.set(0);   
        }
     } else {
        if ((EEPROM.read(31) >= 0) && (EEPROM.read(31)<= BANDMAX)) {
           band.set(EEPROM.read(31));
        } else {
           band.set(0);   
        }         
     }

     vx.vfostep[VFOA]=byte2step(EEPROM.read(32));
     vx.vfostep[VFOB]=byte2step(EEPROM.read(33));
     
     if (vx.vfoAB == VFOA) {                    
        if ((EEPROM.read(32) >= 0) && (EEPROM.read(32)<= 4)) {
           stp.set(EEPROM.read(32));
        } else {
           stp.set(0);   
        }
     } else {
        if ((EEPROM.read(33) >= 0) && (EEPROM.read(33)<= 4)) {
           stp.set(EEPROM.read(33));
        } else {
           stp.set(0);   
        }         
     }

    

     if ((EEPROM.read(18) >= 0) && (EEPROM.read(18)<= 15)) {
        shf.set(EEPROM.read(18));
     } else {
        shf.set(0);   
     }

 
     return;
}          

//*----

void writeEEPROM() {
  
  
  
  EEPROM.write(18,shf.get());
  
  EEPROM.write(30,vx.vfoband[VFOA]);
  EEPROM.write(31,vx.vfoband[VFOB]);
  EEPROM.write(32,step2byte(vx.vfostep[VFOA]));
  EEPROM.write(33,step2byte(vx.vfostep[VFOB]));
  
  return;
}

void showGUI(){
      showBand();
      showDDS();
      return;
}

void checkBandLimit(){

#if DEBUG
  sprintf(hi,"setup band=%i",band.get());
  Serial.println(hi);
  
  sprintf(hi,"entry with VFOA=%ld VFOB=%ld",vx.vfo[VFOA],vx.vfo[VFOB]);
  Serial.println(hi);
#endif


  
  if (vx.vfo[VFOA]<vx.loFreq[band.get()]*1000 || vx.vfo[VFOA]>vx.hiFreq[band.get()]*1000) {vx.vfo[VFOA]=vx.loFreq[band.get()]*1000;}
  if (vx.vfo[VFOB]<vx.loFreq[band.get()]*1000 || vx.vfo[VFOB]>vx.hiFreq[band.get()]*1000) {vx.vfo[VFOB]=vx.loFreq[band.get()]*1000;}

#if DEBUG
  sprintf(hi,"exit checkBandLimit with VFOA=%ld VFOB=%ld",vx.vfo[VFOA],vx.vfo[VFOB]);
  Serial.println(hi);
#endif
  
  return;
}

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

  if (stp.get()==0) {
     lcd.print(" ");
     lcd.print((*v).hundreds);
     lcd.print((*v).tens);
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
  if (vx.vfoAB==VFOA){lcd.print("A");} else {lcd.print("B");}
  //lcd.print("");
  return;
}


#endif

