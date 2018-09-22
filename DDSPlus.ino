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


//*--- Program & version identification

#if PICOFM

#define PROGRAMID "picoFM"
#define PROG_VERSION   "1.0"
#define PROG_BUILD  "030"
#define COPYRIGHT "(c) LU7DID 2018"

#endif

#if SINPLEA

#define PROGRAMID "sinpleA"
#define PROG_VERSION   "1.0"
#define PROG_BUILD  "000"
#define COPYRIGHT "(c) LU7DID 2018"

#endif


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
#define BUSY      B10000000

//*----- Joystick Control Word (JSW)

#define JLEFT     B00000001
#define JRIGHT    B00000010
#define JUP       B00000100
#define JDOWN     B00001000

#if PICOFM
//*------------------------------------------------------------------------------------------------------------
//*--- Read Squelch control
//*------------------------------------------------------------------------------------------------------------

#define ZERO             0
#define SERIAL_MAX      16
#define VOLUME           5
#define EEPROM_COOKIE  0x1f

#define HPF 0
#define LPF 0
#define PRE 0

#endif

//*-----------------------------------------------------------------------------------------------
//* Control lines and VFO Definition [Project dependent]
//*-----------------------------------------------------------------------------------------------
#if PICOFM


//*--- VFO initialization parameters

#define VFO_SHIFT          600000
#define VFO_START       144000000
#define VFO_END         147990000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_1MHZ     1000000

#endif

//*-----------------------------------------------------------------------------------------------
//* Control lines and VFO Definition [Project dependent]
//*-----------------------------------------------------------------------------------------------
#if SINPLEA

#define VFO_SHIFT            1000
#define VFO_START         7000000
#define VFO_END           7300000
#define VFO_STEP_1KHz        1000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_1MHZ     1000000
#define VFO_RESET         7000000

#endif
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


//*--------------------------------------------------------------------------------------------------
//* Definitions to manage DRA818V
//*--------------------------------------------------------------------------------------------------
#if PICOFM

#define CTCSSCODEMAX 38

//*--- Control lines for the DRA818V

#define PTTPin          13
#define HLPin           12
#define PDPin           11

#endif


//*----------------------------------------[INCLUDE]-------------------------------------------------
#include <LiquidCrystal.h>
#include <stdio.h>
#include <EEPROM.h>

#include "MemSizeLib.h"
#include "VFOSystem.h"
#include "ClassMenu.h"

void  Encoder_san();

//*-------------------------------------------------------------------------------------------------
//* Define class to manage VFO
//*-------------------------------------------------------------------------------------------------
void showFreq();   //Prototype fur display used on callback
VFOSystem vx(showFreq,NULL);


#if PICOFM
//*==============================================================================================================================
//*    DefiniciÃ²n de custom characters
//*==============================================================================================================================
byte BB3[8] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
};

 
byte BB5[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte BB1[8] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
};
byte BB2[8] = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
};
byte BB4[8] = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
};

byte TX[8] = {
  B11111,
  B10001,
  B11011,
  B11011,
  B11011,
  B11011,
  B11111,
};

byte WATCHDOG[8] = {
  B10001,
  B10001,
  B01010,
  B00100,
  B01010,
  B10001,
  B10001,
};

byte RX[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
};

#endif


#if SINPLEA
//*=======================================================================================================================================================
//* SI5351 Library
//*=======================================================================================================================================================
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_SI5351.h>

Adafruit_SI5351 clkVFO = Adafruit_SI5351();
#endif

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
        
//*---------------------- Custom fonts for the LCD display

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

MenuClass menuRoot(NULL);

#if PICOFM

void CTCSSUpdate();
void SQLUpdate();

#define PWRMENU 0
#define RPTMENU 1
#define SPDMENU 2
#define BDWMENU 3
/*
#define HPFMENU 4
#define LPFMENU 5
#define PREMENU 6
*/
#define TONMENU 4
#define CTCMENU 5
#define VFOMENU 6
#define STPMENU 7
#define WDGMENU 8
#define SQLMENU 9

