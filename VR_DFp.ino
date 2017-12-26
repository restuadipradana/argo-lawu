#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

//-------VR
VR myVR(2,3);    // 2:RX 3:TX, VR

uint8_t records[7]; // save record
uint8_t buf[64];

int kontak = 4; // relay1 kontak
int starter = 5; // relay2 starter
int kondisi = 8; //kondisi mesin

#define engStart    (0)
#define nyalakan    (1)
#define matikan    (2)
//#define offRecord   (1) 


//----DFp
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX DFp

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


//------------------------------------------------------------
void printSignature(uint8_t *buf, int len)
{
  int i;
  for(i=0; i<len; i++){
    if(buf[i]>0x19 && buf[i]<0x7F){
      Serial.write(buf[i]);
    }
    else{
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf  -->  VR module return value when voice is recognized.
             buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
             buf[1]  -->  number of record which is recognized. 
             buf[2]  -->  Recognizer index(position) value of the recognized record.
             buf[3]  -->  Signature length
             buf[4]~buf[n] --> Signature
*/
void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if(buf[0] == 0xFF){
    Serial.print("NONE");
  }
  else if(buf[0]&0x80){
    Serial.print("UG ");
    Serial.print(buf[0]&(~0x80), DEC);
  }
  else{
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if(buf[3]>0){
    printSignature(buf+4, buf[3]);
  }
  else{
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


//------------------------------------------------------------

void setup()
{
  Serial.begin(115200); //serial ke komputer
  myVR.begin(9600); // serial ke VR
  Serial.begin(115200);
  Serial.println("Elechouse Voice Recognition V3 Module\r\nControl LED sample");
  mySoftwareSerial.begin(9600);   //serial ke DFp

  pinMode(kontak, OUTPUT);
  pinMode(starter, OUTPUT);
  pinMode(kondisi, INPUT); 

  digitalWrite(kontak, HIGH);
  digitalWrite(starter, HIGH);
  //-----------debug

  //debug vr
  if(myVR.clear() == 0){
    Serial.println("Recognizer cleared.");
  }else{
    Serial.println("Not find VoiceRecognitionModule.");
    Serial.println("Please check connection and restart Arduino.");
    while(1);
  }

  
  //debug DFp
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));


  //definisi 

  if(myVR.load((uint8_t)engStart) >= 0){
    Serial.println("Start loaded");
  }
  if(myVR.load((uint8_t)nyalakan) >= 0){
    Serial.println("On loaded");
  }
  if(myVR.load((uint8_t)matikan) >= 0){
    Serial.println("Off loaded");
  }

  myDFPlayer.volume(20);  //Set volume value. From 0 to 30

}




void loop()
{
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

  
  int ret;
  ret = myVR.recognize(buf, 50);
  if(ret>0){
    switch(buf[1]){
      
      case engStart:
        digitalWrite(starter, LOW);
        delay(1500);
        digitalWrite(starter, HIGH);
        if(digitalRead(kondisi) == HIGH){
          myDFPlayer.play(1);  //Play the first mp3
        }
        break;
      
      case nyalakan:
        /** turn off Kontak*/
        digitalWrite(kontak, LOW);
        break;
      
      case matikan:
        myDFPlayer.play(2);
        delay(2000);
        /** turn off Kontak*/
        digitalWrite(kontak, HIGH);
        break;
      
      default:
        Serial.println("Record function undefined");
        break;
    }
    /** voice recognized */
    printVR(buf);
  }
}
