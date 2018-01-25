//*--------------------------------------------------------------------------------------------------
//* DDSPlus Firmware Version 1.0
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseño picoFM en su version inicial
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//* Plataforma: Arduino NANO/UNO/Mega
//* LCD: 16x2 HD44780 o equivalente
//* Standard Arduino PIN Assignment
//*   A0 - PushButton
//*   A1 - Encoder CW
//*   A2 - Encoder CCW
//*   A3 - SQ  (sense) Squelch Open
//*   A4 - PTT (sense)
//*   A5 - SQL (sense) Squelch Reference
//*   
//*   LCD Handling
//*   D2 - RS
//*   D3 - E
//*   D4 - DB4
//*   D5 - DB5
//*   D6 - DB6
//*   D7 - DB7
//*
//*   Transceiver control
//*   D8 - RxD
//*   D9 - TxD
//*   D10- PD
//*   D11- (free)
//*   D12- H/L
//*   D13- PTT (send)
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
#define PROG_BUILD  "024"
#define COPYRIGHT "(c) LU7DID 2018"

#endif

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


//*--- Read Squelch control

#define SQLPIN           A5
#define SQLMAX         1023
#define SQLSCALE          8
int     SQLSig=0;

#define ZERO             0
#define SERIAL_MAX      16
#define VOLUME           5

#if PICOFM

//*--- Control lines for the DRA818V

#define PTTPin          13
#define HLPin           12
#define PDPin           11

#define VFO_SHIFT       600000
#define VFO_START    144000000
#define VFO_END      147990000
#define VFO_STEP_10KHz   10000
#define VFO_STEP_5KHz     5000
#endif


//*----------------------------------------[DEFINITION]----------------------------------------------
#define DELAY_DISPLAY 4000     //delay to show the initial display in mSecs

//*--- Control delays

#define DELAY_SAVE    1000     //delay to consider the frequency needs to be saved in mSecs
#define LCD_DELAY     1000
#define CMD_DELAY      100
#define PTY_DELAY      200
#define DIM_DELAY    10000
#define DOG_DELAY    60000
#define BLINK_DELAY   1000

#if PICOFM
//*--------------------------------------------------------------------------------------------------
typedef struct {
  char* s;
} ctcsscode;

#define CTCSSCODEMAX 39


#define LONGLAP         200    // Elapsed time to consider a Long Push at the MULTI button
#define NMAX             12    // Max number of menu items 

typedef struct
   { 
    byte       mItem;
    char*      mText[NMAX];
   } menu;

#define QUEUEMAX  16        // Queue of incoming characters 

#endif

#if PICOFM
//*--- Max number of elements in each menu item

#define PWRMAX  2
#define STPMAX  2
#define RPTMAX  3
#define SPDMAX  2
#define BDWMAX  2
#define HPFMAX  2
#define LPFMAX  2
#define PREMAX  2
#define TONEMAX 2
#define CTCMAX 39
#define VFOMAX  2
#define PTYMAX  2

#endif

#if PICOFM


#define FORCEFREQ     0
#define LCD_ON        1
#define LCD_OFF       0


#endif

//*----------------------------------------[INCLUDE]-------------------------------------------------
#include <LiquidCrystal.h>
#include <stdio.h>
#include <EEPROM.h>
#include "MemSizeLib.h"
#include "VFOSystem.h"

//*-------------------------------------------------------------------------------------------------
//* Define class to manage VFO
//*-------------------------------------------------------------------------------------------------
void  Encoder_san();
void showFreq();   //Prototype fur display used on callback
VFOSystem vx(showFreq,NULL);


