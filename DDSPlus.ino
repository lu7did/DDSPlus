//*--------------------------------------------------------------------------------------------------
//* DDSPlus Firmware Version 1.0
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o picoFM en su version inicial
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
#define PICOFM        true


//*--- Program & version identification

#if PICOFM

#define PROGRAMID "picoFM"
#define PROG_VERSION   "1.0"
#define PROG_BUILD  "027"
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

//*------------------------------------------------------------------------------------------------------------
//*--- Read Squelch control
typedef struct {
    char* name;
    byte  pin;
    word  vmax;
    word  vscale;
    float v;
    float vref;
    float vant;
    
} Meter;

#define SQLPIN           A5
#define SQLMAX         1023
#define SQLSCALE          8
#define SQLREF          5.0


#define ZERO             0
#define SERIAL_MAX      16
#define VOLUME           5
#define EEPROM_COOKIE  0x1f

//*-----------------------------------------------------------------------------------------------
//* Control lines and VFO Definition
//*-----------------------------------------------------------------------------------------------
#if PICOFM


//*--- VFO initialization parameters

#define VFO_SHIFT       600000
#define VFO_START    144000000
#define VFO_END      147990000
#define VFO_STEP_10KHz   10000
#define VFO_STEP_5KHz     5000
#define VFO_STEP_1MHZ  1000000

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

#define FORCEFREQ     0
#define LCD_ON        1
#define LCD_OFF       0


//*--------------------------------------------------------------------------------------------------
//* Definitions to manage DRA818V
//*--------------------------------------------------------------------------------------------------
#if PICOFM

#define CTCSSCODEMAX 20
#define QUEUEMAX  16        // Queue of incoming characters 

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

MenuClass menuRoot(NULL,NULL,NULL);

#if PICOFM

void CTCSSUpdate();
void CTCSSDisplay(); 
byte ctcssvalue=0;

#define PWRMENU 0
#define RPTMENU 1
#define SPDMENU 2
#define BDWMENU 3
#define HPFMENU 4
#define LPFMENU 5
#define PREMENU 6
#define TONMENU 7
#define CTCMENU 8
#define VFOMENU 9
#define STPMENU 10
#define WDGMENU 11

MenuClass pwr(NULL,NULL,NULL);
MenuClass rpt(NULL,NULL,NULL);
MenuClass spd(NULL,NULL,NULL);
MenuClass bdw(NULL,NULL,NULL);
MenuClass hpf(NULL,NULL,NULL);
MenuClass lpf(NULL,NULL,NULL);
MenuClass pre(NULL,NULL,NULL);
MenuClass ton(NULL,NULL,NULL);
MenuClass ctc(CTCSSUpdate,NULL,NULL);
MenuClass vfo(NULL,NULL,NULL);
MenuClass stp(NULL,NULL,NULL);
MenuClass wdg(NULL,NULL,NULL);

//*------------------------------------------------------------------------------------------------
//* Init meter management objects
//*------------------------------------------------------------------------------------------------
Meter sqlMeter;

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
  pinMode(A5, INPUT);         //Squelch level

  
//*--- Setup ledPIN (future Keyer)
  
  pinMode(PDPin, OUTPUT);
  pinMode(HLPin, OUTPUT);
  pinMode(PTTPin, OUTPUT);

//*--- Enable pull-up resistors

  digitalWrite(A3, INPUT);     //Enable internal pull-up resistor for Squelch
  digitalWrite(A4, INPUT);     //Enable internal pull-up resistor for PTT Keyer

#endif

//*--- Create special characters for LCD

  lcd.createChar(0,RX);
  lcd.createChar(1,BB1);
  lcd.createChar(2,BB2);
  lcd.createChar(3,BB3);
  lcd.createChar(4,BB4);
  lcd.createChar(5,BB5);
  lcd.createChar(6,TX);
  lcd.createChar(7,WATCHDOG);
  
  lcd.setCursor(0, 0);        // Place cursor at [0,0]
  lcd.print(String(PROGRAMID)+" v"+String(PROG_VERSION)+"-"+String(PROG_BUILD));

  lcd.setCursor(0, 1);        // Place cursor at [0,1]
  lcd.print(String(COPYRIGHT));

  delay(DELAY_DISPLAY);
  lcd.clear();

