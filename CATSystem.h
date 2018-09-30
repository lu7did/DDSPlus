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

//*--------------------------------------------------------------------------------------------------
//* Structure to exchange with other objects (API)
//*--------------------------------------------------------------------------------------------------
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
//* Define max size for serial queues and max number of command byte
//*============================================================================================
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

//*--- Now process characters in the queue

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
//* process Queue extract elements from the queue and process command if detected as completed
//*--------------------------------------------------------------------------------------------
void CATSystem::processQueue() {
 
 char h[40];
 char b[2];
 char BCD[2];
 
 if (pQueue==0) { return; }


#if DEBUGCAT
//*-------------------------------------------*
//* This parts executes only in DEBUGCAT mode *
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
//* This parts executes only in !DEBUGCAT mode*
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

#if DEBUGCAT   //* Process only in DEBUG mode
  
  char h[40];
  sprintf(h,"Sending [%x]",m);
  Serial.println(h);

#else          //* Process when not in DEBUG mode

  Serial.println(m);

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

