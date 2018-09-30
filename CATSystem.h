//*--------------------------------------------------------------------------------------------------
//* CATSystem CAT Management Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de CAT para DDS
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef CATSystem_h
#define CATSystem_h

#include "Arduino.h"

typedef struct {
  byte    cmd;
  byte*   bCAT;
  byte*   pCAT;
  byte    rc;
  boolean r;
} APISTR;

typedef void (*CALLBACK)();
typedef byte (*CALLAPI)(APISTR* a);
#define DEBUGCAT true


//*============================================================================================
//* Define band limits
//*============================================================================================
//*---- HF band definition

#define QUEUEMAX  16        // Queue of incoming characters 
#define CATMAX 5            // Queue of CAT commands
//*---------------------------------------------------------------------------------------------------
//* Function prototypes & interfaces 
//*---------------------------------------------------------------------------------------------------
int x2i(String Hex);
uint8_t byte2bcd(uint8_t bx);
byte bcd2byte(byte bcd);
int uppercase (int charbytein);
//*---------------------------------------------------------------------------------------------------
//* CATSystem CLASS
//*---------------------------------------------------------------------------------------------------
class CATSystem
{
  public:   
      CATSystem(CALLBACK a);
      void addQueue(char c);
      char getQueue();
      void analyzeCAT();
      void processQueue();
      void shiftCAT();
      void clearCAT();
      boolean isQueueFull();
      void sendCAT(byte m);

      CALLAPI  api=NULL;
      CALLBACK hook=NULL;

      APISTR   a;
  
  private:
      boolean _boot;
      char Q[QUEUEMAX];           // Actual Queue space [a0,a1,...,an]
      byte pQueue = 0;            // Pointer to next position to use
      byte bufferCAT[CATMAX];     // Actual buffer
      byte pCAT=0;                // pointer to next position

};
#endif
//*---------------------------------------------------------------------------------------------------
//* CATSystem CLASS Implementation
//*---------------------------------------------------------------------------------------------------
//*--------------------------------------------------------------------------------------------------
//* CATSystem CAT Management Class   (CODE)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de CAT para DDS - FT817 clone
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//*****************************************************************************************************
//*                               CAT Finite Status Machine (FSM)
//*                             Implemented at SerialEvent Handler
//*
//*
//*
//*
//*****************************************************************************************************
//*======================================================================================================
//*                                   CAT Sub-System
//*
//*======================================================================================================
CATSystem::CATSystem(CALLBACK s)
{

//*---- This is the API callback
  
 hook=s;
 
}

//*--------------------------------[Queue Manager]---------------------------------------------------
//* Manages the queue, add element to the queue
//*--------------------------------------------------------------------------------------------------
void CATSystem::addQueue(char c) {

if(pQueue < QUEUEMAX)
{
  Q[pQueue] = c;
  pQueue++;
}

//*--- Now process it
processQueue();

}
//*--------------------------------------------------------------------------------------------------
//* Check if the Queue is full
//*--------------------------------------------------------------------------------------------------
boolean CATSystem::isQueueFull(){
  
  if (pQueue<=(QUEUEMAX)-1){
     return false;
  } else {
    return true;   
  }
}

//*--------------------------------------------------------------------------------------------------
//* Manages the queue, remove elemento from the queue
//*--------------------------------------------------------------------------------------------------
char CATSystem::getQueue() {
  char c;
  
  if(pQueue > 0)
  {
    c = Q[0];
    for(byte i=0; i<pQueue; i++)
    {
      Q[i] = Q[i+1];
    }
    pQueue--;
    return c;
  }
  return char(0x00);
}

