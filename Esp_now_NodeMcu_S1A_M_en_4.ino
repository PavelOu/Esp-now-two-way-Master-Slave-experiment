//Esp_now_NodeMcu_S1A_M_en_4
//based on Esp_now_NodeMcu_SA_M_en_2
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
String sketchNameSlaveA = "Esp_now_NodeMcu_AD8232_S1A_M_en_4";
String sketchNameSlaveB = "Esp_now_Wemos_D1_R1_HW-827_S2B_M_en_4";
String sketchNameMaster = "Esp_now_NodeMcu_M_S1A2B_en_4";

String boardMacAddress = "?";

int slaveNumber = 1;
int slaveNumberOnLine = 0;



// REPLACE WITH THE MAC Address of your receivers or sender
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t masterAddress[] = {0xEC, 0xFA, 0xBC, 0x8F, 0x64, 0x48};   // ESP-now - master V1(3) NodeMcu V0.9 ESP-12

//NodeMcu V1.0 ESP-12E mac address
//uint8_t slaveMacAddressA[] = {0x84, 0xCC, 0xA8, 0x97, 0xEE, 0x08};   // ESP-now - slave B NodeMcu V1.0 ESP-12E

//Wemos D1 R1 Lolin mac address 48:55:19:17:1C:50
//uint8_t slaveMacAddressB[] = {0x48, 0x55, 0x19, 0x17, 0x1C, 0x50};   // ESP-now - slave B Wemos D1 R1 ESP-12

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
  byte data1;
  unsigned int data2;
  unsigned int data3;
  byte data4;
 } struct_message;

// Create a struct_message called espReadMessage to hold sensor readings
struct_message espReadMessage;

// Create a struct_message called espSendMessage to hold sensor readings
struct_message espSendMessage;

byte dataAcommand;
byte dataAstatus;
unsigned int dataAindex;
byte dataA1;
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

unsigned int dataOsc;
unsigned int dataOscOffset = 0;
unsigned int dataPar[6]; 
// 0=not used, 1=dataStep, 2=dataPocetOsc, 3=dataTime, 4=dataOscOffset
// 5=dataPocetOscMax

