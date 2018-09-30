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

#define FI_LOW     0
#define FI_HIGH   15

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
void vfoUpdate();
void StepUpdate();
void LckUpdate();
void ModUpdate();

//void setDDSFreq();

void showBand();
void showDDS();

void setWord(byte* SysWord,byte v, boolean val);
boolean getWord (byte SysWord, byte v);

MenuClass band(BandUpdate);
MenuClass vfo(vfoUpdate);
MenuClass stp(StepUpdate);
MenuClass shf(ShiftUpdate);
MenuClass lck(LckUpdate);
MenuClass mod(ModUpdate);


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
  lck.set(0);
  
  mod.add((char*)"DDS",NULL);
  mod.set(0);
  

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

//*----- FI Displacemente

 if (mod.get()>0) {
    FI=shf.get()*4;
 } 

//*---- Set DDS with new frequency
 long int fDDS=vx.get(vx.vfoAB)/1000;


 //*---- trace code make into DEBUG later
 sprintf(hi,"DDS frequency= %ld FI=%ld mod(%d) shf.get=%d",fDDS,FI,mod.get(),shf.get());
 Serial.println(hi);

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

   sprintf(hi,"<%s> VFOA=%ld VFOB=%ld",m,vx.get(VFOA),vx.get(VFOB));
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
      debugPrint((char*)"VFO Change Entry");
      
      vx.vfoAB=vfo.mItem;
      
      band.mItem=(vx.vfoband[vx.vfoAB]);
      BandUpdate();
      
      stp.mItem=(vx.step2code(vx.vfostep[vx.vfoAB]));
      StepUpdate();
      
      //vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);

      debugPrint((char*)"VFO Change Exit");
       
   } else {

      if (vx.vfoband[vx.vfoAB]!=band.mItem) { //Switch Band
         debugPrint((char*)"BAND Change Entry"); 
         vx.vfoband[vx.vfoAB]=band.mItem;
       
/*            if ((vx.get(vx.vfoAB])>=vx.loFreq[band.mItem]*1000) && (vx.get(vx.vfoAB)<=vx.hiFreq[band.mItem]*1000)) {

            } else {
              vx.set(vx.vfoAB,vx.loFreq[band.mItem]*1000);  //* solo altera la frecuencia si resulta que es distinto
            }
*/            
          vx.set(vx.vfoAB,vx.loFreq[vx.vfoband[vx.vfoAB]]*1000);
          debugPrint((char*)"BAND Change Exit"); 
   
    }

         if (vx.vfostep[vx.vfoAB]!=vx.code2step(stp.mItem)) { //Change tuning step
            debugPrint((char*)"STEP Change Entry");
            vx.vfostep[vx.vfoAB]=vx.code2step(stp.mItem);
            debugPrint((char*)"STEP Change Exit"); 
         }
   }

   vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);
   vx.set(vx.vfoAB,vx.get(vx.vfoAB));
   
   //vx.changeVFO();

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


  band.l.get(0)->mText=s;
  band.CW=false;
  band.CCW=false;
  
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
void vfoUpdate() {

  char* s=(char*)"  "; 

  if (vfo.mItem < VFOB && vfo.CW == true) {
      vfo.mItem++;
  }
  if (vfo.mItem > VFOA && vfo.CCW == true) {
      vfo.mItem--;
  }

  if (vfo.mItem==VFOA){s=(char*)"A";}
  if (vfo.mItem==VFOB){s=(char*)"B";}
  
  vfo.l.get(0)->mText=s;
  
  vfo.CW=false;
  vfo.CCW=true;
  
  return;
}

//*--------------------------------------------------------------------------------------------
//* LckUpdate Callback
//* Control VFO Lock
//*--------------------------------------------------------------------------------------------
void LckUpdate(){
  char* s=(char*)"    "; 

  if (lck.mItem < 1 && lck.CW == true) {
      lck.mItem++;
  }
  if (lck.mItem > 0 && lck.CCW == true) {
      lck.mItem--;
  }
  switch(lck.mItem) {
    case 0:                          {s=(char*)"Off";vx.VFOlock=false;break;};     
    default:                         {s=(char*)"On";vx.VFOlock=true;break;};  
  }
  
  lck.l.get(0)->mText=s;
  lck.CW=false;
  lck.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* ModUpdate Callback
//* Control VFO Mode (DDS/VFO)
//*--------------------------------------------------------------------------------------------
void ModUpdate(){
  char* s=(char*)"    "; 

  if (mod.mItem < 1 && mod.CW == true) {
      mod.mItem++;
  }
  if (mod.mItem > 0 && mod.CCW == true) {
      mod.mItem--;
  }

  switch(mod.mItem) {
      case 0:              {s=(char*)"DDS";break;};
      default:             {s=(char*)"VFO";break;};  
  }
  
  mod.l.get(0)->mText=s;
  mod.CW=false;
  mod.CCW=false;
  return;
}
//*--------------------------------------------------------------------------------------------
//* Step Update
//* Set step limit
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

     byte c=EEPROM.read(35);
     byte b=EEPROM.read(36);

     if ((b>=0) && (b<=1)){lck.set(b);}
     if ((c>=0) && (c<=1)){mod.set(b);}
 
     return;
}    

