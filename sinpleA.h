//*--------------------------------------------------------------------------------------------------
//* Specific Headers for sinpleA implementation
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
//*--- Program & version identification


#if SINPLEA

#define PROGRAMID "sinpleA"
#define PROG_VERSION   "1.0"
#define PROG_BUILD  "000"
#define COPYRIGHT "(c) LU7DID 2018"

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
#define VFO_STEP_1MHz     1000000
#define VFO_STEP_100Hz        100
#define VFO_STEP_100KHz    100000
#define VFO_RESET         7000000

#endif

#if SINPLEA
#define BANDMAX 3


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
//*------------------------------------------------------------------------------------------------
//* Set here SINPLEA Menu definitions
//*------------------------------------------------------------------------------------------------

#if SINPLEA

void BandUpdate();
MenuClass band(BandUpdate);
MenuClass vfo(NULL);
MenuClass stp(NULL);



#endif