MenuClass pwr(NULL);
MenuClass rpt(NULL);
MenuClass spd(NULL);
MenuClass bdw(NULL);
/*
MenuClass hpf(NULL,NULL,NULL);
MenuClass lpf(NULL,NULL,NULL);
MenuClass pre(NULL,NULL,NULL);
*/
MenuClass ton(NULL);
MenuClass ctc(CTCSSUpdate);
MenuClass vfo(NULL);
MenuClass stp(NULL);
MenuClass wdg(NULL);
MenuClass sql(SQLUpdate);

//*------------------------------------------------------------------------------------------------
//* Init meter management objects
//*------------------------------------------------------------------------------------------------
//Meter sqlMeter;
//Meter pwrMeter;

#endif

//*------------------------------------------------------------------------------------------------
//* Set here SINPLEA Menu definitions
//*------------------------------------------------------------------------------------------------

#if SINPLEA


#endif
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
   
  //========================================
  attachInterrupt(1, Encoder_san, FALLING);        //interrupts: numbers 0 (on digital pin 2) and 1 (on digital pin 3).

//*-------------------------------------------------------------------------------
//* I/O Definitions for the DRA818V
//*-------------------------------------------------------------------------------
#if PICOFM

  pinMode(A3, INPUT);         //SQ Squelch signal (0=Open;1=Closed)
  pinMode(A4, INPUT);         //PTT MIC (0=Active,1=Inactive) 
  //pinMode(A5, INPUT);         //Squelch level

  
//*--- Setup ledPIN (future Keyer)
  
  pinMode(PDPin, OUTPUT);
  pinMode(HLPin, OUTPUT);
  pinMode(PTTPin, OUTPUT);

//*--- Enable pull-up resistors

  digitalWrite(A3, INPUT);     //Enable internal pull-up resistor for Squelch
  digitalWrite(A4, INPUT);     //Enable internal pull-up resistor for PTT Keyer

#endif

#if PICOFM
//*--- Create special characters for LCD

  lcd.createChar(0,RX);
  lcd.createChar(1,BB1);
  lcd.createChar(2,BB2);
  lcd.createChar(3,BB3);
  lcd.createChar(4,BB4);
  lcd.createChar(5,BB5);
  lcd.createChar(6,TX);
  lcd.createChar(7,WATCHDOG);
#endif
  
  lcd.setCursor(0, 0);        // Place cursor at [0,0]
  lcd.print(String(PROGRAMID)+" v"+String(PROG_VERSION)+"-"+String(PROG_BUILD));

  lcd.setCursor(0, 1);        // Place cursor at [0,1]
  lcd.print(String(COPYRIGHT));

  delay(DELAY_DISPLAY);
  lcd.clear();
/*
//*------  Define sql meter
  
  sqlMeter.name =(char*)"sql";
  sqlMeter.pin  = 5;
  sqlMeter.vmax = 1023;
  sqlMeter.vref = 5.0;
  sqlMeter.vscale = 3;
  sqlMeter.v=0.0;
  sqlMeter.vant=0.0;
*/
//*------ Define Menu related objects

 

  //*---- Define the VFO System parameters

  //Serial.print("Init VFO System");
  //Serial.println(VFOA);

  vx.setVFOFreq(VFOA,VFO_START);
  vx.setVFOStep(VFOA,VFO_STEP_1KHz);
  vx.setVFOBand(VFOA,VFO_START,VFO_END);

  vx.setVFOFreq(VFOB,VFO_START);
  vx.setVFOStep(VFOB,VFO_STEP_1KHz);
  vx.setVFOBand(VFOB,VFO_START,VFO_END);

  vx.setVFO(VFOA);



  //*---

//*--- Initialization of Squelch meter 
//sqlMeter.v=readMeter(&sqlMeter);  
 
    
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

