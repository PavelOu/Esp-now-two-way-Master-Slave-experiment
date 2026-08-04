//Esp_now_SQ1A_M_NodeMcu_en_4
//based on Esp_now_NodeMcu_S1A_M_en_4
// more quick version of slave station im M-S more quick system
//sketch for slave station A with NodeMcu V1.0 ESP-12E
// based on Rui Santos tutorials 
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp8266-nodemcu/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

String sketchNameSlave = "";
String sketchNameSlaveA = "Esp_now_SQ1A_M_NodeMcu_AD8232_en_4";

//String sketchNameSlaveB = "Esp_now_Wemos_D1_R1_HW-827_S2B_M_en_4";
//String sketchNameSlaveC = "Esp_now_NodeMcu_CP_S3C_M_en_4";

String sketchNameMaster = "Esp_now_M_SQ_NodeMcu_en_4";


int slaveNumber = 1;
int slaveNumberOnLine = 0;



// REPLACE WITH THE MAC Address of your receivers or sender
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t masterAddress[] = {0xEC, 0xFA, 0xBC, 0x8F, 0x64, 0x48};   // ESP-now - master V1(3) NodeMcu V0.9 ESP-12

String boardMacAddress = "Wifi STA slave board mac address";
String masterMacAddress = "EC:FA:BC:8F:64:48";

//NodeMcu V1.0 ESP-12E mac address
//uint8_t slaveMacAddressA[] = {0x84, 0xCC, 0xA8, 0x97, 0xEE, 0x08};   // ESP-now - slave B NodeMcu V1.0 ESP-12E

//Wemos D1 R1 Lolin mac address 48:55:19:17:1C:50
//uint8_t slaveMacAddressB[] = {0x48, 0x55, 0x19, 0x17, 0x1C, 0x50};   // ESP-now - slave B Wemos D1 R1 ESP-12

//NodeMcu Esp12-E V3 mac addres BC:DD:C2:3D:66:9A
//uint8_t slaveMacAddressC[] = {0xBC, 0xDD, 0xC2, 0x3D, 0x66, 0x9A};   // ESP-now - slave C NodeMcu V0.9 ESP-12E V3

//uint8_t slaveMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // test

#define redLED 12 // D6 = red Led led
#define yellowLED 13 // D7 = yelloe Led led
#define greenLED 15 // D8 = Green led
#define analogPin 0
//#define LoPlus 4 // D4 D14 GPIO4    //not used
//#define LoMinus 5 // D3 D15 GPIO5

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message
 {  
  byte dataCommand;
  unsigned int data1esp;
 } struct_message;

// Create a struct_message called espReadMessage to hold sensor readings
struct_message espReadMessage;

// Create a struct_message called espSendMessage to hold sensor readings
struct_message espSendMessage;

byte dataAcommand;
//byte dataAstatus;
//unsigned int dataAindex;
unsigned int dataA1esp;
unsigned int dataA2;
unsigned int dataA3;
byte dataA4;

bool espParOn = false;
bool espOscOn = false;

/*
//rest ot tests with AD8232
bool LoPlusInt = 0;
bool LoMinusInt = 0;
*/

unsigned int dataAnalog;
//unsigned int dataOffset = 0;
//unsigned int dataPar[6]; 
// 0=not used, 1=dataStep, 2=dataPocetOsc, 3=dataTime, 4=dataOffset
// 5=dataPocetOscMax

//unsigned int dataAnalogArray[401];
//unsigned int dataPocetOscMax = 401;
//unsigned int dataPocetOsc = 401;
bool dataStopOn = false;
//unsigned long timeStart = 0;

unsigned int espWaitTime = 100;
int espWaitTimeStepDown = 100;
bool espSendOK = false;
bool espReceivedOK = false;

byte dataStep = 20;
unsigned int dataTime = 0;

bool serialPrintOn = false;
bool espNowOK = false;
bool serialRxOn = 0;

byte loopMode = 0;

int I = 0;
int J = 0;

