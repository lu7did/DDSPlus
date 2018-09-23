//*--------------------------------------------------------------------------------------------------
//* Specific Headers for PicoFM implementation
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#if PICOFM

#define PROGRAMID "picoFM"
#define PROG_VERSION   "1.0"


#endif


#if PICOFM
//*------------------------------------------------------------------------------------------------------------
//*--- Read Squelch control
//*------------------------------------------------------------------------------------------------------------

#define ZERO             0
#define SERIAL_MAX      16
#define VOLUME           5

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

void definepicoFMmenu(){
  //*============================================================================================
//* Define master menu and lower level tree for picoFM
//*============================================================================================
//*---- Setup Master system menus

  menuRoot.add((char*)"Power",&pwr);
  menuRoot.add((char*)"Split",&rpt);
  menuRoot.add((char*)"SPD",&spd);
  menuRoot.add((char*)"BDW",&bdw);
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

}

void picoFMpinsetup() {
  

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

//*--- Create special characters for LCD

  lcd.createChar(0,RX);
  lcd.createChar(1,BB1);
  lcd.createChar(2,BB2);
  lcd.createChar(3,BB3);
  lcd.createChar(4,BB4);
  lcd.createChar(5,BB5);
  lcd.createChar(6,TX);
  lcd.createChar(7,WATCHDOG);
}  
#endif