#if PICOFM
//*==============================================================================================================================
byte HALF[8] = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
};

 
byte FULL[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
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

//*---- Definitions for various LCD display shields

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



#if PICOFM

//*---- CTCSS Code Tables

const ctcsscode ctcss[CTCSSCODEMAX]={
                              {(char*)"Off"},
                              {(char*)"67.0"},
                              {(char*)"71.9"},
                              {(char*)"74.4"},
                              {(char*)"77.0"},
                              {(char*)"79.7"},
                              {(char*)"82.5"},
                              {(char*)"85.4"},
                              {(char*)"88.5"},
                              {(char*)"91.5"},
                              {(char*)"94.8"},
                              {(char*)"97.4"},
                              {(char*)"100.0"},
                              {(char*)"103.5"},
                              {(char*)"107.2"},
                              {(char*)"110.9"},
                              {(char*)"114.8"},
                              {(char*)"118.8"},
                              {(char*)"123.0"},
                              {(char*)"127.3"},
                              {(char*)"131.8"},
                              {(char*)"136.5"},
                              {(char*)"141.3"},
                              {(char*)"146.2"},
                              {(char*)"151.4"},
                              {(char*)"156.7"},
                              {(char*)"162.2"},
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

#endif

//*--- Pseudo Real Time Clock  

byte mm=0;
byte ss=0;
unsigned long Tclk=1000;
byte btnPrevio=btnNONE;

//*--- Timer related definitions

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


//*--------------------------------------------------------------------------------
//* Menu related structures
//*--------------------------------------------------------------------------------

#if PICOFM

//*---- Menu storage definition
//*----- Menu memory structures

byte mItembackup=0;      //Item backup during actualization

menu mainMenu;
menu pwrMenu;
menu ctcssMenu;
menu stpMenu;
menu rptMenu;
menu spdMenu;
menu bdwMenu;
menu hpfMenu;
menu lpfMenu;
menu preMenu;
menu wdogMenu;
menu vfoMenu;
menu ptyMenu;

#endif

//*-----------------------------------------------------------------------------------
//*--- Define System Status Words
//*   

byte MSW = 0;
byte TSW = 0;
byte USW = 0;
byte JSW = 0;

char serialQueue[QUEUEMAX]; // Actual Queue space [a0,a1,...,an]
byte pQueue = 0;            // Pointer to next position to use
byte inState= 0;
byte inCmd=0;


//*---- Control panel state variables

byte seqA = 0;
byte seqB = 0;
byte cnt1 = 0;
byte cnt2 = 0;

byte ones, tens, hundreds, thousands, tenthousands, hundredthousands, millions ; //Placeholders

int_fast32_t timepassed = millis(); // int to hold the arduino miilis since startup
int_fast32_t menupassed = millis();

int      memstatus   = 1;           // value to notify if memory is current or old. 0=old, 1=current.


//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//*  Setup
//*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup() {

  //*--- PinOut Definition


  lcd.begin(16, 2);   // start the library  
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

//*--- Create special characters for LCD


  lcd.createChar(2,FULL);
  lcd.createChar(4,HALF);
  lcd.createChar(5,TX);
  lcd.createChar(6,RX);
  lcd.createChar(7,WATCHDOG);
  
  lcd.setCursor(0, 0);        // Place cursor at [0,0]
  lcd.print(String(PROGRAMID)+" v"+String(PROG_VERSION)+"-"+String(PROG_BUILD));

  lcd.setCursor(0, 1);        // Place cursor at [0,1]
  lcd.print(String(COPYRIGHT));

  delay(DELAY_DISPLAY);
  lcd.clear();


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


  //*---
  

  SQLSig=readV(SQLPIN,SQLMAX,SQLSCALE);
  
 
    
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
  
  //*---- Setup Master system menus

  mainMenu.mItem = 0;
  
  mainMenu.mText[0] = (char*)"Power";
  mainMenu.mText[1] = (char*)"WDog";
  mainMenu.mText[2] = (char*)"CTCSS ";
  mainMenu.mText[3] = (char*)"Pre ";
  mainMenu.mText[4] = (char*)"LPF";
  mainMenu.mText[5] = (char*)"HPF";
  mainMenu.mText[6] = (char*)"BDW";
  mainMenu.mText[7] = (char*)"SPD";
  mainMenu.mText[8] = (char*)"RPT";
  mainMenu.mText[9] = (char*)"Step";
  mainMenu.mText[10]= (char*)"VFO";
  mainMenu.mText[11]= (char*)"Pty";  

//*------ Secondary menues

 //*- Power

 pwrMenu.mItem = 0;
 pwrMenu.mText[0] = (char*)"Low";
 pwrMenu.mText[1] = (char*)"High";

 //*- WatchDog
 wdogMenu.mItem = 0;
 wdogMenu.mText[0] = (char*)"Off";
 wdogMenu.mText[1] = (char*)"On";

 //*- Pre

 preMenu.mItem = 0;
 preMenu.mText[0] = (char*)"Off";
 preMenu.mText[1] = (char*)"On";

 //*- LPF

 lpfMenu.mItem = 0;
 lpfMenu.mText[0] = (char*)"Off";
 lpfMenu.mText[1] = (char*)"On";

  //*- HPF

 hpfMenu.mItem = 0;
 hpfMenu.mText[0] = (char*)"Off";
 hpfMenu.mText[1] = (char*)"On";

  //*- BDW

 bdwMenu.mItem = 1;
 bdwMenu.mText[0] = (char*)"12.5 KHz";
 bdwMenu.mText[1] = (char*)"25.0 KHz";


  //*- SPD

 spdMenu.mItem = 0;
 spdMenu.mText[0] = (char*)"Off";
 spdMenu.mText[1] = (char*)"On";

  //*- RPT

 rptMenu.mItem = 0;
 rptMenu.mText[0] = (char*)" ";
 rptMenu.mText[1] = (char*)"+";
 rptMenu.mText[2] = (char*)"-";

//*- Step 
 
 stpMenu.mItem = 1;
 stpMenu.mText[0] = (char*)" 5  KHz";
 stpMenu.mText[1] = (char*)"10  KHz";

//*- Step 
 
 vfoMenu.mItem = 0;
 vfoMenu.mText[0] = (char*)"VFO A";
 vfoMenu.mText[1] = (char*)"VFO B";

//*- Priority Channel 
 
 ptyMenu.mItem = 0;
 ptyMenu.mText[0] = (char*)"Off";
 ptyMenu.mText[1] = (char*)"On";

//*--- CTCSS
 ctcssMenu.mItem = 0;
 ctcssMenu.mText[0] = (char*)" Hz";

 
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
  
  //*--- Load the stored frequency

  if (FORCEFREQ == 0) {
     
     SQLSig = String(EEPROM.read(0)).toInt();
     String freq = String(EEPROM.read(1)) + String(EEPROM.read(2)) + String(EEPROM.read(3)) + String(EEPROM.read(4)) + String(EEPROM.read(5)) + String(EEPROM.read(6)) + String(EEPROM.read(7));
            vx.vfo[VFOA] = freq.toInt();
            
            freq = String(EEPROM.read(8)) + String(EEPROM.read(9)) + String(EEPROM.read(10)) + String(EEPROM.read(11)) + String(EEPROM.read(12)) + String(EEPROM.read(13)) + String(EEPROM.read(14));
            vx.vfo[VFOB] = freq.toInt();
            vx.vfoAB = EEPROM.read(15);        
            
            pwrMenu.mItem = EEPROM.read(16);
            wdogMenu.mItem= EEPROM.read(17);
            stpMenu.mItem = EEPROM.read(18);
            rptMenu.mItem = EEPROM.read(19);
            spdMenu.mItem = EEPROM.read(20);
            bdwMenu.mItem = EEPROM.read(21);
            hpfMenu.mItem = EEPROM.read(22);
            lpfMenu.mItem = EEPROM.read(23);
            preMenu.mItem = EEPROM.read(24);
            ctcssMenu.mItem=EEPROM.read(25);
            vfoMenu.mItem = EEPROM.read(26);


            MSW = EEPROM.read(27);
            USW = EEPROM.read(28);
            TSW = EEPROM.read(29);

  } 
//*--------- Attempt to establish contact with the DRA018F module
  
  doHandShake();
  doSetVolume();
  doSetFilter();
  doSetGroup();

  showPanel();
//*--------- Update SQL
     
  int s=readV(SQLPIN,SQLMAX,SQLSCALE);
  SQLSig=s;
  
//*=========================================================================================================
#if DEBUG
//*--- Print Serial Banner (TEST Mode Mostly)
  char hi[80];
  sprintf(hi,"%s %s Compiled %s %s",PROGRAMID,PROG_VERSION,__TIME__, __DATE__);
  Serial.println(hi);
  sprintf(hi,"(c) %s",COPYRIGHT);
  Serial.print("RAM Free=");
  Serial.println(freeMemory());
#endif
//*=========================================================================================================

}

//*****************************************************************************************************
//*                               Menú Finite Status Machine (FSM)
//*
//*
//*
//*
//*
//*****************************************************************************************************
//*--------------------------------------------------------------------------------------------
//* menuText
//* solves text based on Menu FSM state
//*--------------------------------------------------------------------------------------------
String menuText(byte mItem) {


   switch(mItem) {
   
   
   case 0: {
        return String(pwrMenu.mText[pwrMenu.mItem]);
   }
   case 1: {
        return String(wdogMenu.mText[wdogMenu.mItem]);
   }

   case 2: {
        if (ctcssMenu.mItem==0) {return String("Off");}
        return String(ctcss[ctcssMenu.mItem].s)+" "+ctcssMenu.mText[0];
        //return (char*)ctcss[ctcssMenu.mItem].s;
   }
   case 3: {
        return String(preMenu.mText[preMenu.mItem]);
   }
   case 4: {
        return String(lpfMenu.mText[lpfMenu.mItem]);
   }
   case 5: {
        return String(hpfMenu.mText[hpfMenu.mItem]);
   }
   case 6: {
        return String(bdwMenu.mText[bdwMenu.mItem]);
   }
   case 7: {
        return String(spdMenu.mText[spdMenu.mItem]);
   }
   case 8: {
        return String(rptMenu.mText[rptMenu.mItem]);
   }
   case 9: {
        return String(stpMenu.mText[stpMenu.mItem]);
   }
   case 10: {
        return String(vfoMenu.mText[vfoMenu.mItem]);
   }
   case 11: {
        return String(ptyMenu.mText[ptyMenu.mItem]);
   }

   }
   return String("??");
 
}
//*-------------------------------------------------------------------------------------------
//*
//*-------------------------------------------------------------------------------------------
void splitFreq(long int fx) {
  

  millions =       int(fx / 1000000);
  hundredthousands = ((fx / 100000) % 10);
  tenthousands =     ((fx / 10000) % 10);
  thousands =        ((fx / 1000) % 10);
  hundreds =         ((fx / 100) % 10);
  tens =             ((fx / 10) % 10);
  ones =             ((fx / 1) % 10);
  
}
//*--------------------------------------------------------------------------------------------
//* showFreq
//* show frequency at the display
//*--------------------------------------------------------------------------------------------
void showFreq() {

  FSTR v;
   
  long int f=vx.vfo[vx.vfoAB];
  
  if (getWord(MSW,PTT)==false) {
     f=getFreq(f,vx.vfoshift[vx.vfoAB],rptMenu.mItem);
  }

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
  lcd.print(rptMenu.mText[rptMenu.mItem]);

};
//*--------------------------------------------------------------------------------------------
//* showCTC
//* show CTCSS operation mode at the display
//*--------------------------------------------------------------------------------------------
void showCTC() {

  lcd.setCursor(1,0);
  if (ctcssMenu.mItem==0){lcd.print(" ");} else {lcd.print("T");}

};
//*--------------------------------------------------------------------------------------------
//* showPwr
//* show power level at the display
//*--------------------------------------------------------------------------------------------
void showPwr() {
  
  lcd.setCursor(13,1);
  
  if (pwrMenu.mItem==0){
     lcd.write(2);
     lcd.setCursor(14,1);
     lcd.write(4); 
  } else {
     lcd.write(2);
     lcd.setCursor(14,1);
     lcd.write(2);
     lcd.setCursor(15,1);
     lcd.write(2);
  }   
}
//*--------------------------------------------------------------------------------------------
//* showSPD
//* show HILO operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSPD() {

  lcd.setCursor(3,0);
  if (spdMenu.mItem==0) {lcd.print(" ");} else {lcd.print("Z");}
};