bool menuOn = false;
char markRx = 0;



// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
 {
  if (sendStatus == 0)
   {
    espSendOK = true;
    if (serialPrintOn)
     {
      Serial.print("Last Packet Send Status on Command ");
      Serial.print(dataAcommand);
      Serial.println(" : Delivery success");    
     }
   }     
  else
   {
    if (serialPrintOn)
     {
      Serial.print("Last Packet Send Status on Command ");
      Serial.print(dataAcommand);
      Serial.println(" : Delivery fail");
     }
   }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *espReadData, uint8_t len) 
 {
  //memcpy(&incommingMessage, incommingData, sizeof(incommingMessage));
   memcpy(&espReadMessage, espReadData, sizeof(espReadMessage));
   if (serialPrintOn)
    {
     Serial.print("Bytes received: ");
     Serial.println(len);
    }
   dataAcommand = espReadMessage.dataCommand;
   dataA1esp = espReadMessage.data1esp;
   //dataA2 = espReadMessage.data2;
   //dataA3 = espReadMessage.data3;
   //dataA4 = espReadMessage.data4;

   if (serialPrintOn)
     {
      //Display Readings in Serial Monitor
      PrintReadMessage();
     }

   if (dataAcommand == 65)    // A - read analog data and send one to master
      {   
       //dataAindex = 0;
       loopMode = 1;    
      }  

   if ((dataAcommand == 66) || (dataAcommand == 67) ||(dataAcommand == 68))  // B,C,D - read analog data and send one to master
      {   
       //dataAindex = dataA2;
       if (serialPrintOn)
            {
              Serial.print("B(C,D) received dataStep = ");
              Serial.println(dataStep);
            } 
       //dataAstatus = 1;
       loopMode = 1;    
      }        

   if ( dataAcommand == 70 )            // F - menu and next transfer detail are print on
          {
            serialPrintOn = true;    
            menuOn = true;               
            digitalWrite(greenLED, LOW);
            digitalWrite(yellowLED, HIGH);
            digitalWrite(redLED, LOW);
            delay(500);
            digitalWrite(yellowLED, LOW);
            loopMode = 0;
          }

   if ( dataAcommand == 71 )            // G - yellow led on
        {
         loopMode = 7;
        }

   if ( dataAcommand == 72 )            // H - red led on
        {
         loopMode = 7;
        }


   if ( dataAcommand == 74 )            // J - green led on
          {
            loopMode = 7;
          }

   if ( dataAcommand == 75 )            // J - green led on
          {
            loopMode = 1;
          }

  if ( dataAcommand == 81 )            // Q - data print Off
      {
        serialPrintOn = false;     
        menuOn = false;              
        digitalWrite(yellowLED, HIGH);
        delay(500);
        digitalWrite(yellowLED, LOW);
        loopMode = 0;
      }


    if (dataAcommand == 84)    // T - espWaitTime
     {      
      espWaitTime = dataA4;
      digitalWrite(yellowLED, HIGH);
      delay(500);
      digitalWrite(yellowLED, LOW);
      loopMode = 0;
     }

   digitalWrite(yellowLED, LOW);
   espReceivedOK = true;
}
   
void LoadSendMessage()
 {
    espSendMessage.dataCommand = dataAcommand;
    espSendMessage.data1esp = dataA1esp;
    //espSendMessage.data2 = dataA2;
    //espSendMessage.data3 = dataA3;
    //espSendMessage.data4 = dataA4;
 }

void PrintReadMessage()
 {
  // Display Readings in Serial Monitor
  Serial.println("Incomming READINGS");
  Serial.print("Command: ");
  Serial.print(dataAcommand);
  Serial.print(", dataA1esp: ");
  Serial.println(dataA1esp);
}

void PrintSendMessage()
 {
  // Display Readings in Serial Monitor
  Serial.println("Outgoing message:");
  Serial.print("Command: ");
  Serial.print(dataAcommand);
  Serial.print(", dataA1esp: ");
  Serial.println(dataA1esp);
}