unsigned int dataOscArray[401];
unsigned int dataPocetOscMax = 401;
unsigned int dataPocetOsc = 401;
bool dataStopOn = false;
unsigned long timeStart = 0;

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
      Serial.print("Last Packet Send Status: ");
      Serial.println("Delivery success");    
     }
   }     
  else
   {
    if (serialPrintOn)
     {
      Serial.print("Last Packet Send Status: ");
      Serial.println("Delivery fail");
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
   dataA1 = espReadMessage.data1;
   dataA2 = espReadMessage.data2;
   dataA3 = espReadMessage.data3;
   dataA4 = espReadMessage.data4;
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
       dataAindex = dataA2;
       if (serialPrintOn)
            {
              Serial.print("B(C,D) received dataAindex = ");
              Serial.println(dataAindex);
            } 
       dataAstatus = 0;
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


   if (dataAcommand == 77)    // M - read analog data and parse them to memory
      {   
       loopMode = 1;    
      }  

  
   if (dataAcommand == 78)   // N - prepares reading data from memory
      {                     
            dataAstatus = 1;
            dataAindex = 0;
            loopMode = 1;                       
      }
  

   if (dataAcommand == 80)    // Save parameters
     {      
      dataPar[1] = dataA1;
      dataStep = dataA1;
      dataPar[2] = dataA2;
      dataPocetOsc = dataA2+1;
      dataPar[3] = dataA3;
      dataTime = dataA3;
      dataPar[4] = dataA4;
      dataOscOffset = dataA4;
      dataAstatus = 0;
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

    if ( dataAcommand == 83 )            // S - data stop on
          {
            digitalWrite(greenLED, LOW);
            digitalWrite(yellowLED, HIGH);
            digitalWrite(redLED, LOW);
            dataStopOn = true;
            loopMode = 4;
          }

    if (dataAcommand == 84)    // T - espWaitTime
     {      
      espWaitTime = dataA4;
      digitalWrite(yellowLED, HIGH);
      delay(500);
      digitalWrite(yellowLED, LOW);
      loopMode = 0;
     }

    if (dataAcommand == 85)   // N - vypis dat postupne z pameti a ESP-now transfer
      {                     
            dataAindex = dataA2;
            dataAstatus = 0;
            loopMode = 1;                       
      }


   digitalWrite(yellowLED, LOW);
   espReceivedOK = true;
}
   
void LoadSendMessage()
 {
    espSendMessage.dataCommand = dataAcommand;
    espSendMessage.data1 = dataA1;
    espSendMessage.data2 = dataA2;
    espSendMessage.data3 = dataA3;
    espSendMessage.data4 = dataA4;
 }

void PrintReadMessage()
 {
  // Display Readings in Serial Monitor
  Serial.println("Incomming READINGS");
  Serial.print("Command: ");
  Serial.print(dataAcommand);
  Serial.print(", Status: ");
  Serial.print(dataAstatus);
  Serial.print(", index: ");
  Serial.print(dataAindex);
  Serial.print(", A1: ");
  Serial.print(dataA1);
  Serial.print(", A2: ");
  Serial.print(dataA2);
  Serial.print(", A3: ");
  Serial.print(dataA3);  
  Serial.print(", A4: ");
  Serial.println(dataA4);  
}

void PrintSendMessage()
 {
  // Display Readings in Serial Monitor
  Serial.println("Outgoing message:");
  Serial.print("Command: ");
  Serial.print(dataAcommand);
  Serial.print(", Status: ");
  Serial.print(dataAstatus);
  Serial.print(", Index: ");
  Serial.print(dataAindex);
  Serial.print(", A1: ");
  Serial.print(dataA1);
  Serial.print(", A2: ");
  Serial.print(dataA2);
  Serial.print(", A3: ");
  Serial.print(dataA3);  
  Serial.print(", A4: ");
  Serial.println(dataA4);  
}

void PrintMenuLocal()
  {
    digitalWrite(greenLED, HIGH);      
    digitalWrite(yellowLED, LOW);
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
    Serial.println("Exx=Pause(ms), Lxxx=Nr of data, Oxxx=Ofset of data,");
    Serial.println("F=send details, R,R=Read parameters, Q=stop send details, S=Stop data cycle");
    Serial.println("G=Yellow, H=Red, J=Green Led On");
    Serial.println("M=Meas data and save them to memory, N=Read saved data from memory");
    Serial.print("Max number of data saved to memory: ");
    Serial.println(dataPocetOscMax-1);
    Serial.println();
    Serial.println("Print data is On");
    Serial.println();
    espSendOK = true;
    espReceivedOK = true;      
    loopMode = 0;   
    serialPrintOn = true;
    menuOn = false;
    delay(500);
    digitalWrite(greenLED, LOW);      
  }

 
void setup() 
{
  // Init Serial Monitor
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  sketchNameSlave = sketchNameSlaveB;
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
  dataPar[0] = 0;
  dataStep = 20;
  dataPar[1] = dataStep;
  dataPocetOsc = 401;
  dataPar[2] = dataPocetOsc-1;
  dataTime = 0;
  dataPar[3] = dataTime;
  dataOscOffset = 0;
  dataPar[4] = dataOscOffset;
  dataPar[5] = dataPocetOscMax-1;
  serialPrintOn = true;
  //menuOn = false;  //run verze
  menuOn = true; //test of slave
  loopMode = 0; 
  espParOn = true;
  espOscOn = false;
  espWaitTime = 100;
  Serial.println();
  boardMacAddress = WiFi.macAddress();
  Serial.print("ESP Board MAC Address:  ");
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

 if (loopMode == 1)
  {
   digitalWrite(greenLED, LOW);        
   digitalWrite(yellowLED, LOW);        
   digitalWrite(redLED, LOW);        
   dataPocetOsc = dataPar[2] + 1;

  if (dataAcommand == 65)
    {
     dataAstatus = 0;
     dataA1 = dataPar[1];
     dataA2 = dataPar[2];
     dataOsc = analogRead(analogPin)+dataOscOffset;
     dataA3 = dataOsc;   
     dataOscOffset = dataPar[4];
     loopMode = 2;
    }

  if ((dataAcommand == 66) || (dataAcommand == 67) || (dataAcommand == 68))
    {
     dataAstatus = 0; 
     if (serialPrintOn)
            {
              Serial.print(dataAcommand);
              Serial.print(" sended dataAindex =");
              Serial.println(dataAindex);
            } 
     dataA1 = 0;
     dataA2 = 0;
     dataOscOffset = dataPar[4];
     dataOsc = analogRead(analogPin)+dataOscOffset;
     dataA3 = dataOsc;
     loopMode = 2;
    }

    if ( dataAcommand == 75 )            // K - slave number on line with master
        {
         digitalWrite(yellowLED, HIGH);
         slaveNumberOnLine = dataA3;
         if (serialPrintOn)  
          {
           Serial.print("Slave number on line = ");
            Serial.println(slaveNumberOnLine);
         }
         delay(500);
         dataAstatus = 0;
         dataA3 = slaveNumberOnLine;   
         loopMode = 2;
         digitalWrite(yellowLED, LOW);
        }

   if ( dataAcommand == 77 )            // M - Zaznam  do pameti
          {
            I = 1;
            dataStep = dataPar[1];
            dataPocetOsc = dataPar[2] + 1;
            dataOscOffset = dataPar[4];
            timeStart = millis();
        OpakMesp:
            dataOsc = analogRead(analogPin);
            dataOscArray[I] = dataOsc + dataOscOffset;
            if (serialPrintOn)
             {
               Serial.print("Index = ");
               Serial.print(I);
               Serial.print(" data = ");
               Serial.println(dataOscArray[I]);
             }
            if (dataStep>0) {delay(dataStep);};
            I = I + 1;
            if (I < dataPocetOsc)
              {
                goto OpakMesp;
              }
            dataTime = millis() - timeStart;
            dataOscArray[0] = dataTime;      
            dataA1 = dataStep;
            dataA2 = dataPocetOsc - 1;
            dataA3 = dataTime;
            loopMode = 2;
          }
    
  if (dataAcommand == 78)
    {
     dataAstatus = 1; 
     dataAindex = 1;
     dataA1 = dataPar[1];
     dataA2 = dataPar[2];
     dataA3 = dataOscArray[0];
     dataA4 = dataPar[4];
     loopMode = 2;
    }
  

   if  ((dataAcommand == 80) || (dataAcommand == 82))   // Parametry
      {      
       dataAstatus = 0;
       //dataAindex = 0;
       //dataA1 = dataPar[1];
       dataA1 = dataPar[1];
       dataA2 = dataPar[2];
       dataA3 = dataPar[3];
       dataA4 = dataPar[4];
       loopMode = 2;
     }

      if (dataAcommand == 83)    // S - stop of data transfer
      {      
       dataAstatus = 0;
       digitalWrite(greenLED, LOW);
       digitalWrite(yellowLED, HIGH);
       digitalWrite(redLED, LOW);
       //delay(250);
       digitalWrite(yellowLED,LOW);
       dataA1 = dataPar[1];
       dataA2 = dataPar[2];
       dataA3 = dataPar[3];
       dataA4 = dataPar[4];
       loopMode = 2;
     }
  
   if (dataAcommand == 85)
    {
      dataA2 = dataAindex;
      dataA3 = dataOscArray[dataAindex] ;
      loopMode = 2;  
    }  // end of 85
   

    espWaitTimeStepDown = espWaitTime;
    digitalWrite(yellowLED, LOW);        
   } // end of loopMode 1
   
 if (loopMode == 2)
  {
    if (espSendOK)
     {
      if (serialPrintOn)
         {
          Serial.print("Data command: ");
          Serial.print(dataAcommand);
          Serial.print(", index: ");
          Serial.print(dataAindex); 
          Serial.print(", esp wait time step down: ");
          Serial.println(espWaitTimeStepDown);
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
        dataAstatus = 0;
       } 
      if (dataAstatus == 0)
       {
        loopMode = 0;
       }
      else 
       {
        loopMode = 4;       
       }  
       
  } // end of loopMode 3 
    
 if (loopMode == 4)
   {
    if (espSendOK)
      {
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED,LOW);     
        if (serialPrintOn)  
          {
           Serial.println("loopMode 4 Esp send OK");
          }
        if (dataAcommand == 83)
        {
         dataAstatus = 0;
         dataStopOn = true;
         loopMode = 1;
        }
      }    
    else 
      {
         if (serialPrintOn)  
          {
           Serial.println("wait for espSendOK");
          }
         delay(espWaitTime);
      } 
       
   }  // end of loopMode = 4
 

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
