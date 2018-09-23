//*--------------------------------------------------------------------------------------------------
//* DDSPlus Firmware Version 1.0
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de DDS  en su version inicial
//* Implementación 
//*         PICOFM      Transceiver basado en DRA881V
//*         SINPLEA     Receiver SDR usando shield Arduino Elektor
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//* Plataforma: Arduino NANO/UNO/Mega
//* LCD: 16x2 HD44780 o equivalente
//* Standard Arduino PIN Assignment
//*   A0 - LCDKeyShield (Joystick)
//*   A1 - DDS DATA (reserved)
//*   A2 - DDS RESET (reserved)
//*   A3 - PAD CW
//*   A4 - Power (HI/LO)
//*   A5 - Meter 
//*   
//*   LCD Handling
//*   D2 - Encoder 
//*   D3 - Encoder
//*   D4 - DB4
//*   D5 - DB5
//*   D6 - DB6
//*   D7 - DB7
//*   D8 - E
//*   D9 - RW
//*   D10- BackLigth
//*
//*   Transceiver control
//*   D0 - RxD
//*   D1 - TxD

//*   D11- DDS W_CLK (reserved)
//*   D12- DDS FU_UD (reserved)
//*   D13- KEYER
//*
//*-------------------------------------------------------------------------------------------------------
//*----- Program Reference data
//*-------------------------------------------------------------------------------------------------------

#define DEBUG         false
#define PICOFM        false
#define SINPLEA       true


//*-------- Copyright and Program Build information

#define PROG_BUILD  "040"
#define COPYRIGHT "(c) LU7DID 2018"

//*----------------------------------------------------------------------------------
//*  System Status Word
//*----------------------------------------------------------------------------------
//*--- Master System Word (MSW)

#define CMD       B00000001
#define GUI       B00000010
#define PTT       B00000100
#define DRF       B00001000
#define DOG       B00010000
#define LCLK      B00100000
#define SQL       B01000000
#define BCK       B10000000

//*----- Master Timer and Event flagging (TSW)

#define FT1       B00000001
#define FT2       B00000010
#define FT3       B00000100
#define FT4       B00001000
#define FCLOCK    B00010000
#define FTS       B00100000
#define FDOG      B01000000
#define FBCK      B10000000

//*----- UI Control Word (USW)

#define BBOOT     B00000001
#define BMULTI    B00000010
#define BCW       B00000100
#define BCCW      B00001000
#define SQ        B00010000
#define MIC       B00100000
#define KDOWN     B01000000
 
#define BUSY      B10000000       //* Used for Squelch Open in picoFM and for connected to DDS on sinpleA
#define CONX      B10000000

//*----- Joystick Control Word (JSW)

#define JLEFT     B00000001
#define JRIGHT    B00000010
#define JUP       B00000100
#define JDOWN     B00001000

//*----- EEPROM signature

#define EEPROM_COOKIE  0x1f
//*----------------------------------------[DEFINITION]----------------------------------------------

//*--- Control delays

#define DELAY_DISPLAY 4000     //delay to show the initial display in mSecs
#define DELAY_SAVE    1000     //delay to consider the frequency needs to be saved in mSecs
#define LCD_DELAY     1000
#define CMD_DELAY      100
#define PTY_DELAY      200
#define DIM_DELAY    30000
#define DOG_DELAY    60000
#define BLINK_DELAY   1000
#define SAVE_DELAY    2000

#define FORCEFREQ     0
#define LCD_ON        1
#define LCD_OFF       0

#define QUEUEMAX  16        // Queue of incoming characters 

//*----------------------------------------[INCLUDE]-------------------------------------------------
#include <LiquidCrystal.h>
#include <stdio.h>
#include <EEPROM.h>

#include "MemSizeLib.h"
#include "VFOSystem.h"
#include "ClassMenu.h"


//*-------------------------------------------------------------------------------------------------
//* Define callback functions
//*-------------------------------------------------------------------------------------------------
void  Encoder_san();
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*------ define root for all the menu system
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
MenuClass menuRoot(NULL);

//*-------------------------------------------------------------------------------------------------
//* Define class to manage VFO
//*-------------------------------------------------------------------------------------------------
void showFreq();   //Prototype fur display used on callback
VFOSystem vx(showFreq,NULL);

//*--------------------------------------------------------------------------------------------
//*---- Definitions for various LCD display shields
//*--------------------------------------------------------------------------------------------
       LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  
       
       //==============================================
       //Set Encoder pin
       //==============================================
       const int Encoder_A =  3;            // Incremental Encoder singal A is PD3 
       const int Encoder_B =  2;            // Incremental Encoder singal B is PD2 
       
       unsigned int Encoder_number=0;
       
       int state=0;

       // define some values used by the panel and buttons
       int lcd_key     = 0;
       int adc_key_in  = 0;
       
       #define btnRIGHT  0
       #define btnUP     1 
       #define btnDOWN   2 
       #define btnLEFT   3
       #define btnSELECT 4
       #define btnNONE   5   
       #define btnEncodeOK  6
        
//*---- Debug buffer
char hi[80];


//*--------------------------------------------------------------------------------
//*--- Pseudo Real Time Clock  
//*--------------------------------------------------------------------------------

byte mm=0;
byte ss=0;

unsigned long Tclk=1000;
byte btnPrevio=btnNONE;

//*--------------------------------------------------------------------------------
//*--- Timer related definitions
//*--------------------------------------------------------------------------------

#define T_1mSec 65473 //Timer pre-scaler for 1 KHz or 1 msec

uint16_t T1=0;
uint16_t T2=0;
uint16_t T3=0;
uint16_t T4=0;
uint16_t TV=0;
uint16_t TS=0;
uint16_t TDOG=0;
uint16_t TBCK=0;
uint16_t TDIM=0;

//*-----------------------------------------------------------------------------------
//*--- Define System Status Words
//*-----------------------------------------------------------------------------------

byte MSW = 0;
byte TSW = 0;
byte USW = 0;
byte JSW = 0;

//*-----------------------------------------------------------------------------------
//*--- Serial port management areas
//*-----------------------------------------------------------------------------------

char serialQueue[QUEUEMAX]; // Actual Queue space [a0,a1,...,an]
byte pQueue = 0;            // Pointer to next position to use
byte inState= 0;
byte inCmd=0;


int_fast32_t timepassed = millis(); // int to hold the arduino miilis since startup
int_fast32_t menupassed = millis();

int memstatus   = 1;           // value to notify if memory is current or old. 0=old, 1=current.


//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*  Model specific includes
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "picoFM.h"
#include "sinpleA.h"

