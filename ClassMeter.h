
//*--------------------------------------------------------------------------------------------------
//* ClassMeter Class   (HEADER CLASS)
//*--------------------------------------------------------------------------------------------------
//* Este es el firmware del diseÃ±o de una clase para gestionar medidores analógicos
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#ifndef ClassMeter_h
#define ClassMeter_h

#include "LinkedList.h"
#include "Arduino.h"


class MeterClass

{
  public:
  typedef void (*CALLBACK)();
  
      typedef struct {
          char* mText;
          MeterClass* mChild;
      } List;

      
      byte mItem;
      byte mItemBackup;
      List x;
      LinkedList<List*> l = LinkedList<List*>(); 
      

      
      MeterClass(CALLBACK u,CALLBACK d,CALLBACK s);
      
      void  add(char* t,MeterClass* m);
      char* getText(byte i);
      void  set(byte i);
      byte  get();
      byte  getBackup();
      
      void  setText(byte i,char* c);
      boolean isUpdated();
      int readV(byte VPIN,int VMAX,int VSCALE);


      
      MeterClass* getChild(byte i);
      void  move(boolean CW,boolean CCW);
      void  backup();
      void  restore();
      void  save();
      //List* getmem();
      
      CALLBACK update;
      CALLBACK display;
      CALLBACK saving;
      
  private:    
      
      
  
};

//*-------------------------------------------------------------------------------------------------
//* Constructor
//*-------------------------------------------------------------------------------------------------
MeterClass::MeterClass(CALLBACK u,CALLBACK d,CALLBACK s) {
   
   update=u;
   display=d;
   saving=s;
   
   mItem=0;
   mItemBackup=0;
   
   return;
}
//*-------------------------------------------------------------------------------------------------
//* Set current Index by force
//*-------------------------------------------------------------------------------------------------

void MeterClass::set(byte i) {
  mItem=i;
  return;
}
//*-------------------------------------------------------------------------------------------------
//* Add element to the menu, the child menu is optional
//*-------------------------------------------------------------------------------------------------
void MeterClass::add(char* t,MeterClass* m) {

     List* x = (List*)malloc(sizeof(List));
     
     x->mText=t;
     x->mChild=m;
     
     l.add(x);
}
//*-------------------------------------------------------------------------------------------------
//* Get the item pointer associated with the menu element
//*------------------------------------------------------------------------------------------------ 
byte MeterClass::get() {
  return mItem;
}
//*-------------------------------------------------------------------------------------------------
//* Get the item pointer associated with the menu element
//*------------------------------------------------------------------------------------------------ 
byte MeterClass::getBackup() {
  return mItemBackup;
}
//*-------------------------------------------------------------------------------------------------
//* Get the text associated with the ith menu element
//*------------------------------------------------------------------------------------------------ 
char* MeterClass::getText(byte i) {
  return (char*)l.get(i)->mText;
}
//*-------------------------------------------------------------------------------------------------
//* Get the text associated with the ith menu element
//*------------------------------------------------------------------------------------------------ 
void MeterClass::setText(byte i,char* c) {
  
  l.get(i)->mText=c;
  return;
}

//*-------------------------------------------------------------------------------------------------
//* Get the pointer to the menu associated with the ith menu element (optional)
//*------------------------------------------------------------------------------------------------ 
MeterClass* MeterClass::getChild(byte i) {
  return l.get(i)->mChild;
}
//*-------------------------------------------------------------------------------------------------
//* Advance or retry menu position, take care of borders
//*------------------------------------------------------------------------------------------------  
void MeterClass::move(boolean CW,boolean CCW){

  if (l.size() == 0) {return; }  //Is list empty? Return
  if (update!=NULL) {
      update();
      if (display!=NULL) {display();}
      return;
  }
  if (mItem < l.size()-1 && CW==true) {
     mItem++;
     if (display!=NULL) {display();}
     return;
  }
  if (mItem>0 && CCW==true) {
     if (display!=NULL) {display();}
     mItem--;
     return;
  }

  if (mItem ==l.size()-1 && CW==true) {
     if (display!=NULL) {display();}
     mItem=0;
     return;
  }
  if (mItem==0 && CCW==true) {
     if (display!=NULL) {display();}   
     mItem=l.size()-1;
     return;
  }
  
} 
//*-------------------------------------------------------------------------------------------------
//* Backup current object pointer
//*------------------------------------------------------------------------------------------------       
void MeterClass::backup(){
  
  byte i=get();
  MeterClass* z=getChild(i); 
  z->mItemBackup=z->mItem;

}
//*----------------------------------------[LCD_FSM]---------------------------------------------------
//* Read analog pin and returns a scaled value
//*--------------------------------------------------------------------------------------------------
int MeterClass::readV(byte VPIN,int VMAX,int VSCALE) {

return (int)( (analogRead(A5)/113) - 1);

}
//*-------------------------------------------------------------------------------------------------
//* Restore previous object pointer (reverse a change)
//*------------------------------------------------------------------------------------------------       
void MeterClass::restore(){
  //mItem=mItemBackup;
  byte i=get();
  MeterClass* z=getChild(i); 
  z->mItem=z->mItemBackup;

}
//*-------------------------------------------------------------------------------------------------
//* Restore previous object pointer (reverse a change)
//*------------------------------------------------------------------------------------------------       
void MeterClass::save(){
  byte i=get();
  MeterClass* z=getChild(i); 

  z->mItemBackup=z->mItem;

   if (saving==NULL) {return;}
   
   saving(); 
  
}
//*-------------------------------------------------------------------------------------------------
//* Restore previous object pointer (reverse a change)
//*------------------------------------------------------------------------------------------------       
boolean MeterClass::isUpdated(){
  byte i=get();
  MeterClass* z=getChild(i);   
   if(z->mItem != z->mItemBackup) {
     return true;
   } else {
     return false;
   }
   return false;
  
}   
#endif