//*--------------------------------------------------------------------------------------------
//* process HEX input (esto hay que retrabajarlo muchisimo)
//*--------------------------------------------------------------------------------------------
void CATSystem::processQueue() {
 
 char h[40];
 char b[2];
 char BCD[2];
 
 if (pQueue==0) { return; }


#if DEBUGCAT
//*-------------------------------------------*
//* This parts executes only in DEBUGCAT mode    *
//*-------------------------------------------* 
 if (pQueue>=4) {
   
    //*--- Start of DEBUGCAT
    
    if (Q[0] == '0' && (Q[1] == 'x' || Q[1] == 'X')) {
      
      b[0]=getQueue();
      b[0]=getQueue();
      
      b[0]=getQueue();
      b[1]=getQueue();
      b[2]='\0';
      
      String s=String(b);
      
      int x=x2i(s);
      
      bufferCAT[pCAT]=(byte)x;
      pCAT++;

      sprintf(h,"HEX(%x) Pos(%d)",x,pCAT);
      Serial.println(h);

      if(pCAT>=CATMAX) {
        analyzeCAT();
      }

      
    } //*--- End of DEBUGCAT
 }

#else
//*-------------------------------------------*
//* This parts executes only in !DEBUGCAT mode   *
//*-------------------------------------------* 

   while (pQueue>0) {      
      bufferCAT[pCAT]=getQueue();
      pCAT++;

      if(pCAT>=CATMAX) {
        analyzeCAT();
      }
   }
   
#endif

  return;  
}