#if PICOFM

//*---- Setup Master system menus

  menuRoot.add((char*)"Power",&pwr);
  menuRoot.add((char*)"Split",&rpt);
  menuRoot.add((char*)"SPD",&spd);
  menuRoot.add((char*)"BDW",&bdw);
  /*
  menuRoot.add((char*)"HPF",&hpf);
  menuRoot.add((char*)"LPF",&lpf);
  menuRoot.add((char*)"Pre",&pre);
  */
  menuRoot.add((char*)"Tone",&ton);
  menuRoot.add((char*)"CTCSS",&ctc);
  menuRoot.add((char*)"VFO",&vfo);
  menuRoot.add((char*)"Step",&stp);
  menuRoot.add((char*)"Watchdog",&wdg);
  menuRoot.add((char*)"Squelch",&sql); 

  pwr.add((char*)"Low",NULL);
  pwr.add((char*)"High",NULL);

  rpt.add((char*)"Off",NULL);
  rpt.add((char*)"On",NULL);
  //rpt.add((char*)"-",NULL);

  spd.add((char*)"Off",NULL);
  spd.add((char*)"On",NULL);
  
  bdw.add((char*)"12.5 KHz",NULL);
  bdw.add((char*)"25.0 KHz",NULL);
/*
  hpf.add((char*)"Off",NULL);
  hpf.add((char*)"On",NULL);

  lpf.add((char*)"Off",NULL);
  lpf.add((char*)"On",NULL);

  pre.add((char*)"Off",NULL);
  pre.add((char*)"On",NULL);
*/  
  ton.add((char*)"Off",NULL);
  ton.add((char*)"On",NULL);
  
  ctc.add((char*)"Off   ",NULL);

  vfo.add((char*)"A",NULL);
  vfo.add((char*)"B",NULL);

  stp.add((char*)" 5 KHz",NULL);
  stp.add((char*)"10 KHz",NULL);
  
  wdg.add((char*)"On ",NULL);
  wdg.add((char*)"Off",NULL);

  sql.add((char*)"SQL[0]",NULL);
/*
  sql.add((char*)"[0]",NULL);
  sql.add((char*)"[1]",NULL); 
  sql.add((char*)"[2]",NULL);  
  sql.add((char*)"[3]",NULL); 
  sql.add((char*)"[4]",NULL);
  sql.add((char*)"[5]",NULL);
  sql.add((char*)"[6]",NULL);
  sql.add((char*)"[7]",NULL);
  sql.add((char*)"[8]",NULL);
*/
  
#endif

//#*--------------------------------------------------------------------
//#* Set here SINPLEA Menu Items
//#*--------------------------------------------------------------------
#if SINPLEA


#endif
  //*--- Load the stored frequency

#if PICOFM

  if (FORCEFREQ == 0) {

     if (EEPROM.read(30)==EEPROM_COOKIE) {
            sql.set(EEPROM.read(0));
            char hi[12];
     
            sprintf(hi,"%3d%1d%1d%1d%1d%1d%1d",EEPROM.read(1),EEPROM.read(2),EEPROM.read(3),EEPROM.read(4),EEPROM.read(5),EEPROM.read(5),EEPROM.read(6),EEPROM.read(7));
            vx.vfo[VFOA]=String(hi).toInt();
 
            sprintf(hi,"%3d%1d%1d%1d%1d%1d%1d",EEPROM.read(8),EEPROM.read(9),EEPROM.read(10),EEPROM.read(11),EEPROM.read(12),EEPROM.read(13),EEPROM.read(14));
            vx.vfo[VFOB]=String(hi).toInt();
            
            
            vx.vfoAB = EEPROM.read(15);        

            pwr.set(EEPROM.read(16));
            wdg.set(EEPROM.read(17));
            stp.set(EEPROM.read(18));
            rpt.set(EEPROM.read(19));
            spd.set(EEPROM.read(20));
            bdw.set(EEPROM.read(21));
            //hpf.set(EEPROM.read(22));
            //lpf.set(EEPROM.read(23));
            //pre.set(EEPROM.read(24));
            ctc.set(EEPROM.read(25));
            vfo.set(EEPROM.read(26));

            MSW = EEPROM.read(27);
            USW = EEPROM.read(28);
            TSW = EEPROM.read(29);
     }
  } 

  
  //if (vx.vfo[VFOA]<VFO_START || vx.vfo[VFOA]>VFO_END) {vx.vfo[VFOA]=VFO_START;}
  //if (vx.vfo[VFOB]<VFO_START || vx.vfo[VFOB]>VFO_END) {vx.vfo[VFOB]=VFO_START;}