//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*  Setup
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  //*--- PinOut Definition

  lcd.begin(16, 2);   // start the LCD library  
  pinMode(10,OUTPUT);
   
   //*---- Turn On LED Backlight (1) or off (0)
   
  digitalWrite(10,LCD_ON);
  TDIM=DIM_DELAY;
   
   
  lcd.setCursor(0,0);  
   
  pinMode(Encoder_A, INPUT); 
  pinMode(Encoder_B, INPUT); 
   
  digitalWrite(Encoder_A, 1);
  digitalWrite(Encoder_B, 1);
   
//*========================================
  attachInterrupt(1, Encoder_san, FALLING);        //interrupts: numbers 0 (on digital pin 2) and 1 (on digital pin 3).

//*-------------------------------------------------------------------------------
//* I/O Definitions for the DRA818V
//*-------------------------------------------------------------------------------
#if PICOFM
  picoFMPinSetup();
#endif

//*=====================================================================================
  
  lcd.setCursor(0, 0);        // Place cursor at [0,0]
  lcd.print(String(PROGRAMID)+" v"+String(PROG_VERSION)+"-"+String(PROG_BUILD));

  lcd.setCursor(0, 1);        // Place cursor at [0,1]
  lcd.print(String(COPYRIGHT));

  delay(DELAY_DISPLAY);
  lcd.clear();

//*---- Define the VFO System parameters (Initial Firmware conditions)
 
  vx.setVFOFreq(VFOA,VFO_START);
  vx.setVFOStep(VFOA,VFO_STEP_1KHz);
  vx.setVFOBand(VFOA,VFO_START,VFO_END);

  vx.setVFOFreq(VFOB,VFO_START);
  vx.setVFOStep(VFOB,VFO_STEP_1KHz);
  vx.setVFOBand(VFOB,VFO_START,VFO_END);

  vx.setVFO(VFOA);
 
    
//*--- initializes the frequency step for both VFO A & B
//*--- Interrupt manipulation
//#if LCD_STANDARD
//
//  PCICR  = 0b00000010;        // 1. PCIE1: Pin Change Interrupt Enable 1
//  PCMSK1 = 0b00011111;        // Enable Pin Change Interrupt for A0, A1, A2, A3, A4
//
//#endif
  
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  //*---> TCNT1 = 34286; // preload timer 65536-16MHz/256/2Hz
  TCNT1 = T_1mSec; // preload timer 65536- [(16000000/256)/f(Hz)]
  TCCR1B |= (1 << CS12); // 256 prescaler
  TIMSK1 |= (1 << TOIE1); // enable timer overflow interrupt
  interrupts(); // enable all interrupts

//*--- Init serial port and establish handshake with DRA018F 

  Serial.begin(9600);
  serialQueue[0]=0x00;

//*============================================================================================
//* Define menu system PICOFM
//*============================================================================================
#if PICOFM
  definepicoFMmenu();
#endif

//*============================================================================================
//* Define menu system SINPLEA
//*============================================================================================
#if SINPLEA
  definesinpleAmenu();
#endif

//*============================================================================================
//*--- Load the stored frequency
//*============================================================================================


  if (FORCEFREQ == 0) {

     if (EEPROM.read(30)==EEPROM_COOKIE) {

            //*---- Recover frequency for VFOA
            char hi[12];    
            sprintf(hi,"%3d%1d%1d%1d%1d%1d%1d",EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),EEPROM.read(4),EEPROM.read(5),EEPROM.read(5),EEPROM.read(6),EEPROM.read(7));
            vx.vfo[VFOA]=String(hi).toInt();

            //*---- Recover frequency for VFOB
            sprintf(hi,"%3d%1d%1d%1d%1d%1d%1d",EEPROM.read(8),EEPROM.read(9),EEPROM.read(10),EEPROM.read(11),EEPROM.read(12),EEPROM.read(13),EEPROM.read(14));
            vx.vfo[VFOB]=String(hi).toInt();            
            
            vx.vfoAB = EEPROM.read(15);        
            vfo.set(EEPROM.read(26));
            stp.set(EEPROM.read(18));

            #if PICOFM
                sql.set(EEPROM.read(0));
                pwr.set(EEPROM.read(16));
                wdg.set(EEPROM.read(17));
                rpt.set(EEPROM.read(19));
                spd.set(EEPROM.read(20));
                bdw.set(EEPROM.read(21));
                //hpf.set(EEPROM.read(22));
                //lpf.set(EEPROM.read(23));
                //pre.set(EEPROM.read(24));
                ctc.set(EEPROM.read(25));
            #endif

            #if SINPLEA
                if ((EEPROM.read(31) >= 0) && (EEPROM.read(31)<= BANDMAX)) {
                   band.set(EEPROM.read(31));
                } else {
                   band.set(0);   
                }

                for (int i=0; i <= BANDMAX; i++){
                    bandvfo[i]=EEPROM.read(i+32);
                }
                
            #endif

            MSW = EEPROM.read(27);
            USW = EEPROM.read(28);
            TSW = EEPROM.read(29);
     }
  } 
    
  if (vx.vfo[VFOA]<loFreq[band.mItem]*1000 || vx.vfo[VFOA]>hiFreq[band.mItem]*1000) {vx.vfo[VFOA]=loFreq[band.mItem]*1000;}
  if (vx.vfo[VFOB]<loFreq[band.mItem]*1000 || vx.vfo[VFOB]>hiFreq[band.mItem]*1000) {vx.vfo[VFOB]=loFreq[band.mItem]*1000;}


//**********************************************************************************************
//*--- Initial value for system operating modes
//**********************************************************************************************

  setWord(&MSW,CMD,false);
  setWord(&MSW,GUI,false);
  setWord(&MSW,PTT,true);
  setWord(&MSW,DRF,false);
  setWord(&MSW,DOG,false);
  setWord(&MSW,LCLK,false);
  setWord(&MSW,SQL,false);

  setWord(&USW,BBOOT,true);
  setWord(&USW,BMULTI,false);
  setWord(&USW,BCW,false);
  setWord(&USW,BCCW,false);
  setWord(&USW,SQ,false);
  setWord(&USW,MIC,false);
  setWord(&USW,KDOWN,false);

#if PICOFM
  setWord(&USW,BUSY,false);
#endif

#if PICOFM
  setWord(&USW,CONX,false);
#endif


  setWord(&JSW,JLEFT,false);
  setWord(&JSW,JRIGHT,false);
  setWord(&JSW,JUP,false);
  setWord(&JSW,JDOWN,false);


#if PICOFM
//===================================================================================
//* DRA018V module initialization and setup
//===================================================================================

  doHandShake();
  doSetVolume();
  doSetFilter();
  doSetGroup();

#endif
//===================================================================================


#if SINPLEA
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
  
#endif
//===================================================================================


//*=========================================================================================================
#if DEBUG

