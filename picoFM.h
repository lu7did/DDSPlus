//*--------------------------------------------------------------------------------------------------
//* Specific Headers for PicoFM implementation
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#if PICOFM

#define PROGRAMID "picoFM"
#define PROG_VERSION   "1.0"
#define PROG_BUILD  "030"
#define COPYRIGHT "(c) LU7DID 2018"

#endif


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

#endif