//*--------------------------------------------------------------------------------------------
//* showSQL
//* show SQL operation mode at the display
//*--------------------------------------------------------------------------------------------
void showSQL() {

  lcd.setCursor(13,0);
  if (digitalRead(A3)==LOW) {lcd.write(2);} else {lcd.write(6);}
  
};
//*--------------------------------------------------------------------------------------------
//* showPTT
//* show PTT operation mode at the display
//*--------------------------------------------------------------------------------------------
void showPTT() {

  lcd.setCursor(14,0);
  if (getWord(MSW,DOG)==true && wdogMenu.mItem!=0){lcd.write(7); lcd.setCursor(14,0);lcd.blink();return;}
  if (getWord(MSW,PTT)==false) {lcd.write(5); lcd.setCursor(14,0);lcd.noBlink();} else {lcd.print(" ");lcd.setCursor(14,0);lcd.noBlink();}
};

//*--------------------------------------------------------------------------------------------
//* showPre
//* show pre-emphasis filter at the display
//*--------------------------------------------------------------------------------------------
void showPre() {
  
  lcd.setCursor(4,0);
  if (preMenu.mItem==0){lcd.print(" ");} else {lcd.print("P");}

};
//*--------------------------------------------------------------------------------------------
//* showHPF
//* show HPF filter at the display
//*--------------------------------------------------------------------------------------------
void showHPF() {
  
  lcd.setCursor(5,0);
  if (hpfMenu.mItem==0){lcd.print(" ");} else {lcd.print("H");}

};
//*--------------------------------------------------------------------------------------------
//* showDRF
//* show DRF filter at the display
//*--------------------------------------------------------------------------------------------
void showDRF() {
  
  lcd.setCursor(7,0);
  if (getWord(MSW,DRF)==false){lcd.print(char(174));} else {lcd.print(char(42));}

};
//*--------------------------------------------------------------------------------------------
//* showDog
//* show if watchdog is enabled or not
//*--------------------------------------------------------------------------------------------
void showDog() {
  
  lcd.setCursor(8,0);
  if (wdogMenu.mItem==0){lcd.print(" ");} else {lcd.print("W");}

};