//*--- Print Serial Banner (TEST Mode Mostly)
  sprintf(hi,"%s %s Compiled %s %s",PROGRAMID,PROG_VERSION,__TIME__, __DATE__);
  Serial.println(hi);
  sprintf(hi,"(c) %s",COPYRIGHT);
  Serial.print("RAM Free=");
  Serial.println(freeMemory());

#endif
//*=========================================================================================================

//***********************************
//* End of initialization and setup *
//***********************************
  showPanel();

}
//*****************************************************************************************************
//*                               MenÃº Finite Status Machine (FSM)
//*
//*****************************************************************************************************
//*--------------------------------------------------------------------------------------------
//* menuText
//* defines the text based on Menu FSM state
//*--------------------------------------------------------------------------------------------
String menuText(byte mItem) {

//*---- Here CMD==true and GUI==true so it's a second level menu
   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);
   return z->getText(i);
 
}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void showFreq() {

  FSTR v;  

    
  long int f=vx.vfo[vx.vfoAB];   
  long int fDDS=f/1000;
    
  vx.computeVFO(f,&v);

#if DEBUG  
  sprintf(hi,"Frequency %ld",f);
  Serial.println(hi);
  sprintf(hi,"FrequencyDDS %ld",fDDS);
  Serial.println(hi);
#endif



//*---- Prepare to display
  
  if (v.millions > 9) {
    lcd.setCursor(2, 1);
  }
  else {
    lcd.setCursor(3, 1);
  }

  
  lcd.print(v.millions);
  lcd.print(".");
  lcd.print(v.hundredthousands);
  lcd.print(v.tenthousands);
  lcd.print(v.thousands);

#if SINPLEA
//*******************************
//* Setup DDS Frequency         *
//*******************************
  //*---- Set DDS with new frequency
  setDDSfreq(fDDS);

  lcd.print(" ");
  lcd.print(v.hundreds);
  lcd.print(v.tens);

#endif
  
  timepassed = millis();
  memstatus = 0; // Trigger memory write

};


#if PICOFM
//****************************************************
//* picoFM GUI and Panel related unique functions    *
//****************************************************

//*--------------------------------------------------------------------------------------------
//* showRpt
//* show repeater operation mode at the display
//*--------------------------------------------------------------------------------------------
void showRpt() {

  
  lcd.setCursor(0,0);
  if (rpt.get()==0){lcd.print(" ");} else {lcd.print("S");}
  //lcd.print(rpt.getText(rpt.get()));


};
//*--------------------------------------------------------------------------------------------
//* showCTC
//* show CTCSS operation mode at the display
//*--------------------------------------------------------------------------------------------
void showCTC() {


  lcd.setCursor(1,0);
  if (ctc.get()==0){lcd.print(" ");} else {lcd.print("T");}

};
//*--------------------------------------------------------------------------------------------
//* showPwr
//* show power level at the display
//*--------------------------------------------------------------------------------------------
void showPwr() {

  
  lcd.setCursor(11,0);
  if (pwr.get()==0) {lcd.print("L");} else {lcd.print("H");}
  return;


}
//*--------------------------------------------------------------------------------------------
//* showMet
//* show meter
//*--------------------------------------------------------------------------------------------
void showMet() {


     //showMeter(&sqlMeter,sqlMeter.v);
     return;

}
//*--------------------------------------------------------------------------------------------
//* showSPD
//* show HILO operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSPD() {


  lcd.setCursor(5,0);
  if (spd.get()==0) {lcd.print(" ");} else {lcd.print("Z");}


};
//*--------------------------------------------------------------------------------------------
//* showSQL
//* show SQL operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSQL() {


  lcd.setCursor(12,0);
  if (getWord(MSW,CMD)==true) {return;}
  if (digitalRead(A3)==LOW && getWord(MSW,CMD)==false) {lcd.write(byte(5));} else {lcd.write(byte(0));}

  
};

//*--------------------------------------------------------------------------------------------
//* showPTT
//* show PTT operation mode at the display
//*--------------------------------------------------------------------------------------------
void showPTT() {

  lcd.setCursor(4,0);
  if (getWord(MSW,DOG)==true && wdg.get()!=0){lcd.write(byte(7)); lcd.setCursor(4,0);lcd.blink();return;}
  if (getWord(MSW,PTT)==false) {lcd.write(byte(6)); lcd.setCursor(4,0);lcd.noBlink();} else {lcd.print(" ");lcd.setCursor(4,0);lcd.noBlink();}


};

//*--------------------------------------------------------------------------------------------
//* showDRF
//* show DRF filter at the display
//*--------------------------------------------------------------------------------------------
void showDRF() {
  
  lcd.setCursor(3,0);
  if (getWord(MSW,DRF)==false){lcd.print(char(174));} else {lcd.print(char(42));}


};
//*--------------------------------------------------------------------------------------------
//* showDog
//* show if watchdog is enabled or not
//*--------------------------------------------------------------------------------------------
void showDog() {

  
  lcd.setCursor(9,0);
  if (wdg.get()==0){lcd.print(" ");} else {lcd.print("W");}
  

};

#endif


#if SINPLEA
//****************************************************
//* sinpleA GUI and Panel related unique functions    *
//****************************************************
//*--------------------------------------------------------------------------------------------
//* showDDS
//* show DDS connection or error condition at the display
//*--------------------------------------------------------------------------------------------
void showDDS() {

  lcd.setCursor(13,0);
  lcd.print("[");
  if (getWord(USW,CONX)==true) {lcd.print("*");} else {lcd.print(" ");}
  lcd.print("]");
 
};


//*--------------------------------------------------------------------------------------------
//* showBand
//* show Band setting at the display
//*--------------------------------------------------------------------------------------------
void showBand() {

  lcd.setCursor(6,0);
  //byte i=band.get();
  lcd.print("<");
  lcd.print(String(band.getCurrentText()));
  lcd.print(">");

#if DEBUG
  sprintf(hi,"showBand %s",band.getCurrentText());
  Serial.println(hi);
#endif
 
};
#endif

//****************************************************
//* Common GUI and Panel related unique functions    *
//****************************************************
//*--------------------------------------------------------------------------------------------
//* showVFO
//* show VFO filter at the display
//*--------------------------------------------------------------------------------------------
void showVFO() {

#if PICOFM
  lcd.setCursor(2,0);
  if (vx.vfoAB==VFOA){lcd.print("A");} else {lcd.print("B");}
#endif

#if SINPLEA
  lcd.setCursor(0,0);
  lcd.print("Vfo");
  if (vx.vfoAB==VFOA){lcd.print("A");} else {lcd.print("B");}
  //lcd.print("");
#endif

};