//*------  Define sql meter
  
  sqlMeter.name =(char*)"sql";
  sqlMeter.pin  = A5;
  sqlMeter.vmax = 1023;
  sqlMeter.vref = 5.0;
  sqlMeter.vscale = 3;
  sqlMeter.v=0.0;
  sqlMeter.vant=0.0;


//*------ Define Menu related objects

 
#if PICOFM

  //*---- Define the VFO System parameters
  
  vx.setVFOFreq(VFOA,VFO_START);
  vx.setVFOShift(VFOA,VFO_SHIFT);
  vx.setVFOStep(VFOA,VFO_STEP_10KHz);
  vx.setVFOBand(VFOA,VFO_START,VFO_END);

  vx.setVFOFreq(VFOB,VFO_START);
  vx.setVFOShift(VFOB,VFO_SHIFT);
  vx.setVFOStep(VFOB,VFO_STEP_10KHz);
  vx.setVFOBand(VFOB,VFO_START,VFO_END);

  vx.setVFO(VFOA);

#endif

  //*---

//*--- Initialization of Squelch meter 
sqlMeter.v=readMeter(&sqlMeter);  
 
    
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
  menuRoot.add((char*)"Rpt",&rpt);
  menuRoot.add((char*)"SPD",&spd);
  menuRoot.add((char*)"BDW",&bdw);
  menuRoot.add((char*)"HPF",&hpf);
  menuRoot.add((char*)"LPF",&lpf);
  menuRoot.add((char*)"Pre",&pre);
  menuRoot.add((char*)"Tone",&ton);
  menuRoot.add((char*)"CTCSS",&ctc);
  menuRoot.add((char*)"VFO",&vfo);
  menuRoot.add((char*)"Step",&stp);
  menuRoot.add((char*)"Watchdog",&wdg);
  

  pwr.add((char*)"Low",NULL);
  pwr.add((char*)"High",NULL);

  rpt.add((char*)"S",NULL);
  rpt.add((char*)"+",NULL);
  rpt.add((char*)"-",NULL);

  spd.add((char*)"Off",NULL);
  spd.add((char*)"On",NULL);

  bdw.add((char*)"12.5 KHz",NULL);
  bdw.add((char*)"25.0 KHz",NULL);

  hpf.add((char*)"Off",NULL);
  hpf.add((char*)"On",NULL);

  lpf.add((char*)"Off",NULL);
  lpf.add((char*)"On",NULL);

  pre.add((char*)"Off",NULL);
  pre.add((char*)"On",NULL);
  
  ton.add((char*)"Off",NULL);
  ton.add((char*)"On",NULL);
  
  ctc.add((char*)"Off   ",NULL);

  vfo.add((char*)"A",NULL);
  vfo.add((char*)"B",NULL);

  stp.add((char*)" 5 KHz",NULL);
  stp.add((char*)"10 KHz",NULL);
  
  wdg.add((char*)"On ",NULL);
  wdg.add((char*)"Off",NULL);

#endif

  //*--- Load the stored frequency

  if (FORCEFREQ == 0) {

     if (EEPROM.read(30)==EEPROM_COOKIE) {
     //sqlMeter.vSQLSig = String(EEPROM.read(0)).toInt();

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
            hpf.set(EEPROM.read(22));
            lpf.set(EEPROM.read(23));
            pre.set(EEPROM.read(24));
            ctc.set(EEPROM.read(25));
            vfo.set(EEPROM.read(26));

            MSW = EEPROM.read(27);
            USW = EEPROM.read(28);
            TSW = EEPROM.read(29);
     }
  } 

  if (vx.vfo[VFOA]<VFO_START || vx.vfo[VFOA]>VFO_END) {vx.vfo[VFOA]=VFO_START;}
  if (vx.vfo[VFOB]<VFO_START || vx.vfo[VFOB]>VFO_END) {vx.vfo[VFOB]=VFO_START;}

//*--- Initial value for system operating modes

  setWord(&MSW,CMD,false);
  setWord(&MSW,GUI,false);
  setWord(&MSW,PTT,true);
  setWord(&MSW,DRF,false);
  setWord(&MSW,DOG,false);
  setWord(&MSW,LCLK,false);
  setWord(&MSW,SQL,true);

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

  showPanel();