//*--------------------------------------------------------------------------------------------
//* showLPF
//* show LPF filter at the display
//*--------------------------------------------------------------------------------------------
void showLPF() {
  
  lcd.setCursor(6,0);
  if (lpfMenu.mItem==0){lcd.print(" ");} else {lcd.print("L");}

};

//*--------------------------------------------------------------------------------------------
//* showVFO
//* show VFO filter at the display
//*--------------------------------------------------------------------------------------------
void showVFO() {
  
  lcd.setCursor(10,0);
  if (vx.vfoAB==VFOA){lcd.print("A");} else {lcd.print("B");}

};





//*--------------------------------------------------------------------------------------------
//* showPanel
//* show frequency at the display
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
      
      return;
   }

//*--- if here then CLI==true

   if (getWord(MSW,GUI)==false) {
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(mainMenu.mItem)+"> "+String(mainMenu.mText[mainMenu.mItem]));
      lcd.setCursor(1,1);
      lcd.print("  "+String(menuText(mainMenu.mItem)));

      return;
      
   } else {
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("<"+String(mainMenu.mItem)+"> "+String(mainMenu.mText[mainMenu.mItem]));
      lcd.setCursor(0,1);
      lcd.print(">");
      lcd.print(" "+String(menuText(mainMenu.mItem)));
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
      
      mainMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),mainMenu.mItem,NMAX);
      showPanel();
      return;
   }