//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency or menu information at the display
//*--------------------------------------------------------------------------------------------
void showPanel() {

   
   if (getWord(MSW,CMD)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);
      
      showFreq();
      showVFO();

#if PICOFM      
      showPwr();
      showRpt();
      showCTC();
      showDog();
      showSQL();
      showPTT();     
      showSPD();
      showDRF();
      //showMet();
#endif

#if SINPLEA
      showBand();
      showDDS();
#endif      
      
      return;
   }


//*--- if here then CMD==true

   byte i=menuRoot.get();
   MenuClass* z=menuRoot.getChild(i);
    
      
   if (getWord(MSW,GUI)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(i)+"> "+String(menuRoot.getText(i)));
      lcd.setCursor(1,1);
      lcd.print("  "+String(z->getText(z->get()) ));

      return;
      
   } else {
 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(menuRoot.get())+"> "+String(menuRoot.getText(menuRoot.get())));
      lcd.setCursor(0,1);
      lcd.print(">");
      lcd.print("  "+String(z->getCurrentText()));
      return;
   }

}
//*----
//* Show Mark
//*----
void showMark(){
      lcd.setCursor(0,1);
      lcd.print(">"); 
}
//*----
//* UnshowMark
//*----
void unshowMark(){
       lcd.setCursor(0,1);
      lcd.print(" "); 
}
//*----------------------------------------------------------------------------------------------------
//* menuFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void menuFSM() {

   if (getWord(MSW,CMD)==false) {   //* VFO mode, no business here!
      return;
   }

   if (getWord(MSW,GUI)==false) {  //It's the first level     
      menuRoot.move(getWord(USW,BCW),getWord(USW,BCCW));
      showPanel();
      return;
   }

//*---- Here CMD==true and GUI==true so it's a second level menu

     byte i=menuRoot.get();
     MenuClass* z=menuRoot.getChild(i);
     z->move(getWord(USW,BCW),getWord(USW,BCCW));
     showPanel();
     return;
   
}
void showSave(){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Saving....");
  
}
#if PICOFM
//*--------------------------------------------------------------------------------------------
//* savepicoFM
//* save specifics of picoFM
//*--------------------------------------------------------------------------------------------
void savepicoFM() {

      //*--- Detect changes that needs to be reflected thru commands to the ChipSet
      if ( (menuRoot.get() == PWRMENU) && (j!=k)) {doSetPower();}
      if ( (menuRoot.get() == SPDMENU) && (j!=k)) {doSetPD();}
      
      
      if ( (menuRoot.get() == STPMENU ||
            menuRoot.get() == RPTMENU ||
            menuRoot.get() == SQLMENU ||
            menuRoot.get() == CTCMENU ||
            menuRoot.get() == TONMENU ||
            menuRoot.get() == VFOMENU ||
            menuRoot.get() == BDWMENU ) && (j!=k)) {doSetGroup();}

      if (stp.get()==0) {
         vx.vfostep[vx.vfoAB]=VFO_STEP_5KHz;
      } else {
         vx.vfostep[vx.vfoAB]=VFO_STEP_10KHz;
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

//*--------------------------------------------------------------------------------------------
//* doSave
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void doSave() {

      showSave();      
      delay(DELAY_SAVE);

      
      byte i=menuRoot.get();
      MenuClass* z=menuRoot.l.get(i)->mChild;
      byte j=z->mItem;
      byte k=z->mItemBackup;

//*--- Query VFO status

      vx.vfoAB=vfo.get();

//**************************************************
//* Device specific parameter saving               *
//**************************************************
#if PICOFM
      savepicoFM(); 
#endif


#if SINPLEA
     savesinpleA();
     
#endif
//***************************************************

      setWord(&MSW,CMD,false);
      setWord(&MSW,GUI,false);

      menuRoot.save();
      showPanel();

}

//*****************************************************************************************************
//*                            Command Finite Status Machine (FSM)
//*
//*****************************************************************************************************
//*----------------------------------------------------------------------------------------------------
//* backupFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void backupFSM() {
     
     byte i=menuRoot.get();
     MenuClass* z=menuRoot.getChild(i);
     z->backup();

}
//*----------------------------------------------------------------------------------------------------
//* restoreFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void restoreFSM() {

      byte i=menuRoot.get();
      MenuClass* z=menuRoot.getChild(i);
      z->restore();  
}