void PrintMenuLocal()
  {
    digitalWrite(greenLED, LOW);      
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
    Serial.println();
    Serial.println(sketchNameSlave);
    Serial.println("ESP-NOW initialisation:");    
    // Init ESP-NOW test
    if (!espNowOK) 
     {
      Serial.println("Error in initializing ESP-NOW !");
     }
    else
     {
      Serial.println("Initializing ESP-NOW OK !");
     }
    Serial.print("Slave Mac Address: ");
    Serial.println(boardMacAddress);
    Serial.println();
    Serial.print("Master Mac Address: ");
    Serial.println(masterMacAddress);
    Serial.println();
    Serial.print("Slave Number: ");
    Serial.println(slaveNumber);
    Serial.println();
    Serial.print("Slave Number On line: ");
    Serial.println(slaveNumberOnLine);
    Serial.println();
    Serial.print("Data to/from Esp-now-two-way station ");
    Serial.println();
    Serial.println("Menu :");
    Serial.println("I=Station info");
    Serial.println("A=Data test and meas, B,C,D=send data out in cycle in the different form,");
    //Serial.println("Exx=Pause(ms), Lxxx=Nr of data, Oxxx=Ofset of data,");
    Serial.println("F=send details, Q=stop send details, S=Stop data cycle");
    Serial.println("G=Yellow, H=Red, J=Green Led On");
    //Serial.println("M=Meas data and save them to memory, N=Read saved data from memory");
    //Serial.print("Max number of data saved to memory: ");
    //Serial.println(dataPocetOscMax-1);
    Serial.println();
    Serial.println("Print data is On");
    Serial.println();
    espSendOK = true;
    espReceivedOK = true;      
    loopMode = 0;   
    serialPrintOn = true;
    menuOn = false;
    delay(500);
    digitalWrite(yellowLED, LOW);      
  }

 