//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------

void writeEEPROM() {
  
  
  EEPROM.write(18,shf.get());
  EEPROM.write(35,mod.get());
  EEPROM.write(36,lck.get());
  
  
  return;
}
//*--------------------------------------------------------------------------------------------
//* showBand
//* show Band setting at the display
//*--------------------------------------------------------------------------------------------
void showBand() {

  lcd.setCursor(8,0);
  lcd.print(String(band.getCurrentText()));
 
};      
//*--------------------------------------------------------------------------------------------
//* showMode
//* show DDS Mode
//*--------------------------------------------------------------------------------------------
void showMode() {

  lcd.setCursor(12,0);
  if (mod.mItem==0) {
    lcd.print("D");
  } else {
    lcd.print("V");
  }
  
 
};     
//*--------------------------------------------------------------------------------------------
//* showLock
//* show Lock Mode
//*--------------------------------------------------------------------------------------------
void showLock() {

/*
  lcd.setCursor(10,1);
  if (lck.mItem==0) {
    lcd.print(" ");
  } else {
    lcd.print("X");
  }
*/  
 
};     
//*--------------------------------------------------------------------------------------------
//* writeEEPROM
//* Write specific configuration
//*--------------------------------------------------------------------------------------------
void showGUI(){
      showBand();
      showLock();
      showMode();
      return;
}
//*--------------------------------------------------------------------------------------------
//* checkBandLimit
//* Verify if the set frequency is within the band limits and correct
//*--------------------------------------------------------------------------------------------
void checkBandLimit(){

#if DEBUG
  sprintf(hi,"setup band=%i",band.get());
  Serial.println(hi);
  
  sprintf(hi,"entry with VFOA=%ld VFOB=%ld",vx.get(VFOA),vx.get(VFOB));
  Serial.println(hi);
#endif

  band.mItem=vx.vfoband[vx.vfoAB];
  stp.mItem=vx.step2code(vx.vfostep[vx.vfoAB]);
  vx.setVFOLimit(vx.vfoAB,vx.loFreq[band.mItem]*1000,vx.hiFreq[band.mItem]*1000);
  
  if (vx.get(VFOA)<vx.loFreq[band.get()]*1000 || vx.get(VFOA)>vx.hiFreq[band.get()]*1000) {vx.set(VFOA,vx.loFreq[band.get()]*1000);}
  if (vx.get(VFOB)<vx.loFreq[band.get()]*1000 || vx.get(VFOB)>vx.hiFreq[band.get()]*1000) {vx.set(VFOB,vx.loFreq[band.get()]*1000);}

  BandUpdate();
  StepUpdate();

#if DEBUG
  sprintf(hi,"exit checkBandLimit with VFOA=%ld VFOB=%ld",vx.get(VFOA),vx.get(VFOB));
  Serial.println(hi);
#endif
  
  return;
}

//*-----------------------------------------------------------------------------------------------------------------------------------
//* setFrequencyHook
//* Device specific frequency display
//*-----------------------------------------------------------------------------------------------------------------------------------
void setFrequencyHook(long int f,FSTR* v) {
 

#if DEBUG  
  sprintf(hi,"Frequency %ld",f);
  Serial.println(hi);
  sprintf(hi,"FrequencyDDS %ld",fDDS);
  Serial.println(hi);
#endif
    
//*******************************
//* Setup DDS Frequency         *
//*******************************


  if (stp.get()<3) {
     lcd.print(" ");
     lcd.print((*v).hundreds);
     lcd.print((*v).tens);
  }

  if (stp.get()==0){
     lcd.print((*v).ones);
  }

  
}
//*--------------------------------------------------------------------------------------------
//* setSysOM
//* Setup specific System Vars
//*--------------------------------------------------------------------------------------------
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
//*--------------------------------------------------------------------------------------------
//* handleTimerHook
//* Hook for timer tick
//*--------------------------------------------------------------------------------------------

void handleTimerHook(){

}
//*--------------------------------------------------------------------------------------------
//* showVFOHook
//* Hook for the VFO display
//*--------------------------------------------------------------------------------------------
void showVFOHook(){
  lcd.setCursor(0,0);
  //lcd.print("Vfo");
  if (vfo.get()==VFOA){lcd.print("A");} else {lcd.print("B");}
  //lcd.print("");
  return;
}


#endif