#endif

//*--- Initial value for system operating modes

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

  setWord(&JSW,JLEFT,false);
  setWord(&JSW,JRIGHT,false);
  setWord(&JSW,JUP,false);
  setWord(&JSW,JDOWN,false);


//*--------- Attempt to establish contact with the DRA018F module


#if PICOFM

  doHandShake();
  doSetVolume();
  doSetFilter();
  doSetGroup();

#endif

#if SINPLEA

//**------set here last minute initialization grubblets

#endif
//#*--- Define VFO

#if SINPLEA
  
  if (clkVFO.begin() != ERROR_NONE)
  {
     Serial.print("Error");
     while(1);
  }
  Serial.println("OK");
  clkVFO.enableOutputs(true);
  clkVFO.setupPLL(SI5351_PLL_A, 36, 0, 1000); //900 MHz
  //setSI5351freq (6000);

#endif

  showPanel();
  
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

}
/*
//*---------------------------------------------------------------------------------------------------------------
//* Test code to print a given menu class
//*---------------------------------------------------------------------------------------------------------------
void printList(MenuClass* r){
  char hi[80];
  
  for(int i = 0; i < r->l.size(); i++){

    sprintf(hi,"Class Object index[%2d] mItem[%2d] Text[%s] Size[%1d]",i,r->mItem,r->l.get(i)->mText,r->l.size());
    Serial.println(hi);
  
   }

}

*/
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
  
  sprintf(hi,"Frequency %ld",f);
  Serial.println(hi);
  sprintf(hi,"FrequencyDDS %ld",fDDS);
  Serial.println(hi);

//*---- Set DDS with new frequency

  setDDSfreq(fDDS);

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
  
  lcd.print(" ");
  lcd.print(v.hundreds);
  lcd.print(v.tens);

#endif

  
  //Serial.print(v.millions);
  //Serial.print(".");
  //Serial.print(v.hundredthousands);
  //Serial.print(v.tenthousands);
  //Serial.print(v.thousands);
  
  timepassed = millis();
  memstatus = 0; // Trigger memory write

};
//*--------------------------------------------------------------------------------------------
//* showRpt
//* show repeater operation mode at the display
//*--------------------------------------------------------------------------------------------
void showRpt() {

#if PICOFM
  
  lcd.setCursor(0,0);
  if (rpt.get()==0){lcd.print(" ");} else {lcd.print("S");}
  //lcd.print(rpt.getText(rpt.get()));

#endif

};
//*--------------------------------------------------------------------------------------------
//* showCTC
//* show CTCSS operation mode at the display
//*--------------------------------------------------------------------------------------------
void showCTC() {

#if PICOFM

  lcd.setCursor(1,0);
  if (ctc.get()==0){lcd.print(" ");} else {lcd.print("T");}

#endif
};
//*--------------------------------------------------------------------------------------------
//* showPwr
//* show power level at the display
//*--------------------------------------------------------------------------------------------
void showPwr() {

#if PICOFM
  
  lcd.setCursor(11,0);
  if (pwr.get()==0) {lcd.print("L");} else {lcd.print("H");}
  return;

#endif

}
//*--------------------------------------------------------------------------------------------
//* showMet
//* show meter
//*--------------------------------------------------------------------------------------------
void showMet() {

#if PICOFM

     //showMeter(&sqlMeter,sqlMeter.v);
     return;
#endif

}
//*--------------------------------------------------------------------------------------------
//* showSPD
//* show HILO operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSPD() {