//*----------------------------------------------------------------------------------------------------
//* processVFO
//* come here to update the VFO
//*----------------------------------------------------------------------------------------------------
void processVFO() {

//int v1;
   
   if (getWord(USW,BCW)==true) {
       
       vx.updateVFO(vx.vfoAB,vx.vfostep[vx.vfoAB]);
       lcd.setCursor(0,1);
       lcd.print((char)126);
   
   }
   
   if (getWord(USW,BCCW)==true) {
       
       vx.updateVFO(vx.vfoAB,-vx.vfostep[vx.vfoAB]); 
       lcd.setCursor(0,1);
       lcd.print((char)127);
   
   }
   
   T4=LCD_DELAY;
 
}
//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the reading of the KeyShield buttons
//*--------------------------------------------------------------------------------------------------
void readKeyShield() {

 

 if(readButton()==true && getWord(USW,KDOWN)==true) {
  
   if (millis()-menupassed>=SAVE_DELAY) {
      setWord(&USW,KDOWN,false);
      setWord(&USW,BMULTI,false);
      setWord(&MSW,LCLK,true);
      return;
   }
   
   setWord(&USW,KDOWN,false);
   setWord(&USW,BMULTI,true);
   setWord(&MSW,LCLK,false);
   return;
 }
 

 if(readButton()==false && getWord(USW,KDOWN)==false) {
  
    setWord(&USW,KDOWN,true);
    menupassed=millis();
   return;
 }
 return; 

}

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the command and VFO handling FSM
//*--------------------------------------------------------------------------------------------------
void CMD_FSM() {

   readKeyShield();

#if PICOFM

   //*-------------------------------------------------------------------------
   //* Operate Watchdog
   //*-------------------------------------------------------------------------
       if (getWord(TSW,FDOG)==true && wdg.get() !=0) {
          setWord(&TSW,FDOG,false);
          setWord(&MSW,DOG,true);
          //showMeter(&sqlMeter,sqlMeter.v);
          digitalWrite(PTTPin,LOW);
          
          if (rpt.get()!=0) {vx.swapVFO();showVFO();showFreq();} //If split enabled        
          showPTT();        
       }

#endif

   //*-------------------------------------------------------------------------------
   //* Handle PTT  (detect MIC PTT pressed and PTT signal not activated) just pressed
   //*-------------------------------------------------------------------------------
       if (digitalRead(A4)==LOW && getWord(MSW,PTT)==true){

        
           setWord(&MSW,PTT,false);

#if PICOFM
           
           digitalWrite(PTTPin,HIGH);    //*-- Prende TX
           //showMeter(&pwrMeter,pwrMeter.v);
           if (rpt.get()!=0) {vx.swapVFO();showVFO();showFreq();} //If split enabled
#endif
           
           digitalWrite(10,LCD_ON);
           TDIM=DIM_DELAY; 

#if PICOFM           
           if (wdg.get()!=0) {TDOG=DOG_DELAY;}
           
           setWord(&MSW,DOG,false);
           showPTT();
#endif
           
       } else {
        
          if (digitalRead(A4)==HIGH && getWord(MSW,PTT)==false) {
              
              setWord(&MSW,PTT,true);
#if PICOFM
              
              digitalWrite(PTTPin,LOW);   //*-- Apaga TX
              //showMeter(&sqlMeter,sqlMeter.v);
              if (rpt.get()!=0) {vx.swapVFO();showVFO();} //If split enabled
              //showFreq();
              setWord(&MSW,DOG,false);
              showPTT();
#endif              
              showFreq();
 
          
          }   
       }

#if PICOFM
       
   //*-------------------------------------------------------------------------
   //* PTT == true means not activated (RX mode)
   //*-------------------------------------------------------------------------
   if (getWord(MSW,PTT) == true) {

     showSQL();
   }  
     //*--- Lectura analógica int s=readV(SQLPIN,SQLMAX,SQLSCALE);
     //sqlMeter.v=readMeter(&sqlMeter);

#endif   
   
//*-----------------------------------------------------------------------------
//* Menu management with KEY Shield cualquier cambio enciende LED
//*-----------------------------------------------------------------------------

   if ( getWord(USW,BMULTI)==true ||
        getWord(USW,BCW)==true    ||
        getWord(USW,BCCW)==true   ||
        getWord(MSW,LCLK)==true ) {
          
        digitalWrite(10,LCD_ON);
        TDIM=DIM_DELAY; 
   }

//*------------------------------------------------------------------------------
//* If PTT==false (TX) clear all signals, thus disable any activity of the keys
//*------------------------------------------------------------------------------
   if (getWord(MSW,PTT)==false) {
    
      setWord(&USW,BMULTI,false);
      setWord(&USW,BCCW,false);
      setWord(&USW,BCW,false);
      setWord(&MSW,LCLK,false);
      setWord(&JSW,JLEFT,false);
      setWord(&JSW,JRIGHT,false);
      setWord(&JSW,JUP,false);
      setWord(&JSW,JDOWN,false);
      
   }

//*-------------------------------------------------------------------------------
//* PTT==true (RX) and CMD==false (VFO)
//*-------------------------------------------------------------------------------
   if (getWord(MSW,CMD)==false && getWord(MSW,PTT)==true) {      //S=0 VFO Mode   

      
//*----------------------------------------------------------------------------
//*---- Process MULTI Button (VFO Mode)
//*----------------------------------------------------------------------------
      
      if (getWord(USW,BMULTI)==true) { //S (0)->(1) enter CMD mode
         
         setWord(&MSW,CMD,true);
         setWord(&MSW,GUI,false);
         showPanel();
         menuFSM();
         setWord(&USW,BMULTI,false);
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;
      }


//*----------------------------------------------------------------------------
//*---- Erase the GUI update mark after a timeout
//*----------------------------------------------------------------------------     

      if (getWord(TSW,FT4)) {  //Clear GUI clue if present
         lcd.setCursor(0,1);
         lcd.print(" ");
         setWord(&TSW,FT4,false);
      }
      
//*--------------------------------------------------------------------------------------
//*---- Process rotation of VFO encoder   (VFO Mode)
//*--------------------------------------------------------------------------------------
      
      if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=0 operates VFO and clear signals
         processVFO();
      }

      if (vx.isVFOChanged(vx.vfoAB)==true) {
 
#if PICOFM
        
         doSetGroup();
#endif
         
         vx.resetVFO(vx.vfoAB);
       }
       
       setWord(&USW,BCW,false);
       setWord(&USW,BCCW,false);
       return;             

   }
//******************************************************************************************
//* Process menu commands
//******************************************************************************************
//*---- If here is in S=1 or higher (CMD mode)

   if(readButton()==false && getWord(USW,KDOWN)==true && (millis()-menupassed>=SAVE_DELAY) && getWord(MSW,CMD)==true){
     if (getWord(MSW,GUI)==false) {  
         showMark();
     } else {
         showSave();
     }
   }
   
   if (getWord(MSW,GUI)==false && getWord(MSW,PTT)==true) {   //S=1 pure command mode

//*--- Process S=1 transitions
      
      if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
       }

       if (getWord(USW,BMULTI)==true) { //S=1 (1)->(0)
          setWord(&USW,BMULTI,false);
          setWord(&MSW,CMD,false);
          setWord(&MSW,GUI,false);
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }
       
       if (getWord(MSW,LCLK)==true) { //S=1 (1)->(2)
          setWord(&MSW,LCLK,false);
          setWord(&MSW,CMD,true);
          setWord(&MSW,GUI,true);
          backupFSM();
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }
   }

//*---- Only with GUI at this level, check anyway

   if (getWord(MSW,GUI)==true && getWord(MSW,PTT)==true) {

      if (getWord(USW,BCW)== true || getWord(USW,BCCW)== true) { //S=1 operates Menu at first level and clear signals
         menuFSM();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;             
       }
       
       if (getWord(USW,BMULTI)==true) { //S=2 (2)->(1) Restoring
          setWord(&USW,BMULTI,false);
          setWord(&MSW,CMD,true);
          setWord(&MSW,GUI,false);
          restoreFSM();
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }

       if (getWord(MSW,LCLK)==true) { //S=2 (2)->(1) Saving
          setWord(&MSW,LCLK,false);
          setWord(&MSW,CMD,true);
          setWord(&MSW,GUI,false);
          doSave();
          showPanel();
          setWord(&USW,BCW,false);
          setWord(&USW,BCCW,false);
          return;             
       }
   }
       
   setWord(&USW,BCW,false);
   setWord(&USW,BCCW,false);             

}

#if PICOFM
//*****************************************************************************************************
//*                               Manages DRA818F Configuration
//*
//*
//*
//*
//*
//*****************************************************************************************************

