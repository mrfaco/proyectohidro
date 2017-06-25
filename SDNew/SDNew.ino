#include <SPI.h>
#include <SD.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <LiquidCrystal.h>

const int waitingTime = 66;
const int sdPin=A0;
const int lcdPowerPin = A4;
const int xbeeSleep=1;
const int xbeeAwake=0;
const int xbeePin=A3;
const unsigned long waitedTooMuch=63000;
LiquidCrystal lcd(9,8,7,6,5,4);
String GlobalDataStringForLCD = "";


///////INTERRUPCIONES//////////
///////
///////Interr. WDog. p/despertar micro cada 8 seg
ISR(WDT_vect){}
//////////////////////////////////////
///////Interrup. p despertar micro al recibir paq.
////////////////////////////////////////////////OK

void setup() {
  Serial.begin(9600);
  setupSPI();
  SetupWatchdog();
  pinMode(3,INPUT_PULLUP);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  pinMode(A0,OUTPUT);
  digitalWrite(A0,LOW);
  pinMode(2,INPUT_PULLUP);
  ShutPins();  
  digitalWrite(A3,LOW);
  digitalWrite(A4,HIGH);
  lcd.begin(16, 2);
  digitalWrite(A4,LOW);
}

void ShowLCD () {
  digitalWrite(lcdPowerPin,HIGH);
}

void PrintDisplay () {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(GlobalDataStringForLCD.substring(0,4));
  lcd.setCursor(4, 0);
  lcd.print(GlobalDataStringForLCD.substring(5,10));
  lcd.setCursor(9, 0);
  lcd.print(GlobalDataStringForLCD.substring(11,16));
  lcd.setCursor(0, 1);
  lcd.print(GlobalDataStringForLCD.substring(18));
  delay(8000);
  ShutPins();
  digitalWrite(A4,LOW);
}

void ShutPins () { 
  for(int i=4; i<11;i++){
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);  
  }
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void SetupWatchdog(void) {
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

void WaitForData(){
  while (!Serial.available()){}
}

String ReadIncomingData(){
    String output="";
    while (Serial.available()) {
      delay(3);  //delay to allow buffer to fill 
      if (Serial.available() >0) {
        char c = Serial.read();  //gets one byte from serial buffer
        output += c; //makes the string readString
        } 
      }
      return output;
   }

void setupSPI() {                
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  SD.begin(10);//CHIP SELECT = 10
}

void WriteData(String dataString){      
    digitalWrite(sdPin,HIGH);
    delay(100);
    setupSPI();
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    }
    delay(100);
    digitalWrite(sdPin,LOW);
}

void Sleep(){
    sleep_enable();
    MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
    MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6);
    sleep_mode();
    sleep_disable();
}

void ShutEverythingDown(){
  SetupWatchdog();
  digitalWrite(xbeePin,xbeeSleep);
  attachInterrupt(digitalPinToInterrupt(3),ShowLCD,LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ShutPins();
}

void TurnEverythingOnAgain(){
  digitalWrite(xbeePin,xbeeAwake);
  detachInterrupt(digitalPinToInterrupt(3));
  wdt_disable();
}

void GoToSleep(int skippedTime){
  for(int i=0;i<(waitingTime-skippedTime);i++) {  ///i<(Cantidad de tiempo que queremos ard dormido)/9s...1hr=3600s 3600/9=400 guard depende del tiempo de ventana, 45s => guard 5s
    Sleep();
    if(digitalRead(lcdPowerPin)){
      PrintDisplay();
      i++;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long wait=0;
  WaitForData();
  String result = "";
  bool exitLoop=false;
  int skippedTime = 0;
  while(exitLoop==false)
  {
    wait=millis();
    result = ReadIncomingData();
    
    if(result==""){
      if(wait>waitedTooMuch){
        skippedTime=2;
        exitLoop=true;
      }
    }
    
    else{
      int skippedTime=0;
      WriteData(result);
      GlobalDataStringForLCD = result;
      exitLoop=true;
    }
  }
  delay(10);
  ShutEverythingDown();
  GoToSleep(skippedTime);
  TurnEverythingOnAgain();
}