void setup() 
{
  // Init Serial Monitor
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  sketchNameSlave = sketchNameSlaveA;
  Serial.println(sketchNameSlave);
  pinMode(analogPin, INPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  //pinMode(LoPlus, INPUT);
  //pinMode(LoMinus, INPUT);
  digitalWrite(greenLED, HIGH);     
  delay(1000);
  digitalWrite(greenLED, LOW);        
  digitalWrite(yellowLED, HIGH);     
  delay(1000);
  digitalWrite(yellowLED, LOW);        
  digitalWrite(redLED, HIGH);     
  delay(1000);
  digitalWrite(redLED, LOW);        
  I = 1;
  J = 1;
  //dataPar[0] = 0;
  dataStep = 20;
  //dataPar[1] = dataStep;
  //dataPocetOsc = 401;
  //dataPar[2] = 0;
  dataTime = 0;
  //dataPar[3] = dataTime;
  //dataOffset = 0;
  //dataPar[4] = dataOffset;
  //dataPar[5] = dataPocetOscMax-1;
  serialPrintOn = true;
  //menuOn = false;  //run verze
  menuOn = true; //test of slave
  loopMode = 0; 
  espParOn = true;
  espOscOn = false;
  espWaitTime = 100;
  Serial.println();
  boardMacAddress = WiFi.macAddress();
  Serial.print("ESP Slave 1 A Board MAC Address:  ");
  Serial.println(boardMacAddress);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) 
   {
    Serial.println("Error initializing ESP-NOW !");
    espNowOK = false;;
   }
  else
   {
    Serial.println("Initializing ESP-NOW OK !");
    espNowOK = true;
   }
  
  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(masterAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() 
 {
  if (menuOn == true)
    {
     PrintMenuLocal(); 
    }

  if (Serial.available())
    {
     //digitalWrite(yellowLED, HIGH);
     markRx = Serial.read();
     if (markRx == 70) //  F - print menu off-line
      {
        menuOn = true;
        loopMode = 0; 
      }

     if (markRx == 71) //  G - test of yellow Led off-esp-line
      {
       dataAcommand = 71;
       loopMode = 7;        
      }

     if (markRx == 72) //  H - test of red Led off-esp-line
      {
       dataAcommand = 72;
       loopMode = 7;        
      }

     if (markRx == 74) //  J - test of green Led off-esp-line
      {
       dataAcommand = 74;
       loopMode = 7;        
      }
    }

 if (loopMode == 1)
  {
   digitalWrite(greenLED, LOW);        
   digitalWrite(yellowLED, LOW);        
   digitalWrite(redLED, LOW);        

  if (dataAcommand == 65)
    {
     //dataAstatus = 0;
     dataAnalog = analogRead(analogPin);
     dataA1esp = dataAnalog;   
     loopMode = 2;
    }

  if ((dataAcommand == 66) || (dataAcommand == 67) || (dataAcommand == 68))
    {
     //dataAstatus = 1; 
     if (serialPrintOn)
            {
              Serial.print("dataAcommand =");
              Serial.println(dataAcommand);
            } 
     dataAnalog = analogRead(analogPin);
     dataA1esp = dataAnalog;
     loopMode = 2;
    }

    if ( dataAcommand == 75 )            // K - slave on line with master
        {
         digitalWrite(yellowLED, HIGH);
         slaveNumberOnLine = dataA1esp;
         if (serialPrintOn)  
          {
           Serial.print("Slave number on line = ");
            Serial.println(slaveNumberOnLine);
          }
         delay(500);
         //dataAstatus = 0;
         dataA1esp = slaveNumberOnLine;   
         loopMode = 2;
         digitalWrite(yellowLED, LOW);
        }
     
    espWaitTimeStepDown = espWaitTime;
   } // end of loopMode 1
   
 if (loopMode == 2)
  {
    if (espSendOK)
     {
      if (serialPrintOn)
         {
          Serial.print("Slave Number On Line: ");
          Serial.print(slaveNumberOnLine);
          Serial.print(", Data command: ");
          Serial.print(dataAcommand);
          Serial.print(", dataA1esp: ");
          Serial.println(dataA1esp); 
          //Serial.print(", esp wait time step down: ");
          //Serial.println(espWaitTimeStepDown);
         } 
        loopMode = 3;
     }
  }  // end of loopMode 2
  
 if (loopMode == 3)
  {    
      if (serialPrintOn)
        {
         PrintSendMessage(); 
        }
      if (slaveNumber == slaveNumberOnLine) 
       {
        LoadSendMessage();
        espSendOK = false;
        espReceivedOK = false;
        // Send message via ESP-NOW
        esp_now_send(masterAddress, (uint8_t *) &espSendMessage, sizeof(espSendMessage));
        delay(1);
       }
     else
       {
        if (serialPrintOn)
         {
          Serial.print("Ihis was call for Slave ");
          Serial.println(slaveNumberOnLine);
         }
       }  
      loopMode = 0;       
  } // end of loopMode 3 

  if (loopMode == 7) 
   {
    if ( dataAcommand == 71 )            // G - yellow led on
        {
         digitalWrite(greenLED, LOW);
         digitalWrite(redLED, LOW);
         digitalWrite(yellowLED,HIGH);
         delay(2000);
         digitalWrite(yellowLED,LOW);
         loopMode = 0;
        }
    
    if ( dataAcommand == 72 )            // H - red led on
        {
         digitalWrite(greenLED, LOW);
         digitalWrite(yellowLED, LOW);
         digitalWrite(redLED, HIGH);
         delay(2000);
         digitalWrite(redLED, LOW);
         loopMode = 0;
        }

    if ( dataAcommand == 74 )            // J - green led on
        {
         digitalWrite(greenLED, HIGH);
         digitalWrite(yellowLED, LOW);
         digitalWrite(redLED, LOW);
         delay(2000);
         digitalWrite(greenLED, LOW);
         loopMode = 0;
        }


   }  // end of loopMode 7

  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
} 