#if PICOFM

  lcd.setCursor(5,0);
  if (spd.get()==0) {lcd.print(" ");} else {lcd.print("Z");}

#endif

};

//*--------------------------------------------------------------------------------------------
//* showSQL
//* show SQL operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSQL() {

#if PICOFM

  lcd.setCursor(12,0);
  if (getWord(MSW,CMD)==true) {return;}
  if (digitalRead(A3)==LOW && getWord(MSW,CMD)==false) {lcd.write(byte(5));} else {lcd.write(byte(0));}

#endif
  
};
//*--------------------------------------------------------------------------------------------
//* showPTT
//* show PTT operation mode at the display
//*--------------------------------------------------------------------------------------------
void showPTT() {

#if PICOFM

  lcd.setCursor(4,0);
  if (getWord(MSW,DOG)==true && wdg.get()!=0){lcd.write(byte(7)); lcd.setCursor(4,0);lcd.blink();return;}
  if (getWord(MSW,PTT)==false) {lcd.write(byte(6)); lcd.setCursor(4,0);lcd.noBlink();} else {lcd.print(" ");lcd.setCursor(4,0);lcd.noBlink();}
#endif


};
/*
//*--------------------------------------------------------------------------------------------
//* showPre
//* show pre-emphasis filter at the display
//*--------------------------------------------------------------------------------------------
void showPre() {
  
  lcd.setCursor(8,0);
  if (pre.get()==0){lcd.print(" ");} else {lcd.print("P");}

};
//*--------------------------------------------------------------------------------------------
//* showHPF
//* show HPF filter at the display
//*--------------------------------------------------------------------------------------------
void showHPF() {
  
  lcd.setCursor(7,0);
  if (hpf.get()==0){lcd.print(" ");} else {lcd.print("H");}

};
//*--------------------------------------------------------------------------------------------
//* showLPF
//* show LPF filter at the display
//*--------------------------------------------------------------------------------------------
void showLPF() {
  
  lcd.setCursor(6,0);
  if (lpf.get()==0){lcd.print(" ");} else {lcd.print("L");}

};

*/
//*--------------------------------------------------------------------------------------------
//* showDRF
//* show DRF filter at the display
//*--------------------------------------------------------------------------------------------
void showDRF() {

#if PICOFM
  
  lcd.setCursor(3,0);
  if (getWord(MSW,DRF)==false){lcd.print(char(174));} else {lcd.print(char(42));}

#endif

};
//*--------------------------------------------------------------------------------------------
//* showDog
//* show if watchdog is enabled or not
//*--------------------------------------------------------------------------------------------
void showDog() {

#if PICOFM
  
  lcd.setCursor(9,0);
  if (wdg.get()==0){lcd.print(" ");} else {lcd.print("W");}
  
#endif

};


//*--------------------------------------------------------------------------------------------
//* showVFO
//* show VFO filter at the display
//*--------------------------------------------------------------------------------------------
void showVFO() {
  
  lcd.setCursor(2,0);
  if (vx.vfoAB==VFOA){lcd.print("A");} else {lcd.print("B");}

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
      showPwr();
      showRpt();
      showCTC();
      showDog();
      /*
      showPre();
      showHPF();
      showLPF();
      */
      showSQL();
      showPTT();
      showVFO();
      showSPD();
      showDRF();
      //showMet();
      
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
      //char hi[80];
      //sprintf(hi,"item=%1d",menuRoot.get());
      //Serial.println(hi);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(menuRoot.get())+"> "+String(menuRoot.getText(menuRoot.get())));
      lcd.setCursor(0,1);
      lcd.print(">");
      lcd.print("  "+String(z->getCurrentText()));
      return;
   }

}

