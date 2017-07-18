/*
Updated 19/03/2017

 */

 //BIBLIOTECAS//
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
/////////VARIABLES////////////////////
int alimsens1 = 10;
int alimsens2 = 11;
int pinTest=6;//llevaria interruptor no pulsador
int alim_clock=7;
String dataString="";
tmElements_t tm;
int dormirx= 1;
int desperx= 0;
int mini=0;
int pin_sleep_xbee=5;


/////FUNCIONES///////////////////////////////

/////////////////////////////////////////////
////////TEST
////////////////////////////////////////////*
/*
void Test(){
  dataString="TEST";
  digitalWrite(alim_clock,HIGH);
  delay(50);
  if (RTC.read(tm)) {
      if (tm.Minute<10){
     
        dataString += "0";
        dataString += String(tm.Minute);
      }
      else{
        dataString += String(tm.Minute);
          }
      dataString += ":";
      if (tm.Second<10){
     
        dataString += "0";
        dataString += String(tm.Second);
      }
      else{
        dataString += String(tm.Second);
          }}
  digitalWrite(alim_clock,LOW);
  digitalWrite(pin_sleep_xbee,desperx);
  delay(200);
  Transmitir();
  delay(200);
  digitalWrite(pin_sleep_xbee,dormirx);
  delay(7550);

}


*/////////////////////////////////////////////
////////SENSAR
////////////////////////////////////////////
void sensar(void){
  digitalWrite(alimsens1,HIGH);
  digitalWrite(alimsens2,HIGH);
  delay(10);
    for (int analogPin = 0; analogPin < 3; analogPin++) {

    double sensor = analogRead(analogPin);
    double sensor1=0;
    char sensor2[6];
    sensor1=0.0017*sensor-0.191;
    dtostrf(sensor1,4,3,sensor2);
    dataString += sensor2;
    if (analogPin < 2) {
      dataString += ",";
      
    }
    else{/////FUNC TIMESPTAMP
      digitalWrite(alimsens1,LOW);
      digitalWrite(alimsens2,LOW);
      Timestamp();
      }
  }
}
/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/
void Timestamp(void)
 {
      dataString += ",";
      if (RTC.read(tm)) {
      if (tm.Hour<10){
     
        dataString += "0";
        dataString += String(tm.Hour);
      }
      else{
        dataString += String(tm.Hour);
          }
      dataString += ":";
      if (tm.Minute<10){
     
        dataString += "0";
        dataString += String(tm.Minute);
      }
      else{
        dataString += String(tm.Minute);
          }
      dataString += "-";
      dataString += String(tm.Day);
      dataString += "/";
      if (tm.Month<10){
         dataString += "0";
         dataString +=(tm.Month);
      }
      dataString +="/";
      dataString +=String(tmYearToCalendar(tm.Year));
  }
  }

 
ISR(WDT_vect)
{}

  //////////////////////////////////////////////
  /***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ADCSRA &= ~(1<<7);
  //for (int j=0;j<2;j++){
  do{
  sleep_enable();
  MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); 
  sleep_mode();
  sleep_disable();
  digitalWrite(alim_clock,HIGH);
  delay(50);
  RTC.read(tm);
  digitalWrite(alim_clock,LOW);
  }while(CheckIfItIsTime(mini,tm.Minute,tm.Second)==false);
  ADCSRA |= (1<<7);
}
bool CheckIfItIsTime(int lastcheck,int minutes,int seconds){
  if(seconds<10||seconds>58){
    if (minutes==lastcheck)
        {
          return true;
        }
        else
        {
          return false;
        }
  }
  else{
    return false;
  }
}


//////////////////////////////////////
//*************SETUP***************//
void setup() {
  // Open serial communications and wait for port to open:
   pinMode(alimsens1, OUTPUT);
   pinMode(alimsens2,OUTPUT);
   pinMode(alim_clock,OUTPUT);
   pinMode(pin_sleep_xbee,OUTPUT);
   //pinMode(pinTest,INPUT);
   digitalWrite(pin_sleep_xbee,dormirx);
   Serial.begin(9600);
//   pinMode(pulsador1, INPUT); 
   /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
}

void Transmitir()
{
  Serial.println(dataString);
}

///////////////////////////////////////////////////
///MAIN LOOP
///////////////////////////////////////////////////
void loop() {
  dataString="";
  /*while(digitalRead(pinTest)){
    Test();
    dataString="";
    }*/
  //digitalWrite(alim_clock,LOW);
  //digitalWrite(alimsens2,LOW);
  // make a string for assembling the data to log:
  // String dataString = "";
  //  digitalWrite(alim,HIGH);
  //  delay(15); revisar
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another
 
  digitalWrite(alim_clock,HIGH);
  delay(10);
  sensar();
  digitalWrite(alim_clock,LOW);
  digitalWrite(pin_sleep_xbee,desperx);
  delay(200);
  Transmitir();
  delay(200);
  digitalWrite(pin_sleep_xbee,dormirx);
  mini=tm.Minute;
  enterSleep();
}