//*---- Here CMD==true and GUI==true so it's a second level menu
   switch (mainMenu.mItem) {
     
      case 0 : {
         pwrMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),pwrMenu.mItem,PWRMAX);
         showPanel();
         return;
      }
      case 1 : {
         wdogMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),wdogMenu.mItem,TONEMAX);
         showPanel();
         return;
      }
      
      case 2 : {
         ctcssMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),ctcssMenu.mItem,CTCMAX);
         showPanel();
         return;
      }
      case 3 : {
         preMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),preMenu.mItem,PREMAX);
         showPanel();
         return;
      }
      case 4 : {
         lpfMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),lpfMenu.mItem,LPFMAX);
         showPanel();
         return;
      }
      case 5 : {
         hpfMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),hpfMenu.mItem,HPFMAX);
         showPanel();
         return;
      }
      case 6 : {
         bdwMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),bdwMenu.mItem,BDWMAX);
         showPanel();
         return;
      }
      case 7 : {
         spdMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),spdMenu.mItem,SPDMAX);
         showPanel();
         return;
      }
      case 8 : {
         rptMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),rptMenu.mItem,RPTMAX);
         showPanel();
         return;
      }
      
      case 9 : {
         stpMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),stpMenu.mItem,STPMAX);
         showPanel();
         return;
      }
     case 10 : {
         vfoMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),vfoMenu.mItem,VFOMAX);
         showPanel();
         return;
      }
     case 11 : {
         ptyMenu.mItem = incMenu(getWord(USW,BCW),getWord(USW,BCCW),ptyMenu.mItem,PTYMAX);
         showPanel();
         return;
      }

   }
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

      //*--- Detect changes that needs to be reflected thru commands to the ChipSet

      if ((mainMenu.mItem == 0) && (pwrMenu.mItem   != mItembackup)) {doSetPower();}
      if ((mainMenu.mItem == 7) && (spdMenu.mItem   != mItembackup)) {doSetPD();}

      if ((mainMenu.mItem == 1) && (wdogMenu.mItem  != mItembackup)) {doSetGroup();}
      if ((mainMenu.mItem == 2) && (ctcssMenu.mItem != mItembackup)) {doSetGroup();}
      if ((mainMenu.mItem == 6) && (bdwMenu.mItem   != mItembackup)) {doSetGroup();}
      if ((mainMenu.mItem == 8) && (rptMenu.mItem   != mItembackup)) {doSetGroup();}
      if ((mainMenu.mItem ==10) && (vfoMenu.mItem   != mItembackup)) {doSetGroup();}


      if ((mainMenu.mItem == 3) && (preMenu.mItem   != mItembackup)) {doSetFilter();}
      if ((mainMenu.mItem == 4) && (lpfMenu.mItem   != mItembackup)) {doSetFilter();}
      if ((mainMenu.mItem == 5) && (hpfMenu.mItem   != mItembackup)) {doSetFilter();}

      vx.vfoAB=vfoMenu.mItem;

      if (stpMenu.mItem==0) {
         vx.vfostep[vx.vfoAB]=VFO_STEP_5KHz;
      } else {
         vx.vfostep[vx.vfoAB]=VFO_STEP_10KHz;
      }   

      setWord(&MSW,CMD,false);
      setWord(&MSW,GUI,false);
     
      showPanel;

}