//*--------------------------------------------------------------------------------------------
//* doSetGroup
//* send commands related to setup group information
//*--------------------------------------------------------------------------------------------
void doSetGroup() {
  FSTR     v;
  long int fr;
  long int ft;

  if (rpt.get()==0) {  //*--- Is Simplex
      fr=vx.vfo[vx.vfoAB];
      ft=fr;
  } else {             //*--- Is Split
      fr=vx.vfo[vx.vfoAB];
      if (vx.vfoAB==VFOA) {ft=vx.vfo[VFOB];} else {ft=vx.vfo[VFOA];}
  }
  
  
  vx.computeVFO(ft,&v);   
  char hi[40];
  sprintf(hi,"AT+DMOSETGROUP=%1d,%3d.%1d%1d%1d%1d,",bdw.get(),v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds);
  Serial.print(hi);
  
  //f=vx.vfo[vx.vfoAB];
  vx.computeVFO(ft,&v); 
  sprintf(hi,"%3d.%1d%1d%1d%1d,0,%1d,%1d\r\n",v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds,sql.get(),ctc.get());  
  Serial.print(hi);
  
  delay(CMD_DELAY);
  
}
//*--------------------------------------------------------------------------------------------
//* doSetFilter
//* send commands related to setup filter information
//*--------------------------------------------------------------------------------------------
void doSetFilter() {
   
  char hi[30];
  byte pre=PRE;
  byte hpf=HPF;
  byte lpf=LPF;
  sprintf(hi,"AT+SETFILTER=%1d,%1d,%1d\r\n",pre,hpf,lpf);
  Serial.print(hi);
  delay(CMD_DELAY);
  
}
//*--------------------------------------------------------------------------------------------
//* doSetVolume
//* send commands related to setup volume information
//*--------------------------------------------------------------------------------------------
void doSetVolume() {
  
  char hi[20];
  byte v=VOLUME;
  sprintf(hi,"AT+DMOSETVOLUME=%1d\r\n",v);
  Serial.print(hi);
  delay(CMD_DELAY);
}
//*--------------------------------------------------------------------------------------------
//* doScan
//* send commands related to scan frequency information
//*--------------------------------------------------------------------------------------------
void doScan() {
  
}

//*--------------------------------------------------------------------------------------------
//* SQL Update
//* update squelch level text
//*--------------------------------------------------------------------------------------------
void SQLUpdate(){
  
  if (sql.mItem < 8 && sql.CW==true) {
     sql.mItem++;
  }
  if (sql.mItem>0 && sql.CCW==true) {
     sql.mItem--;
  }
  
  char s[]="SQL[0] ";

  sprintf(s,"SQL[%1d]",sql.mItem);
  strcpy(sql.l.get(0)->mText,s);
  showPanel();
  
}
//*--------------------------------------------------------------------------------------------
//* CTCSS Update
//* send commands related to scan frequency information
//*--------------------------------------------------------------------------------------------
void CTCSSUpdate() {

  if (ctc.mItem < CTCSSCODEMAX && ctc.CW == true) {
      ctc.mItem++;
  }
  if (ctc.mItem > 0 && ctc.CCW == true) {
      ctc.mItem--;
  }
  char* s=(char*)"                  "; 
  switch(ctc.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"67.0";break;};
    case 2:                          {s=(char*)"71.9";break;};
    case 3:                          {s=(char*)"74.4";break;};
    case 4:                          {s=(char*)"77.0";break;};
    case 5:                          {s=(char*)"79.7";break;};
    case 6:                          {s=(char*)"82.5";break;};
    case 7:                          {s=(char*)"85.4";break;};
    case 8:                          {s=(char*)"88.5";break;};
    case 9:                          {s=(char*)"91.5";break;};
    case 10:                         {s=(char*)"94.8";break;};
    case 11:                         {s=(char*)"97.4";break;};
    case 12:                         {s=(char*)"100.0";break;};
    case 13:                         {s=(char*)"103.5";break;};
    case 14:                         {s=(char*)"107.2";break;};
    case 15:                         {s=(char*)"110.9";break;};
    case 16:                         {s=(char*)"114.8";break;};   
    case 17:                         {s=(char*)"118.8";break;};
    case 18:                         {s=(char*)"123.0";break;};
    case 19:                         {s=(char*)"127.3";break;};
    default:                         {s=(char*)"131.8";break;};
    case 21:                         {s=(char*)"136.5";break;};
    case 22:                         {s=(char*)"141.3";break;};
    case 23:                         {s=(char*)"146.2";break;};
    case 24:                         {s=(char*)"151.4";break;};
    case 25:                         {s=(char*)"156.7";break;};
    case 26:                         {s=(char*)"162.2";break;};
    case 27:                         {s=(char*)"167.9";break;};
    case 28:                         {s=(char*)"173.8";break;};
    case 29:                         {s=(char*)"179.9";break;};
    case 30:                         {s=(char*)"186.2";break;};
    case 31:                         {s=(char*)"192.8";break;};
    case 32:                         {s=(char*)"203.5";break;};
    case 33:                         {s=(char*)"210.7";break;};
    case 34:                         {s=(char*)"218.1";break;};
    case 35:                         {s=(char*)"225.7";break;};
    case 36:                         {s=(char*)"233.6";break;};
    case 37:                         {s=(char*)"241.8";break;};                             
    case 38:                         {s=(char*)"250.3";break;};
   
  }
  
  ctc.l.get(0)->mText=s;
  showPanel();
  
  return;
  
}

//*--------------------------------------------------------------------------------------------
//* doHandShake
//* send commands related to handshake information
//*--------------------------------------------------------------------------------------------
void doHandShake() {
  
  char hi[20];
  sprintf(hi,"AT+DMOCONNECT\r\n");
  Serial.println(hi);
  delay(CMD_DELAY);
  
}
//*--------------------------------------------------------------------------------------------
//* doSetPower
//* send commands related to set output power
//*--------------------------------------------------------------------------------------------
void doSetPower() {

  if (pwr.get()==0) {
     digitalWrite(HLPin,LOW);
  } else {
     digitalWrite(HLPin,HIGH);
  }
  
}
//*--------------------------------------------------------------------------------------------
//* doSetPD
//* send commands related to set PD mode
//*--------------------------------------------------------------------------------------------
void doSetPD() {
  if (spd.get()==0) {
     digitalWrite(PDPin,LOW);
  } else {
     digitalWrite(PDPin,HIGH);
  }
   
}
#endif


#if SINPLEA
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
#endif
  
  switch(band.mItem) {
    case 0:                          {s=(char*)"Off";break;};                            
    case 1:                          {s=(char*)"160m";break;};
    case 2:                          {s=(char*)"80m";break;}; 
    case 3:                          {s=(char*)"60m";break;}; 
    case 4:                          {s=(char*)"40m";break;}; 
    case 5:                          {s=(char*)"30m";break;}; 
    case 6:                          {s=(char*)"20m";break;}; 
    case 7:                          {s=(char*)"17m";break;}; 
    case 8:                          {s=(char*)"15m";break;}; 
    case 9:                          {s=(char*)"12m";break;}; 
    case 10:                         {s=(char*)"10m";break;}; 
    default:                         {s=(char*)" 6m";break;};   
  }

  vx.setVFOBand(vx.vfoAB,loFreq[band.mItem]*1000,hiFreq[band.mItem]*1000);
  vx.vfo[vx.vfoAB]=loFreq[band.mItem]*1000;

