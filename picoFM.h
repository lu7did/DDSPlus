//*--------------------------------------------------------------------------------------------------
//* Specific Headers for PicoFM implementation
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#if PICOFM

#define PROGRAMID "picoFM"
#define PROG_VERSION   "1.0"


//*------------------------------------------------------------------------------------------------------------
//*--- Read Squelch control
//*------------------------------------------------------------------------------------------------------------

#define ZERO             0
#define SERIAL_MAX      16
#define VOLUME           5

#define HPF 0
#define LPF 0
#define PRE 0

//*-----------------------------------------------------------------------------------------------
//* Control lines and VFO Definition [Project dependent]
//*-----------------------------------------------------------------------------------------------


//*--- VFO initialization parameters

#define VFO_SHIFT          600000
#define VFO_START       144000000
#define VFO_END         147990000
#define VFO_STEP_10KHz      10000
#define VFO_STEP_5KHz        5000
#define VFO_STEP_1MHZ     1000000

//*--------------------------------------------------------------------------------------------------
//* Definitions to manage DRA818V
//*--------------------------------------------------------------------------------------------------

#define CTCSSCODEMAX 38

//*--- Control lines for the DRA818V

#define PTTPin          13
#define HLPin           12
#define PDPin           11


//*----------------------------------------------------------------------------------------------------
//* DDSPLUS interfaces
//*----------------------------------------------------------------------------------------------------
void  CTCSSUpdate();
void  SQLUpdate();
void  doHandShake();
void  doSetVolume();
void  doSetFilter();
void  doSetGroup();
void setWord(byte* SysWord,byte v, boolean val);
boolean getWord (byte SysWord, byte v); 



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

void defineMenu(){
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
//*-------------------------------------------------------------------------------
//* I/O Definitions for the DRA818V
//*-------------------------------------------------------------------------------
void pinSetup() {
  

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

//*--------------------------------------------------------------------------------------------
//* saveMenu
//* save specifics of picoFM
//*--------------------------------------------------------------------------------------------
void saveMenu() {

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
      return;

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

void readEEPROM(){
  
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
     return;
}

void writeEEPROM(){
 
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
  return;
}

void setFrequencyHook(long int f,FSTR* v) {

 long int fDDS=f/1000;
   
}

void showGUI(){
      showPwr();
      showRpt();
      showCTC();
      showDog();
      showSQL();
      showPTT();     
      showSPD();
      showDRF();
      return;

}


void setSysOM(){
  setWord(&USW,BUSY,false);
//===================================================================================
//* DRA018V module initialization and setup
//===================================================================================

  doHandShake();
  doSetVolume();
  doSetFilter();
  doSetGroup();

  return;
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
void handleSerialCommand() {
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

void handleTimerHook(){
  //*--- LCD Light delay control
  if (TDIM>0 && getWord(MSW,PTT)==true) {
      TDIM--;
      if (TDIM==0 && spd.get() == 1) {
         digitalWrite(10,LCD_OFF);
      }
  }
}

void showVFOHook(){
  lcd.setCursor(2,0);
  if (vx.vfoAB==VFOA){lcd.print("A");} else {lcd.print("B");}
  return;
}

#endif