//*--------------------------------------------------------------------------------------------
//* analyzeCAT()
//* Parse CAT sequence and store buffer
//*--------------------------------------------------------------------------------------------
void CATSystem::analyzeCAT() {

byte rc;
byte b[4];
unsigned long fx=0;
unsigned long k=0;

a.cmd=bufferCAT[CATMAX-1];

a.rc=0x00;
a.bCAT=bufferCAT;
a.pCAT=&pCAT;
a.r=true;

if (hook!=NULL) {
  hook();
} else {
  clearCAT();
  return;
}

 //*----- Send CAT response and clear command area
 
 if (a.r==true){
    sendCAT(a.rc);
 }
 
 clearCAT();
 return;



 switch(bufferCAT[CATMAX-1]) {

 case 0x00   :{
               //Serial.println("CAT Command 0x00 (LOCK) -- N/I");
              //*--- Must return 0x00 if not locked 0xf0 if locked, as locked is not supported return always CAT_LOCKED
              rc=0x00;
              break;
              }
 
 case 0x01   : {//Process CAT FREQ (OK)
               //Serial.println("CAT Command 0x01 (FREQ)");
               fx=0;
               k =1000000;
               for (int i=0;i<4;i++) {
                   b[i]=bcd2byte(bufferCAT[i]);
                   fx=fx+(b[i]*k);
                   k=k/100;
               }  
               //*---> v.set(v.vfoAB,fx*10);
               //*----->CALLBACK vfo[bandMenu.mItem][vfoMenu.mItem]=fx*10;              
               //*----->CALLBACK showPanel();
               clearCAT();
               return;
               }
 
 case 0x02   : { //Process CAT SPLIT ON/OFF command (OK)
                //Serial.println("CAT Command 0x02 (SPLIT ON)");
                /* CALLBACK Split
                if (splMenu.mItem == 0) {
                    rc=0x00;
                } else {
                    rc=0xf0;
                }
                splMenu.mItem=1;
                showPanel();
                */
                rc=0x00;
                break;
               }
 
  case 0x03  : {
                //Serial.println("CAT Command 0x03 (READ STATUS)"); (OK)
                //* Return 5 bytes with status XX XX XX XX 01
                /* CALLBACK Band Status*/
                //*----> fx=v.get(v.vfoAB)/10;
                //fx=vfo[bandMenu.mItem][vfoMenu.mItem]/10;
                k=1000000;
                unsigned long j=0;
                for (int i=0;i<4;i++) {
                   j=fx/k;
                   b[i]=byte2bcd((byte)j);
                   fx=fx-(j*k);
                   k=k/100;
                   sendCAT(b[i]);                   
                 }  
                rc=0x01;
                break;

                /*
                switch(modeMenu.mItem) {
                  case 0: {sendCAT(0x02); break;}
                  case 1: {sendCAT(0x03); break;}

//*=========================================================================================================                 
                  #if WSPR
                      case 2: {sendCAT(0x0c); break;}   //FT-817 PKT (0x0c) assigned to WSPR mode here
                  #endif
//*=========================================================================================================

                  default:{sendCAT(0x02); break;}
                }
                */
                
   
                return;
               }
 
 case 0x05   : {                              //Process CAT RIT ON/OFF command
                //Serial.println("CAT Command 0x05 (RIT ON)");
                /*CALLBACK RIT
                if (ritMenu.mItem == 0) {
                    rc=0x00;
                } else {
                    rc=0xf0;
                }
                ritMenu.mItem=1;
                showPanel();
                */
                rc=0x00;
                break;

                }

case 0x07  : {
                //Serial.println("CAT Command 0x07 (OPERATING MODE)");
                /*CALLBACK Operating Mode
                switch(bufferCAT[0]){
                  case 0x02: {
                             modeMenu.mItem = 0x00;
                             }
                  case 0x03: {
                             modeMenu.mItem = 0x01;
                  }
//*=========================================================================================================                  
                  #if WSPR
                      case 0x0c: {
                                  modeMenu.mItem = 0x02;
                      }
                  #endif
//*=========================================================================================================
                  

                  default:   {
                             modeMenu.mItem = 0x00;
                  }
                }
                
                showPanel();
                */
                rc=0x01;
                break;
                
               }

case 0x08   :{
               //Serial.println("CAT Command 0x08 (PTT ON)");
              //*--- Must return 0x00 if not keyed 0xf0 if already keyed
              /* CALLBACK PTT
               *  
               
              if (getWord(SSW,KEY) == false) {
                  rc=0x00;
              } else {
                  rc=0xf0;    
              }
              doKeyDown(0);
              showPanel();
              */
              rc=0x00;
              break;
              }

case 0x10   :{
               //Serial.println("CAT Command 0x10 (PTT STATUS) UNDOC");
              //*--- Must return 0x00 if not keyed 0xf0 if already keyed
              /* CALLBACK PTT OFF
              if (getWord(SSW,KEY)==false) {
                 rc=0x00;
              } else {
                 rc=0xf0;
              }
              */
              rc=0x00;
              break;
              }            

case 0x80   :{
               //Serial.println("CAT Command 0x80 (LOCK OFF)");
              //*--- Must return 0x00 if already locked 0xf0 if not already locked
              rc=0x00;
              break;
              } 
              
 case 0x81   :{
               //Serial.println("CAT Command 0x81 (TOGGLE VFOA/B)");
              //*--- Toggle between VFO-A and VFO-B no response 
              /* CALLBACK SWAP
              if (vfoMenu.mItem == VFOA) {
                 vfoMenu.mItem=VFOB;
              } else {
                 vfoMenu.mItem=VFOA;
              }
              showPanel();
              */
              //*----> v.swapVFO();
              clearCAT();
              return;
              }            

 case 0x82   :{
               //Serial.println("CAT Command 0x82 (SPLIT OFF)");
              //*--- Must return 0x00 if already split 0xf0 if not already split
              /* CALLBACK SPLIT
               *  
               
              if (splMenu.mItem == 0) {
                  rc=0xf0;
              } else {
                  rc=0x00;    
              }
              splMenu.mItem=0;
              showPanel();
              */
              rc=0x00;
              break;
              }  

case 0x85   :{
               //Serial.println("CAT Command 0x82 (RIT OFF)");
              //*--- Must return 0x00 if already split 0xf0 if not already split
              /* CALLBACK RIT
              if (ritMenu.mItem == 0) {
                  rc=0x0f;
              } else {
                  rc=0x00;    
              }
              ritMenu.mItem=0;
              showPanel();
              */
              rc=0x00;
              break;
              }           

case 0x88   :{
               //Serial.println("CAT Command 0x88 (PTT OFF)");
              //*--- Must return 0x00 if already keyed 0xf0 if not already keyed
              /* CALLBACK PTT OFF
              if (getWord(SSW,KEY)==true) {
                 rc=0x00;
              } else {
                 rc=0xf0;
              }
              doKeyUp();
              doKeyEnd();    
              showPanel();
              */
              rc=0x00;
              break;
              }           

case 0xa7   :{
               //Serial.println("CAT Command 0xa7 (Radio Configuration) UnDoc");
              //*--- Return an area of data, fake answer based on recommendation from KA7OEI web page 
              sendCAT(0xA7);
              sendCAT(0x02);
              sendCAT(0x00);
              sendCAT(0x04);
              sendCAT(0x67);
              sendCAT(0xD8);
              sendCAT(0xBF);
              sendCAT(0xD8);
              sendCAT(0xBF);
              //*---    
              clearCAT();
              return;
              }           
case 0xba   :{
               //Serial.println("CAT Command 0xba (Unknown) UnDoc");
              //*--- Returns 0x00 
              rc=0x00;    
              break;
              }     
case 0xbd   :{
               //Serial.println("CAT Command 0xbd (Read TX Meter) UnDoc");
              //*--- Returns 0x00 on RX return [xx][yy] on TX with BCD encoding of PWR,VSWR,ALC,MOD 
              sendCAT(0x00);  //* PWR & VSWR     --- TO BE IMPLEMENTED FROM METER SIGNAL --
              sendCAT(0x00);  //* ALC & MOD 
              clearCAT();
              return;
              }               
case 0xe7   :{
               //Serial.println("CAT Command 0xe7 (Read Receiver status)");
              //*--- Returns [byte] with receiver status 
              rc=0x00;
              //rc=rc | Smeter;
              break;
              }
case 0xf5   :{
               //Serial.println("CAT Command 0xf5 (Set clarifier status)");
              //*--- receive [dir][x][10/1 KHz][100/10 Hz] 
              /* CALLBACK Clarifier status
              fx=0;
              k =100;

              for (int i=2;i<4;i++) {
                   b[i]=bcd2byte(bufferCAT[i]);
                   fx=fx+(b[i]*k);
                   k=k/100;
              }  

              ritShift=fx*10;
              if (bufferCAT[0]!=0) {
                 ritShift=-1*ritShift;
              }
              
              showPanel();
              */
              clearCAT();
              return;
              }              
case 0xf7   :{
               //Serial.println("CAT Command 0xf7 (Read TX Status)");
              //*--- return [byte] with status 
              rc=0x00;  
              /* CALLBACK TX COMMAND  
              if(getWord(SSW,KEY)==true){rc=rc | B1000000;}
              if(splMenu.mItem==1)      {rc=rc | B0010000;}
              rc=rc | B0001111;    //* Meter as full needs to be reworked later
              */
              break;
              }              

 //*--- Several not implemented
 case 0xf9:
 case 0xbe:
 case 0xbb:
 case 0xbc:
 case 0x8f:
 case 0x0f:
 case 0x09:
 case 0x0a:
 case 0x0b:
              clearCAT();
              return;
 
 default     : {
              shiftCAT(); 
              return;
               }  
 }

 //*----- Send CAT response and clear command area
 
 sendCAT(rc);
 clearCAT();
 return;
}

//*------------------------------------------------------------------------------------------------------
//* Purge one character from CAT buffer
//*------------------------------------------------------------------------------------------------------
void CATSystem::shiftCAT() {
   
   for (int i=0;i>CATMAX-2;i++) {
      bufferCAT[i]=bufferCAT[i+1];
   }   
   bufferCAT[CATMAX-1]=0x00;
   pCAT--;
  
}
//*------------------------------------------------------------------------------------------------------
//* Clear CAT buffer
//*------------------------------------------------------------------------------------------------------
void CATSystem::clearCAT() {
   
   for (int i=0;i>CATMAX-1;i++) {
      bufferCAT[i]=0x00;
   }   
   pCAT=0;
  
}
//*--------------------------------------------------------------------------------------------
//* sendCAT
//* Response to a CAT command (1 byte)
//*--------------------------------------------------------------------------------------------
void CATSystem::sendCAT(byte m){

#if DEBUGCAT
  
  char h[40];
  sprintf(h,"Sending [%x]",m);
  Serial.println(h);

#endif

}


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