#if DEBUG  
  sprintf(hi,"BandUpdate callback loFreq= %ld hiFreq= %ld",loFreq[band.mItem]*1000,hiFreq[band.mItem]*1000);
  Serial.println(hi);
  
  sprintf(hi,"BandUpdate callback text %s",s);
  Serial.println(hi);
#endif
    
  band.l.get(0)->mText=s;
  showPanel();
  
  return;
  
}
#endif
//*============================================================================================
//*--------------------------------------------------------------------------------------------
//* swapVFO
//* returns the "other" VFO
//*--------------------------------------------------------------------------------------------
byte swapVFO() {

  if (vx.vfoAB==VFOA) {
     return VFOB;
  }
  return VFOA;
  
}
//*--------------------------------------------------------------------------------------------
//* checkPriority
//* send commands related to priority channel management
//*--------------------------------------------------------------------------------------------
void checkPriority() {
  

  
}
//*------------------------------------------------------------------------------------------------------
//* serialEvent is called when a serial port related interrupt happens
//*------------------------------------------------------------------------------------------------------
void serialEvent() {

#if PICOFM

  while (Serial.available() && pQueue<=(QUEUEMAX)-1) {
      
      char inChar = (char)Serial.read();                //get new character from Serial port
      inChar=uppercase(inChar);

      //*---- Skip processing CR,LF and blanks
      switch (inChar) {
        case '\n' : {
             inState=0;
             pQueue=0;
             inCmd=0;
             break;
        }
        case '\r' : {
             pQueue=0;
             inState=0;
             inCmd=0;
             break;
        }
        case ' ' : {
             pQueue=0;
             inState=0;
             inCmd=0;
             break;
          
        }
        //*--- Semicolon means the transition between a command and the value
        case ':' : {
             if (inState==1) {      //*--- if previously received a +DMO the decode a command
              
             
                if (strcmp(serialQueue,"CONNECT")==0) {
                   inCmd=1;
                }
                if (strcmp(serialQueue,"SETGROUP")==0) {
                   inCmd=2;
                }
                if (strcmp(serialQueue,"SETVOLUME")==0) {
                   inCmd=3;
                }
                if (strcmp(serialQueue,"SETFILTER")==0) {
                   inCmd=4;
                }
                
                inState=2;
             }
             //*--- Cleanse the buffer           
             pQueue=0;
             serialQueue[pQueue]=0x00;
             break;
        }
        case '=': {
                if (strcmp(serialQueue,"S")==0) {
                   inCmd=5;
                }
                
                inState=2;
                break;
        //*--- Process a new character from the serial port
        } 
        default: {
           
           inChar=uppercase(inChar);
           serialQueue[pQueue]=inChar;
           serialQueue[pQueue+1]=0x00;

           //*---- if previously received a "." mark then get the variable
           
           if (inState==2) {
              
              //*--- Process according with the previous command
              
              switch(inCmd) {
                case 1: {  
                     //Process CONNECT
                     if (serialQueue[0]=='0') {
                        setWord(&MSW,DRF,true);
                     } else {
                        setWord(&MSW,DRF,false);
                     }
                     showPanel();
                     break;
                }
                case 2: {
                     //Proceso SETGROUP
                     break;
                }
                case 3: {
                     //Proceso SETVOLUME
                     break;
                }
                case 4: {
                     //Proceso SETFILTER
                     break;
                }
                case 5: {
                     if (serialQueue[0]=='0') {
                        setWord(&USW,BUSY,false);
                     } else {
                        setWord(&USW,BUSY,true);
                     }
                     //Proceso FREQUEENCY
                     break;
                }
              }
              pQueue=0;
              inState=0;      
           } else {
             pQueue++;
           }  
           break;     
        }
      }
      
      //*--- detect the hear of the response        
      
      if (strcmp(serialQueue,"+DMO")==0) {
         pQueue=0;
         serialQueue[pQueue]=0x00;
         inState=1;
      }
}

#endif

}
//*****************************************************************************************************
//*                               Manages Meter
//*
//*****************************************************************************************************

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages frequency display at the LCD
//*--------------------------------------------------------------------------------------------------
void LCD_FSM() {


  //*--- Logic to sense the SQL reference, if changed display value and then restore once finished
  
  if (getWord(MSW,CMD)==false){
    
         
  }

 
  
}

//*======================================================================================================
//*                                   Master Loop Control
//*
//*======================================================================================================
void loop() {
 
  
    //*--- Handle commands 
  CMD_FSM();
  //*--- Update LCD Display
  LCD_FSM();
 
  //*--- Write memory to EEPROM when it has been quiet for a while (2 secs)

  if (memstatus == 0) {
    if (timepassed + 2000 < millis()) {
      storeMEM();
    }
  } 
}
//*----------------------------------------[INTERRUPT]---------------------------------------------------
//*======================================================================================================
//*                                   Master Timer Sub-System
//*
//*======================================================================================================
//*--------------------------------------------------------------------------------------------------
//* Timer Interrupt handler
//*      (TIMER1): This is the interrupt handler for TIMER1 set as 1 msec or 1 KHz
//*--------------------------------------------------------------------------------------------------
ISR(TIMER1_OVF_vect) // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{

  TCNT1 = T_1mSec; // preload timer

  
//*--- Serve Timer 1 (T1) 
  if (T1>0) { 
      T1--;
      if (T1==0){
         setWord(&TSW,FT1,true);
      }
  }
//*--- Serve Timer 2 (T2) 
  if (T2>0) { 
      T2--;
      if (T2==0){
         setWord(&TSW,FT2,true);
      }
  }
//*--- Serve Timer 3 (3) 
  if (T3>0) { 
      T3--;
      if (T3==0){
         setWord(&TSW,FT3,true);
      }
  }
//*--- Serve Timer 4 (T4)

  if (T4>0) {
      T4--;
      if (T4==0) {
         setWord(&TSW,FT4,true);         
      }
  }


//*--- Serve Timer Squelch Display (TS)

  if (TS>0) {
      TS--;
      if (TS==0) {
         setWord(&TSW,FTS,true);         
      }
  }

//*--- Serve Timer Priority Channel (TPTY)

  if (TDOG>0) {
      TDOG--;
      if (TDOG==0) {
         setWord(&TSW,FDOG,true);         
      }
  }

//*--- Serve Timer Priority Channel (TPTY)

  if (TBCK>0) {
      TBCK--;
      if (TBCK==0) {
         setWord(&TSW,FBCK,true);         
      }
  }

//*--- LCD Light delay control