//*---------------------------------------------------------------------------------------
//*-- Increase manu item
//*---------------------------------------------------------------------------------------
byte incMenu(boolean cw,boolean ccw,byte mItem,byte mItemMax) {
      
      if (cw == true) {
         if (mItem==mItemMax-1) { return 0; }
         return mItem+1;
      }
      if (ccw == true) {
         if (mItem==0) {return mItemMax-1;}
         return mItem-1;
      }
      return mItem;
}
//*****************************************************************************************************
//*                            Command Finite Status Machine (FSM)
//*
//*
//*
//*
//*
//*****************************************************************************************************
//*----------------------------------------------------------------------------------------------------
//* backupFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void backupFSM() {
//*---- Here CMD==true and GUI==true so it's a second level menu
   switch (mainMenu.mItem) {

      case 0 : {
         mItembackup=pwrMenu.mItem;
         return;
      }
      case 1 : {
         mItembackup=wdogMenu.mItem;
         return;
      }
      
      case 2 : {
         mItembackup=ctcssMenu.mItem;
         return;
      }
      case 3 : {
         mItembackup=preMenu.mItem;
         return;
      }
      case 4 : {
         mItembackup=lpfMenu.mItem;
         return;
      }
      case 5 : {
         mItembackup=hpfMenu.mItem;
         return;
      }
      case 6 : {
         mItembackup=bdwMenu.mItem;
         return;
      }
      case 7 : {
         mItembackup=spdMenu.mItem;
         return;
      }
      case 8 : {
         mItembackup=rptMenu.mItem;
         return;
      }
      case 9 : {
         mItembackup=stpMenu.mItem;
         return;
      }
      case 10 : {
         mItembackup=vfoMenu.mItem;
         return;
      }
      case 11 : {
         mItembackup=ptyMenu.mItem;
         return;
      }

   }
  
}
//*----------------------------------------------------------------------------------------------------
//* restoreFSM
//* come here with CLI==true so it's either one of the two menu levels
//*----------------------------------------------------------------------------------------------------
void restoreFSM() {
//*---- Here CLI==true and MENU==true so it's a second level menu
   switch (mainMenu.mItem) {

      case 0 : {
         pwrMenu.mItem=mItembackup;
         return;
      }
      case 1 : {
         wdogMenu.mItem=mItembackup;
         return;
      }
      
      case 2 : {
         ctcssMenu.mItem=mItembackup;
         return;
      }
      case 3 : {
         preMenu.mItem=mItembackup;
         return;
      }
      case 4 : {
         lpfMenu.mItem=mItembackup;
         return;
      }
      case 5 : {
         hpfMenu.mItem=mItembackup;
         return;
      }
      case 6 : {
         bdwMenu.mItem=mItembackup;
         return;
      }
      case 7 : {
         spdMenu.mItem=mItembackup;
         return;
      }
      case 8 : {
         rptMenu.mItem=mItembackup;
         return;
      }
      case 9 : {
         stpMenu.mItem=mItembackup;
         return;
      }
      case 10 : {
         vfoMenu.mItem=mItembackup;
         return;
      }
      case 11 : {
         ptyMenu.mItem=mItembackup;
         return;
      }

   }
  
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
       if (getWord(TSW,FDOG)==true && wdogMenu.mItem !=0) {
          setWord(&TSW,FDOG,false);
          setWord(&MSW,DOG,true);
          digitalWrite(PTTPin,LOW);
          if (getFreq(vx.vfo[vx.vfoAB],vx.vfoshift[vx.vfoAB],rptMenu.mItem)!=vx.vfo[vx.vfoAB]) {
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
           
           if (getFreq(vx.vfo[vx.vfoAB],vx.vfoshift[vx.vfoAB],rptMenu.mItem)!=vx.vfo[vx.vfoAB]) {
             showFreq();
           }

           digitalWrite(10,LCD_ON);
           TDIM=DIM_DELAY; 
           
           if (wdogMenu.mItem!=0) {TDOG=DOG_DELAY;}
           
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
    //* Handle squelch
    //*-------------------------------------------------------------------------
       if (digitalRead(A3)==LOW && getWord(MSW,SQL)==false && getWord(MSW,PTT)==true){
           setWord(&MSW,SQL,true);
           showSQL();
       } else {
          if (digitalRead(A3)==HIGH && getWord(MSW,PTT)==true && getWord(MSW,PTT)==true) {
              setWord(&MSW,SQL,false);
              showSQL();
          }   
       }
   //*-------------------------------------------------------------------------
   //* PTT == true means not activated (RX mode)
   //*-------------------------------------------------------------------------
   if (getWord(MSW,PTT) == true) {
    
     int s=readV(SQLPIN,SQLMAX,SQLSCALE);

     if (s != SQLSig) {
        displayV(s);
        SQLSig=s;
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
         vx.updateVFO(vx.vfoAB,-1000000);
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;
      }

//*---- Process KEYRIGHT

      if (getWord(JSW,JRIGHT)==true) {
        
         setWord(&JSW,JRIGHT,false);
         setWord(&USW,BCW,true);
         T4=LCD_DELAY;
         vx.updateVFO(vx.vfoAB,+1000000);
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);

         return;

      }

//*---- Process KEYUP

      if (getWord(JSW,JUP)==true) {
         
         setWord(&JSW,JUP,false);
         vfoMenu.mItem=(vfoMenu.mItem+1) % 2;
         vx.vfoAB=vfoMenu.mItem;
         showPanel();
         setWord(&USW,BCW,false);
         setWord(&USW,BCCW,false);
         return;

      }

//*---- Process KEYDOWN

      if (getWord(JSW,JDOWN)==true) {
         setWord(&JSW,JDOWN,false);
         
         rptMenu.mItem=(rptMenu.mItem+1) % 3;
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
  
  long int f=getFreq(vx.vfo[vx.vfoAB],vx.vfoshift[vx.vfoAB],rptMenu.mItem);
  vx.computeVFO(f,&v);   
  char hi[40];
  sprintf(hi,"AT+DMOSETGROUP=%1d,%3d.%1d%1d%1d%1d,",bdwMenu.mItem,v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds);
  Serial.print(hi);
  
  f=vx.vfo[vx.vfoAB];
  vx.computeVFO(f,&v); 
  sprintf(hi,"%3d.%1d%1d%1d%1d,0,%1d,%1d\r\n",v.millions,v.hundredthousands,v.tenthousands,v.thousands,v.hundreds,SQLSig,ctcssMenu.mItem);
  Serial.print(hi);
  
  delay(CMD_DELAY);
  
}
//*--------------------------------------------------------------------------------------------
//* doSetFilter
//* send commands related to setup filter information
//*--------------------------------------------------------------------------------------------
void doSetFilter() {
   
  char hi[30];
  sprintf(hi,"AT+SETFILTER=%1d,%1d,%1d\r\n",preMenu.mItem,hpfMenu.mItem,lpfMenu.mItem);
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

  if (pwrMenu.mItem==0) {
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
  if (spdMenu.mItem==0) {
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
                   //Serial.println("Frequency");
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
//*
//*
//*
//*
//*****************************************************************************************************

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Read analog pin and returns a scaled value
//*--------------------------------------------------------------------------------------------------
int readV(byte VPIN,int VMAX,int VSCALE) {

return (int)( (analogRead(A5)/113) - 1);

}

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
   lcd.write(2);   
}
lcd.setCursor(12,1);
lcd.print("]");


return ; 
}

//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Manages frequency display at the LCD
//*--------------------------------------------------------------------------------------------------
void LCD_FSM() {

   
/*
  //*--- Change frequency detected?

  if (vx.isVFOChanged(vx.vfoAB)==true) {
     
     
     if (getWord(USW,BBOOT)==true) {
        showPanel();
        setWord(&USW,BBOOT,false);    //bBoot=false;
     } else {
        if (TS>0) {
            TS=0;
            setWord(&TSW,FTS,false);
            showPanel();
            memstatus=0;
            timepassed=millis();
        } else {
          if ((rptMenu.mItem==1 && (vx.vfo[vx.vfoAB]+vx.vfoshift[vx.vfoAB])>vx.vfomax[vx.vfoAB]) ||
              (rptMenu.mItem==2 && (vx.vfo[vx.vfoAB]-vx.vfoshift[vx.vfoAB])<vx.vfomin[vx.vfoAB])) {
              vx.resetVFOFreq(vx.vfoAB);    //vfo[vfoAB]=rxa;
              setWord(&TSW,FTS,false);
              showPanel();
              memstatus=1;
              timepassed=millis();
              return;
             }
          //showFreq();    
          //doSetGroup();
          //vx.equalVFO(vx.vfoAB);
        }
        
     }
    showFreq();
    doSetGroup();
    vx.resetVFO(vx.vfoAB);
  }
*/



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
      if (TDIM==0 && spdMenu.mItem == 1) {
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
  //Serial.println("Writing EEPROM...");

  if (memstatus==1) {return; }
  
  EEPROM.write(0,byte(SQLSig && 0xff));              //* Store last squelch position known
  vx.getStr(VFOA);
  
  //splitFreq(vfo[VFOA]);
  
  EEPROM.write(1, vx.vfostr[VFOA].millions);
  EEPROM.write(2, vx.vfostr[VFOA].hundredthousands);
  EEPROM.write(3, vx.vfostr[VFOA].tenthousands);
  EEPROM.write(4, vx.vfostr[VFOA].thousands);
  EEPROM.write(5, vx.vfostr[VFOA].hundreds);
  EEPROM.write(6, vx.vfostr[VFOA].tens);
  EEPROM.write(7, vx.vfostr[VFOA].ones);

  vx.getStr(VFOB);
  //splitFreq(vfo[VFOB]);
  
  EEPROM.write(8,  vx.vfostr[VFOB].millions);
  EEPROM.write(9,  vx.vfostr[VFOB].hundredthousands);
  EEPROM.write(10, vx.vfostr[VFOB].tenthousands);
  EEPROM.write(11, vx.vfostr[VFOB].thousands);
  EEPROM.write(12, vx.vfostr[VFOB].hundreds);
  EEPROM.write(13, vx.vfostr[VFOB].tens);
  EEPROM.write(14, vx.vfostr[VFOB].ones);

  EEPROM.write(15, vx.vfoAB);
  
  EEPROM.write(16,pwrMenu.mItem);
  EEPROM.write(17,wdogMenu.mItem);
  EEPROM.write(18,stpMenu.mItem);
  EEPROM.write(19,rptMenu.mItem);
  EEPROM.write(20,spdMenu.mItem);
  EEPROM.write(21,bdwMenu.mItem);
  EEPROM.write(22,hpfMenu.mItem);
  EEPROM.write(23,lpfMenu.mItem);
  EEPROM.write(24,preMenu.mItem);
  EEPROM.write(25,ctcssMenu.mItem);
  EEPROM.write(26,vfoMenu.mItem);
  
  EEPROM.write(27,MSW);
  EEPROM.write(28,USW);
  EEPROM.write(29,TSW);

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
  if (charbytein == 158) { charbytein = 142; }  // ž -> Ž
  if (charbytein == 154) { charbytein = 138; }  // š -> Š
  
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