#endif


  
//*=========================================================================================================
#if DEBUG
char hi[80];

//*--- Print Serial Banner (TEST Mode Mostly)
  sprintf(hi,"%s %s Compiled %s %s",PROGRAMID,PROG_VERSION,__TIME__, __DATE__);
  Serial.println(hi);
  sprintf(hi,"(c) %s",COPYRIGHT);
  Serial.print("RAM Free=");
  Serial.println(freeMemory());
#endif
//*=========================================================================================================

}
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
  
  if (getWord(MSW,PTT)==false) {
     f=getFreq(f,vx.vfoshift[vx.vfoAB],rpt.get());
  }
  //Serial.println(f);
  vx.computeVFO(f,&v);
  
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
  timepassed = millis();
  memstatus = 0; // Trigger memory write

};
//*--------------------------------------------------------------------------------------------
//* showRpt
//* show repeater operation mode at the display
//*--------------------------------------------------------------------------------------------
void showRpt() {
  
  lcd.setCursor(0,0);
  lcd.print(rpt.getText(rpt.get()));

};
//*--------------------------------------------------------------------------------------------
//* showCTC
//* show CTCSS operation mode at the display
//*--------------------------------------------------------------------------------------------
void showCTC() {

  lcd.setCursor(1,0);
  if (ctcssvalue==0){lcd.print(" ");} else {lcd.print("T");}

};
//*--------------------------------------------------------------------------------------------
//* showPwr
//* show power level at the display
//*--------------------------------------------------------------------------------------------
void showPwr() {
  
  lcd.setCursor(13,1);
  
  
  if (pwr.get()==0){
     lcd.write(byte(5));
     lcd.setCursor(14,1);
     lcd.write(byte(3)); 
  } else {
     lcd.write(byte(byte(5)));
     lcd.setCursor(14,1);
     lcd.write(byte(byte(5)));
     lcd.setCursor(15,1);
     lcd.write(byte(byte(5)));
  }   
}
//*--------------------------------------------------------------------------------------------
//* showMet
//* show meter
//*--------------------------------------------------------------------------------------------
void showMet() {

     showMeter(&sqlMeter,sqlMeter.v);
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

  lcd.setCursor(13,0);
  if (digitalRead(A3)==LOW) {lcd.write(byte(5));} else {lcd.write(byte(0));}
  
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

//*--------------------------------------------------------------------------------------------
//* showLPF
//* show LPF filter at the display
//*--------------------------------------------------------------------------------------------
void showLPF() {
  
  lcd.setCursor(6,0);
  if (lpf.get()==0){lcd.print(" ");} else {lcd.print("L");}

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
      showPre();
      showHPF();
      showLPF();
      showSQL();
      showPTT();
      showVFO();
      showSPD();
      showDRF();
      showMet();
      
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
      lcd.print("  "+String(z->getText(z->get()) ));
      return;
   }

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

//*--------------------------------------------------------------------------------------------
//* doSave
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void doSave() {

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Saving....");
      
      delay(DELAY_SAVE);

      
      byte i=menuRoot.get();
      MenuClass* z=menuRoot.l.get(i)->mChild;
      byte j=z->mItem;
      byte k=z->mItemBackup;
   
      //*--- Detect changes that needs to be reflected thru commands to the ChipSet

      if ( (menuRoot.get() == PWRMENU) && (j!=k)) {doSetPower();}
      if ( (menuRoot.get() == SPDMENU) && (j!=k)) {doSetPD();}
      
      
      if ( (menuRoot.get() == STPMENU ||
            menuRoot.get() == RPTMENU ||
            menuRoot.get() == TONMENU ||
            menuRoot.get() == VFOMENU ||
            menuRoot.get() == BDWMENU ) && (j!=k)) {doSetGroup();}
      
      if ( (menuRoot.get() == HPFMENU ||
            menuRoot.get() == LPFMENU ||
            menuRoot.get() == PREMENU ) && (j!=k)) {doSetFilter();}
      
      vx.vfoAB=vfo.get();


      if (stp.get()==0) {
         vx.vfostep[vx.vfoAB]=VFO_STEP_5KHz;
      } else {
         vx.vfostep[vx.vfoAB]=VFO_STEP_10KHz;
      }   

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
//* Get the right Freq based on the transmission status and shift applied
//*--------------------------------------------------------------------------------------------------
long int getFreq(long int f, long int s, byte r) {

    switch(r) {

    case 0 : {
              return f;
              break;
             }
    case 1 : {
              return f+s;
              break;
             }
    case 2 : {
              return f-s;
              break;
             }
    
  }
  return f;
  
}
//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the reading of the KeyShield buttons
//*--------------------------------------------------------------------------------------------------
void readKeyShield() {


 lcd_key = read_LCD_buttons();  // read the buttons
 if (lcd_key != btnNONE) {
    if (getWord(USW,KDOWN)==false) {
       setWord(&USW,KDOWN,true);
       btnPrevio=lcd_key;
       return;
    } 
    return;
 }
     

 if ((lcd_key == btnNONE) && (getWord(USW,KDOWN)==true)) {
 
 setWord(&USW,KDOWN,false);
 switch (btnPrevio)               // depending on which button was pushed, we perform an action  
 {    
    case btnRIGHT:   
         setWord(&JSW,JRIGHT,true);
         {break;}   
    case btnLEFT:
         setWord(&JSW,JLEFT,true);
         {break;}  
    case btnUP:
         setWord(&JSW,JUP,true);
         {break;}   
    case btnDOWN:     
         setWord(&JSW,JDOWN,true);
         {break;}   
    case btnSELECT: {
         setWord(&MSW,LCLK,true);
         break;} 
    case btnEncodeOK:   
         {
         //*--- Interrupt produced by bMulti button (A0)?
         setWord(&USW,BMULTI,true);  //bMulti=true;
         menupassed=millis();        // Store for future long hit and double click
         break;     
         } 
     case btnNONE:   
         {
         break;
         }

}      
}

}

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages the command and VFO handling FSM
//*--------------------------------------------------------------------------------------------------
void CMD_FSM() {

   readKeyShield();

   //*-------------------------------------------------------------------------
   //* Operate Watchdog
   //*-------------------------------------------------------------------------
       if (getWord(TSW,FDOG)==true && wdg.get() !=0) {
          setWord(&TSW,FDOG,false);
          setWord(&MSW,DOG,true);
          digitalWrite(PTTPin,LOW);
          if ( getFreq(vx.vfo[vx.vfoAB],vx.vfoshift[vx.vfoAB],rpt.get() )!=vx.vfo[vx.vfoAB]) {
             showFreq();
          }
        
          showPTT();        
       }

   //*-------------------------------------------------------------------------------
   //* Handle PTT  (detect MIC PTT pressed and PTT signal not activated) just pressed
   //*-------------------------------------------------------------------------------
       if (digitalRead(A4)==LOW && getWord(MSW,PTT)==true){
        
           setWord(&MSW,PTT,false);
           digitalWrite(PTTPin,HIGH);    //*-- Prende TX
           
           if (getFreq(vx.vfo[vx.vfoAB],vx.vfoshift[vx.vfoAB],rpt.get()) !=vx.vfo[vx.vfoAB]) {
             showFreq();
           }

           digitalWrite(10,LCD_ON);
           TDIM=DIM_DELAY; 
           
           if (wdg.get()!=0) {TDOG=DOG_DELAY;}
           
           setWord(&MSW,DOG,false);
           showPTT();
           
       } else {
        
          if (digitalRead(A4)==HIGH && getWord(MSW,PTT)==false) {
              
              setWord(&MSW,PTT,true);
              digitalWrite(PTTPin,LOW);   //*-- Apaga TX
              showFreq();
              setWord(&MSW,DOG,false);
              showPTT();
          
          }   
       }
    //*-------------------------------------------------------------------------
    //* Handle squelch (avoid presenting while in menu)
    //*-------------------------------------------------------------------------
       if (digitalRead(A3)==LOW && getWord(MSW,SQL)==false && getWord(MSW,PTT)==true && getWord(USW,CMD)==false){
           setWord(&MSW,SQL,true);
           showSQL();
       } else {
          if (digitalRead(A3)==HIGH && getWord(MSW,PTT)==true && getWord(MSW,PTT)==true && getWord(USW,CMD)==false) {
              setWord(&MSW,SQL,false);
              showSQL();
          }   
       }
   //*-------------------------------------------------------------------------
   //* PTT == true means not activated (RX mode)
   //*-------------------------------------------------------------------------
   if (getWord(MSW,PTT) == true) {
    
     //*--- Lectura analógica int s=readV(SQLPIN,SQLMAX,SQLSCALE);
     sqlMeter.v=readMeter(&sqlMeter);

     if (sqlMeter.v != sqlMeter.vant && getWord(MSW,CMD)==true) {
        showMeter(&sqlMeter,sqlMeter.v);
        TS=DELAY_DISPLAY;
        setWord(&TSW,FTS,false);
     } else { 
       if (getWord(TSW,FTS)==true) {
           doSetGroup();           //* Call DMOSETUPGROUP to establish current SQL Level
           showPanel();
           memstatus=0;
           timepassed=millis();
           setWord(&TSW,FTS,false);
       }
     }
     
   }
   
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
      setWord(&USW,LCLK,false);
      setWord(&JSW,JLEFT,false);
      setWord(&JSW,JRIGHT,false);
      setWord(&JSW,JUP,false);
      setWord(&JSW,JDOWN,false);
      
   }

//*-------------------------------------------------------------------------------
//* PTT==true (RX) and CMD==false (VFO)
//*-------------------------------------------------------------------------------
   if (getWord(MSW,CMD)==false && getWord(MSW,PTT)==true) {      //S=0 VFO Mode   

//*-------------------------------------------------------------------------------
//*---- Process KEYLEFT
//*-------------------------------------------------------------------------------

      if (getWord(JSW,JLEFT)==true) {
        
         setWord(&JSW,JLEFT,false);
         setWord(&USW,BCCW,true);
         T4=LCD_DELAY;
         vx.updateVFO(vx.vfoAB,-VFO_STEP_1MHZ);
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;
      }

//*---- Process KEYRIGHT

      if (getWord(JSW,JRIGHT)==true) {
        
         setWord(&JSW,JRIGHT,false);
         setWord(&USW,BCW,true);
         T4=LCD_DELAY;
         vx.updateVFO(vx.vfoAB,VFO_STEP_1MHZ);
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;

      }

//*---- Process KEYUP

      if (getWord(JSW,JUP)==true) {
         
         setWord(&JSW,JUP,false);
         vfo.set((vfo.get()+1) % 2);
         vx.vfoAB=vfo.get();
         showPanel();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;

      }

//*---- Process KEYDOWN

      if (getWord(JSW,JDOWN)==true) {
         setWord(&JSW,JDOWN,false);
         rpt.mItem=(rpt.mItem+1) % 3;
         showPanel();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;
      }
      
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
         doSetGroup();
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

  
  long int f=getFreq(vx.vfo[vx.vfoAB],vx.vfoshift[vx.vfoAB],rpt.get());
  vx.computeVFO(f,&v);   
  char hi[40];
  sprintf(hi,"AT+DMOSETGROUP=%1d,%3d.%1d%1d%1d%1d,",bdw.get(),v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds);
  Serial.print(hi);
  byte SQLSig=0;
  
  f=vx.vfo[vx.vfoAB];
  vx.computeVFO(f,&v); 
  sprintf(hi,"%3d.%1d%1d%1d%1d,0,%1d,%1d\r\n",v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds,SQLSig,ctc.get());
  Serial.print(hi);
  
  delay(CMD_DELAY);
  
}
//*--------------------------------------------------------------------------------------------
//* doSetFilter
//* send commands related to setup filter information
//*--------------------------------------------------------------------------------------------
void doSetFilter() {
   
  char hi[30];
  sprintf(hi,"AT+SETFILTER=%1d,%1d,%1d\r\n",pre.get(),hpf.get(),lpf.get());
  Serial.println(hi);
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
  Serial.println(hi);
  delay(CMD_DELAY);
}
//*--------------------------------------------------------------------------------------------
//* doScan
//* send commands related to scan frequency information
//*--------------------------------------------------------------------------------------------
void doScan() {
  
}

//*--------------------------------------------------------------------------------------------
//* CTCSS Update
//* send commands related to scan frequency information
//*--------------------------------------------------------------------------------------------
void CTCSSUpdate() {
  byte i=menuRoot.get();
  MenuClass* z=menuRoot.getChild(i);
  
  z->mItem=0;
  ctcssvalue=ctcssvalue+1;
  
  if (ctcssvalue>(CTCSSCODEMAX-1)){ctcssvalue=0;};
  char* s=(char*)"                  ";
  
  switch(ctcssvalue) {
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
    /*
    case 21:                         {s=(char*)"136.5";break;};
    case 22:                         {s=(char*)"141.3";break;};
    case 23:                         {s=(char*)"146.2";break;};
    case 24:                         {s=(char*)"151.4";break;};
    case 25:                         {s=(char*)"156.7";break;};
    default:                         {s=(char*)"162.2";break;};
    /*
                              {(char*)"167.9"},
                              {(char*)"173.8"},
                              {(char*)"179.9"},
                              {(char*)"186.2"},
                              {(char*)"192.8"},
                              {(char*)"203.5"},
                              {(char*)"210.7"},
                              {(char*)"218.1"},
                              {(char*)"225.7"},
                              {(char*)"233.6"},
                              {(char*)"241.8"},                             
                              {(char*)"250.3"}};
    */
    
  }
  
  z->l.get(0)->mText=s;
  showPanel();
  
  return;
  
}

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
//*------------------------------------------------------------------------------------------------------
//* serialEvent is called when a serial port related interrupt happens
//*------------------------------------------------------------------------------------------------------
void serialEvent() {

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
}
//*****************************************************************************************************
//*                               Manages Meter
//*

//*****************************************************************************************************



//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Display analog indicator Read analog pin and returns a scaled value
//*--------------------------------------------------------------------------------------------------
void displayV(int V) {

lcd.clear();
lcd.setCursor(0,0);
lcd.print("<Squelch>");
lcd.setCursor(3,1);
lcd.print("[");
for (int i=0; i <= (V-1); i++){
   lcd.setCursor(4+i,1);
   lcd.write(byte(5));   
}
lcd.setCursor(12,1);
lcd.print("]");


return ; 
}

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

  if (TDIM>0 && getWord(MSW,PTT)==true) {
      TDIM--;
      if (TDIM==0 && spd.get() == 1) {
         digitalWrite(10,LCD_OFF);
      }
  }

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
  EEPROM.write(22,hpf.get());
  EEPROM.write(23,lpf.get());
  EEPROM.write(24,pre.get());
  EEPROM.write(25,ctc.get());
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
int read_LCD_buttons() {  



adc_key_in = analogRead(0);      

// read the value from the sensor  
// my buttons when read are centered at these valies: 0, 144, 329, 504, 741  
// we add approx 50 to those values and check to see if we are close  
//if(digitalRead(11)==0) return EncodeOK;

if (adc_key_in > 1000) return btnNONE; 
if (adc_key_in < 50)   return btnLEFT;   
if (adc_key_in < 150)  return btnUP;   
if (adc_key_in < 250)  return btnRIGHT;   
if (adc_key_in < 450)  return btnSELECT;   
if (adc_key_in < 700)  return btnDOWN;     
if (adc_key_in < 850)  return btnEncodeOK;

return btnNONE;  // when all others fail, return this... 

}   
//*-------------------------------------------------------------------------------------------------
//* Get a sample of the associated variable
//*-------------------------------------------------------------------------------------------------
float readMeter(Meter* m) {
  
  word vread=(word)analogRead(m->pin);
  float vpu=m->vref/m->vmax;
  m->v=vpu*(float)(vread);
  return m->v;
  
}
//*-------------------------------------------------------------------------------------------------
//* Draw the value as an analog mark of SCALE blocks at the LCD real estate
//*-------------------------------------------------------------------------------------------------
void showMeter(Meter* m,float v){

  return;
  byte l[3];
  byte x=(int)m->v*5;
  lcd.setCursor(13,0);
  lcd.print(m->name);

  if (x>=10){l[0]=5;l[1]=5;l[2]=m->v-10;}
  if (x>=5 && x<10){l[0]=5;l[1]=m->v-5;l[2]=0;}
  if (x>=0 && x<5) {l[0]=m->v-5;l[1]=0;l[2]=0;}
  
  lcd.setCursor(13,1);
  
  lcd.write((byte)l[0]);
  lcd.write((byte)l[1]);
  lcd.write((byte)l[2]);

  m->vant=m->v;
  
}