#if PICOFM

  if (TDIM>0 && getWord(MSW,PTT)==true) {
      TDIM--;
      if (TDIM==0 && spd.get() == 1) {
         digitalWrite(10,LCD_OFF);
      }
  }
#endif

//*--- Serve real time clock

  if (Tclk>0) {
     Tclk--;
     if (Tclk==0) {
        setWord(&TSW,FCLOCK,true);
        Tclk=1000;
        ss++;
        if (ss==60){
            ss=0;
            mm++;
            if (mm==60) {
                mm=0;
            }
        }

     }
  }
}
//*======================================================================================================
//*                                   Panel Control Sub-System
//*
//*======================================================================================================
//*----------------------------------------[INTERRUPT]---------------------------------------------------
//* System Interrupt Handler
//* Handles interrupt from
//*         * bMulti button
//*         * Rotary encoder Outputs A & B
//*--------------------------------------------------------------------------------------------------
ISR (PCINT1_vect) {

   Encoder_classic();
   
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Encoder_classic
//* interrupt handler to manage rotary button
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Encoder_classic() {

//*----- Check SQL 
  
  if(!digitalRead(A3)) {
       setWord(&USW,SQ,true);
  }
  
  if(!digitalRead(A4)) {
    setWord(&USW,MIC,true);
  }

  if ((digitalRead(A3)) && (getWord(MSW,SQL)==false)) {
       setWord(&USW,SQ,true);    
  }

  if ((digitalRead(A4)) && (getWord(MSW,PTT)==false)) {
       setWord(&USW,MIC,true);    
  }
  
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* Encoder_san
//* interrupt handler to manage rotary button
//--------------------------------------------------------------------------------------------------------------------------------------------------------
 void Encoder_san()
{  
    if(digitalRead(Encoder_B))
    {
       setWord(&USW,BCW,true);     //bRotaryCW  = true;
       Encoder_number++;
    } else {
       setWord(&USW,BCCW,true);    //bRotaryCCW = true;
       Encoder_number--;
    }     
    state=1;
    Encoder_classic();

}
//*--------------------------[System Word Handler]---------------------------------------------------
//* getSSW Return status according with the setting of the argument bit onto the SW
//*--------------------------------------------------------------------------------------------------
boolean getWord (byte SysWord, byte v) {

  return SysWord & v;

}
//*--------------------------------------------------------------------------------------------------
//* setSSW Sets a given bit of the system status Word (SSW)
//*--------------------------------------------------------------------------------------------------
void setWord(byte* SysWord,byte v, boolean val) {

  *SysWord = ~v & *SysWord;
  if (val == true) {
    *SysWord = *SysWord | v;
  }

}
//*----------------------------------------[EEPROM]---------------------------------------------------
//* Store frequency at EEPROM
//*--------------------------------------------------------------------------------------------------

void storeMEM() {


  FSTR v;   
  long int f=vx.vfo[VFOA];
  vx.computeVFO(f,&v);
 
  if (memstatus==1) {return; }

//*=== Recover and store VFO A & B setup
  
  EEPROM.write(1, v.millions);
  EEPROM.write(2, v.hundredthousands);
  EEPROM.write(3, v.tenthousands);
  EEPROM.write(4, v.thousands);
  EEPROM.write(5, v.hundreds);
  EEPROM.write(6, v.tens);
  EEPROM.write(7, v.ones);

  f=vx.vfo[VFOB];
  vx.computeVFO(f,&v); 

  EEPROM.write(8,  v.millions);
  EEPROM.write(9,  v.hundredthousands);
  EEPROM.write(10, v.tenthousands);
  EEPROM.write(11, v.thousands);
  EEPROM.write(12, v.hundreds);
  EEPROM.write(13, v.tens);
  EEPROM.write(14, v.ones);


  EEPROM.write(15, vx.vfoAB);
  EEPROM.write(18,stp.get());

//*---- picoFM specific

#if PICOFM  
  EEPROM.write(0,sql.get()); 
  EEPROM.write(16,pwr.get());
  EEPROM.write(17,wdg.get());
  EEPROM.write(19,rpt.get());
  EEPROM.write(20,spd.get());
  EEPROM.write(21,bdw.get());
  /*
  EEPROM.write(22,hpf.get());
  EEPROM.write(23,lpf.get());
  EEPROM.write(24,pre.get());
  */
  EEPROM.write(25,ctc.get());
#endif

//*---- sinpleA specific
#if SINPLEA
  
  EEPROM.write(31,band.get());

  for (int i=0; i <= BANDMAX; i++){
      EEPROM.write(i+32,bandvfo[i]);
  }
#endif
  
  EEPROM.write(26,vfo.get());  
  EEPROM.write(27,MSW);
  EEPROM.write(28,USW);
  EEPROM.write(29,TSW);

  EEPROM.write(30,EEPROM_COOKIE);
  memstatus = 1;  // Let program know memory has been written
};

#if DEBUG
//*------------------------------------------------------------------------------------------------------
//* utility to convert a string with a Hex number ('0xnn') into an actual Hex number (0xnn)
//*------------------------------------------------------------------------------------------------------
int x2i(String Hex) 
{
 int x = 0;
 for (int i=0; i <= Hex.length()-1; i++) {
 
   char c = Hex[i];
   c=uppercase(c);
   if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0'; 
   }
   else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10; 
   }
   else break;
 }
 return x;
}
//*------------------------------------------------------------------------------------------------------
//* Encode a decimal (byte) number into BCD cambiar a bcd_encode
//*------------------------------------------------------------------------------------------------------
uint8_t byte2bcd(uint8_t bx)
{
  return (bx/10)*16 + (bx%10);
}

//*------------------------------------------------------------------------------------------------------
//* Decode a BCD coded number back to an integer
//*------------------------------------------------------------------------------------------------------
byte bcd2byte(byte bcd) {

    return (((bcd >> 4)*10)+(bcd & 0xf));
  
}
#endif

//*-------------------------------------------------------------------------------------------------------
//* uppercase a character 
//*-------------------------------------------------------------------------------------------------------
int uppercase (int charbytein)
{
  if (((charbytein > 96) && (charbytein < 123)) || ((charbytein > 223) && (charbytein < 255))) {
    charbytein = charbytein - 32;
  }
  if (charbytein == 158) { charbytein = 142; }  // Å¾ -> Å½
  if (charbytein == 154) { charbytein = 138; }  // Å¡ -> Å 
  
  return charbytein;
}

//*=======================================================================================================================================================
//* Super Library ELEC Freaks LCD Key Shield
//*=======================================================================================================================================================
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//* read_LCD_buttons()
//* read the buttons
//--------------------------------------------------------------------------------------------------------------------------------------------------------
boolean readButton() {
 
 int v = analogRead(A0);      
 if (v>1000) {return true;} else {return false;}
 

}