void showMark(){
      lcd.setCursor(0,1);
      lcd.print(">"); 
}
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
//*--------------------------------------------------------------------------------------------
//* doSave
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void doSave() {

      showSave();      
      delay(DELAY_SAVE);

#if PICOFM
      
      byte i=menuRoot.get();
      MenuClass* z=menuRoot.l.get(i)->mChild;
      byte j=z->mItem;
      byte k=z->mItemBackup;
   
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
     /* 
      if ( (menuRoot.get() == HPFMENU ||
            menuRoot.get() == LPFMENU ||
            menuRoot.get() == PREMENU ) && (j!=k)) {doSetFilter();}

      */
      
      vx.vfoAB=vfo.get();
      Serial.println("Get VfoAB");
      Serial.println(vx.vfoAB);
#endif

#if PICOFM
      //if (pwr.get()==0){pwrMeter.v=2.0;} else {pwrMeter.v=5.0;}
      if (stp.get()==0) {
         vx.vfostep[vx.vfoAB]=VFO_STEP_5KHz;
      } else {
         vx.vfostep[vx.vfoAB]=VFO_STEP_10KHz;
      }   

      setWord(&MSW,CMD,false);
      setWord(&MSW,GUI,false);

      menuRoot.save();

#endif 
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
#endif              
              showFreq();
              setWord(&MSW,DOG,false);
              showPTT();
          
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
         //Serial.println("call to processVFO");
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

#if PICOFM

  FSTR v;   
  long int f=vx.vfo[VFOA];
  vx.computeVFO(f,&v);
 
  if (memstatus==1) {return; }
  EEPROM.write(0,sql.get()); 
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
  
  EEPROM.write(16,pwr.get());
  EEPROM.write(17,wdg.get());
  EEPROM.write(18,stp.get());
  EEPROM.write(19,rpt.get());
  EEPROM.write(20,spd.get());
  EEPROM.write(21,bdw.get());
  /*
  EEPROM.write(22,hpf.get());
  EEPROM.write(23,lpf.get());
  EEPROM.write(24,pre.get());
  */
  EEPROM.write(25,ctc.get());
  EEPROM.write(26,vfo.get());
  
  EEPROM.write(27,MSW);
  EEPROM.write(28,USW);
  EEPROM.write(29,TSW);

  EEPROM.write(30,EEPROM_COOKIE);

#endif

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
 
 //return digitalRead(11); 
 //if(digitalRead(11)==0) return falseEncodeOK;   
}
#if SINPLEA

//*==============================================================================================================
//* Set DDS Frequency fur SI5351 
//* Receive frequency to set expressed in KHz
//*==============================================================================================================
void setDDSfreq (unsigned long freq)
{

 //*---- trace code make into DEBUG later
 sprintf(hi,"DDS frequency= %ld",freq);
 Serial.println(hi);
 //*----
 
 unsigned long f2;
 unsigned long f3;
 unsigned long f4;
 unsigned long f5;
 unsigned long div2;
 unsigned int Divider2;
 unsigned int rdiv;
 
 if (freq > 0) {
    f2=(freq-12)*4;
    if (f2<1000) {
       rdiv = 16;
       f2 = f2 * 16;
    }  else {
       rdiv = 1;
    }
 
 div2 = 900000000/f2;
 f4 = div2/1000;
 f5=div2-(f4*1000);
 clkVFO.setupMultisynth(1, SI5351_PLL_A, f4, f5,1000);
 
 if (rdiv == 16) {
    clkVFO.setupRdiv(1, SI5351_R_DIV_16);
 }
 if (rdiv == 1) {
    clkVFO.setupRdiv(1, SI5351_R_DIV_1);
 }
}
}
#endif
