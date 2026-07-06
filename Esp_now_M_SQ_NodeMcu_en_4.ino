//Esp_now_M_SQ_NodeMcu_en_4
//based on sketch Esp_now_NodeMcu_M_S1A2B3C_en_4
//more quick version direct between master and one slave
//no osc
//master on NodeMcu V0.9 esp-12E V3
//mac address CC:50:E3:11:79:4D
//with serialPrintOn for debuging
//sketch of Master station what can simply communicate 
//with one Slave stations A via ESP-NOW system
/*
  based on projects and guidance by Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp8266-nodemcu/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

String masterName = "Esp_now_M_SQ_NodeMcu_en_4";

// REPLACE WITH THE MAC Address of your receivers or sender
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // example of not used mac address
//uint8_t broadcastAddress[] = {0xEC, 0xFA, 0xBC, 0xC9, 0x65, 0x17};   // ESP-now - master V1(3) NodeMcu V0.9 ESP-12

//uint8_t slaveMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};   // example of not used mac address 
//ESP-now - slave A NodeMcu V1.0 ESP-12E mac address 84:CC:A8:97:EE:08
uint8_t slaveMacAddressA[] = {0x84, 0xCC, 0xA8, 0x97, 0xEE, 0x08};   // ESP-now - slave A NodeMcu V1.0 ESP-12E
//ESP-now - slave B Wemos D1 R1 Lolin mac address 48:55:19:17:1C:50
uint8_t slaveMacAddressB[] = {0x48, 0x55, 0x19, 0x17, 0x1C, 0x50};   // ESP-now - slave B Wemos D1 R1 ESP-12
//NodeMcu Esp12-E V3 mac addres BC:DD:C2:3D:66:9A
uint8_t slaveMacAddressC[] = {0xBC, 0xDD, 0xC2, 0x3D, 0x66, 0x9A};   // ESP-now - slave NodeMcu V1.0 ESP-12E V3

String slaveNameA = "Esp_now_SQ1A-M NodeMcu V1.0 ECG AD8232";
String slaveNameB = "Esp_now_SQ2B-M Wemos D1 R1 BPM HW-827";
String slaveNameC = "Esp_now_SQ3C_M_NodeMcu_CP_BPM HW-487";
String slaveName = "";
String slaveNameMac = "";

String slaveAddressMacA = "84:CC:A8:97:EE:08";
String slaveAddressMacB = "48:55:19:17:1C:50";
String slaveAddressMacC = "BC:DD:C2:3D:66:9A";


int slaveNumber = 1;  // A = 1, B = 2, C = 3

String boardMacAddress = "?";

byte dataAcommand;
byte dataAstatus;
unsigned int dataAindex;
unsigned int dataAnalog;
unsigned int dataA1esp;
unsigned int dataA2;
unsigned int dataA3;
unsigned int dataA4;

//Structure example to send data
//Must match in the receiver structure
typedef struct struct_message
 {  
  byte dataCommand;
  unsigned int data1esp; //data or time 
 } struct_message;

// Create a struct_message called espReadMessage to hold sensor readings
struct_message espReadMessage;

// Create a struct_message called espSendMessage to hold sensor readings
struct_message espSendMessage;

unsigned int greenLED = 15; // D8 = Green led
int yellowLED = 13; // D7 = Yellow led
int analogPin = 0; // ADC0 = A0

bool dataSendOK = false;
bool dataReceivedOK = false;
bool espnowOK = false;

unsigned int espWaitTime = 10;
int espWaitTimeStepDown = 10;


byte dataOffset = 0;
//unsigned int dataAnalogArray[401];
//unsigned int dataPocetOscMax = 401;
//unsigned int dataPocetOsc = 401;

bool dataStopOn = false;
int LoPlus = 300;           //not used in this sketch
int LoMinus = 200;


int dataPar[5];  // 0=not used, dataStep, dataPocetOsc, 3=dataTime, 4=dataOffset, 

unsigned long timeStart = 0;

byte loopMode = 0;

int I = 0;
int J = 0;

bool menuOn = false;
char markRx = 0;

byte dataStep = 20;
unsigned int dataTime = 0;

bool serialPrintOn = false;

void PrintEspSendMessage()
 {
  // Print Esp sended message in Serial Monitor
  Serial.println("ESP send data :");
  Serial.print("Command: ");
  Serial.print(dataAcommand);
  Serial.print(", A1: ");
  Serial.println(dataA1esp);
 }

void PrintEspReadMessage()
 {
  // Print Esp readed message in Serial Monitor
  Serial.println("ESP readed data:");
  Serial.print("Command: ");
  Serial.print(dataAcommand);
  Serial.print(", A1esp: ");
  Serial.println(dataA1esp);
 }

void LoadSendMessage()
 {
    espSendMessage.dataCommand = dataAcommand;
    espSendMessage.data1esp = dataA1esp; //renamed for sketch compatibility
    //espSendMessage.data2 = dataA2;
    //espSendMessage.data3 = dataA3;
    //espSendMessage.data4 = dataA4;
 }

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
 {
  if (sendStatus == 0)
   {    
    dataSendOK = true; 
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
      Serial.println(" : Delivery failed");
     }      
    Serial.print("*E,");
    Serial.print(slaveNumber);
    Serial.println(",0,#,");
    loopMode = 0; 
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
  /*
  dataA2 = espReadMessage.data2;
  dataA3 = espReadMessage.data3;
  dataA4 = espReadMessage.data4;
  */
  if (serialPrintOn)
     {
      PrintEspReadMessage();
     }

  if (dataAcommand == 65)   // A command serial
     {
      dataAstatus = 0;
      dataAindex = 1;
      loopMode = 7;
     } 

  if (dataAcommand == 66)  // B command serial in cycle
     {
      dataAstatus = 1;
      dataAindex = dataAindex + 1;
      loopMode = 7;
     } 

  if (dataAcommand == 67)   // C command  serial in cycle
     {
      dataAstatus = 1;
      dataAindex = dataAindex + 1;
      loopMode = 7;
     } 

  if (dataAcommand == 68)   // D command for direct plotter
     {
      dataAstatus = 1;
      dataAindex = dataAindex + 1;
      loopMode = 7;
     } 

   if (dataAcommand == 75)    // K set number of slave station
     {
      loopMode = 7;
     }

  dataReceivedOK = true;    
 }

    
void setup() 
 {
  // Init Serial Monitor
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  Serial.println(masterName);
  pinMode(analogPin, INPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, HIGH);     
  delay(2000);
  digitalWrite(greenLED, LOW);        
  pinMode(yellowLED, OUTPUT);
  digitalWrite(yellowLED, HIGH);     
  delay(2000);
  digitalWrite(yellowLED, LOW);        
  //pinMode(redLED, OUTPUT);
  //digitalWrite(redLED, HIGH);     
  //delay(2000);
  //digitalWrite(redLED, LOW);        
  menuOn = true;  //on-line verze
  //menuOn = false; // run version
  slaveNumber = 1;
  I = 1;
  //dataPar[0] = 0;
  dataStep = 20;
  dataPar[1] = dataStep;
  //dataPocetOsc = 401;
  dataPar[2] = 0;
  dataTime = 0;
  dataPar[3] = dataTime;
  dataOffset = 0;
  dataPar[4] = dataOffset;
  //dataPar[5] = dataPocetOscMax-1;
  espWaitTime = 100;
  dataAcommand = 0;
  dataAstatus = 0;
  //dataNindex = 0;
  serialPrintOn = true;
  loopMode = 0; // 0 = idle menu, 1+2+3 etc. = master menu,
  dataSendOK = true;
  // Set device as a Wi-Fi Station
  Serial.println();
  boardMacAddress = WiFi.macAddress();
  Serial.print("ESP Master Board MAC Address:  ");
  Serial.println(boardMacAddress);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) 
   {
    Serial.println("Error initializing ESP-NOW !");
    espnowOK = false;
    digitalWrite(greenLED, LOW);       
    //return;
   }
  else
   {
    Serial.println("Initializing ESP-NOW OK !");
    digitalWrite(greenLED, HIGH);     
    espnowOK = true;
   }
  
  if (espnowOK)
   {
    // Set ESP-NOW Role
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
  
    // Register peer
    if (slaveNumber==1)
                {
                  esp_now_add_peer(slaveMacAddressA, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
                }
                
    if (slaveNumber==2)
                {
                 esp_now_add_peer(slaveMacAddressB, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
                }
             
     if (slaveNumber==3)
                {
                 esp_now_add_peer(slaveMacAddressC, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
                }
    
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
   } 
 }

 void PrintMenuEsp()
  {
    digitalWrite(greenLED, HIGH);      
    digitalWrite(yellowLED, LOW);
    //digitalWrite(redLED, LOW);
    Serial.println(masterName);
    //ESP-NOW test
    if (!espnowOK) 
     {
      Serial.println("Error in initializing ESP-NOW !");
     }
    else
     {
      Serial.println("Initializing ESP-NOW OK !");
     }
    Serial.print("Master Staion Name: ");
    Serial.println(masterName);
    Serial.print("Master Mac Address: ");
    Serial.println(boardMacAddress);
    Serial.print("Slave Number: ");
    Serial.println(slaveNumber);
  
    if (slaveNumber==1)
     {
      slaveName = slaveNameA; // test
      slaveNameMac = slaveAddressMacA;
     }

    if (slaveNumber==2)
     {
      slaveName = slaveNameB; // test
      slaveNameMac = slaveAddressMacB;
     }

    if (slaveNumber==3)
     {
      slaveName = slaveNameC; // test
      slaveNameMac = slaveAddressMacC;
     }

    Serial.print("Slave Name: ");
    Serial.println(slaveName);
    Serial.println();
    Serial.print("Slave Name Mac Address: ");
    Serial.println(slaveNameMac);
    Serial.println();
    Serial.print("Data to/from Esp-now-two-way master station ");
    Serial.println();
    Serial.println("Menu :");
    Serial.println("I=Station info");
    Serial.println("A=data test out (*A,data,#,), B=print data for serial (*B,data,#,)CRLF");
    Serial.println("C=Data print for serial (*,data,#,)CRLF D=Data for plotter (data)CRLF");
    Serial.println("Exx=Pause(ms),  Lxxx=Nr of data to memory, Oxxx=Ofset of data,");
    Serial.println("F=Serial Print On, Q=Serial Print Off, S=Data in cycle Stop");
    Serial.println("G=Yellow LED On, J=GreenLed On test");
    //Serial.println("Kx=slave station number, 1 (A=NodeMcu V1.0), 2 (B=Wemos D1 R1), 3 (NodeMcu_CP_V3)");
    //Serial.println("M=Meas data into slave Esp memory, N=read data from slave Esp memory,");
    Serial.println("P=read set one of parameters, R=read set two parameters, Txxx=Esp wait time");
    Serial.println();
    serialPrintOn = true;
    Serial.print("SerialPrintOn = ");
    Serial.println(serialPrintOn);    
    dataSendOK = true;
    menuOn = false;
  }

 
void loop() 
 {
   if (menuOn == true)
    {
     PrintMenuEsp(); 
    }
     
  if (Serial.available())
    {
     digitalWrite(greenLED, HIGH);
     markRx = Serial.read();
     if (markRx == 65) //  A - read one data example *A,data,#,CRLF
      {
        dataAcommand = 65; 
        dataStep = dataPar[1];
        dataA1esp = dataStep;
        dataAindex = 1;
        dataPar[2] = dataAindex;
        dataTime = 0;
        dataPar[3] = 0;
        timeStart = millis();
        loopMode = 1;
      }

     if (markRx == 66)  // B - read data in cycle example *B,data,#,CRLF
      {
        dataStopOn = false;
        dataAindex = 0;
        dataAstatus = 1;
        dataStopOn = false;
        dataStep = dataPar[1];
        dataPar[2] = dataAindex;
        dataPar[3] = 0;
        dataTime = 0;
        dataOffset = dataPar[4];
        dataAcommand = 66;
        dataA1esp = dataStep;
        //dataA2 = dataAindex;
        //dataA3 = 0;
        //dataA4 = dataOffset;
        timeStart = millis();
        loopMode = 1;        
      }

     if (markRx == 67)  // C data  example *,100,#,CRLF 
      {
        //  c - Vypisuje  merena data anlog from slave
        //digitalWrite(greenLED, HIGH); 
        dataAcommand = 67;
        dataAindex = 1;
        dataStopOn = false;
        dataAstatus = 1;
        dataStep = dataPar[1];
        dataPar[2] = dataAindex;
        dataPar[3] = 0;
        dataTime = dataPar[3];
        dataOffset = dataPar[4];
        dataA1esp = dataStep;
        timeStart = millis();
        loopMode = 1;        
      }

                    
     if (markRx == 68)    // D - outgoing analog data from slave for plotter example dataCRLF
      {
        dataAcommand = 68;
        dataAindex = 1;
        dataAstatus = 1;
        dataStopOn = false;
        if (serialPrintOn)
             {
              Serial.print("D data for plotter start = ");     
              Serial.println(dataAindex);
             } 
        dataTime = 0;
        dataPar[3] = 0;
        dataA1esp = dataStep;
        timeStart = millis();
        loopMode = 1;
      }

    
    if ( markRx == 69 )   // E - Exx - xx is data step in ms
          {
            String pomString = Serial.readString();
            //pomString.trim();
            dataStep = pomString.toInt();
            dataPar[1] = dataStep;
            if (serialPrintOn)
             {
              Serial.print("Data step = ");     
              Serial.println(dataStep);
             } 
            markRx = 80; // answeres as R = parameters print out
         }

     if (markRx == 70)    // F - Serial Print On
      {
        menuOn = true;
        serialPrintOn = true;        
        Serial.println("F - Serial Print is On !");
        PrintMenuEsp(); 
        dataAcommand = 70;
        dataAstatus = 0;
        loopMode = 1;
      }     

    if (markRx == 71)    // G - set yellow Led On
      {
        if (serialPrintOn)
         {
          Serial.println("Yellow Led is light on !");
         }
        dataAcommand = 71;
        dataAstatus = 0;
        loopMode = 1;
      }     
   
    if (markRx == 72)    // H - set Red Led On
      {
        if (serialPrintOn)
         {
          Serial.println("Red Led is not installed !");
         }
        //dataAcommand = 72;
        dataAstatus = 0;
        loopMode = 0;
      }     
      
    if ( markRx == 73) // I - info
      {
        if (serialPrintOn)
         {
          Serial.print("*I,");
          Serial.print(masterName);
          Serial.print(",");
          Serial.println(slaveName);
         }
        Serial.print("*I,");
        Serial.print(slaveNumber);
        Serial.println(",#,");     
        dataAcommand = 73;
        dataAstatus = 0;
        loopMode = 0;         
      }
      
    if (markRx == 74)    // J - set green Led On
      {
        if (serialPrintOn)
         {
          Serial.println("Green led is light on !");
         }
        dataAcommand = 74;
        dataAstatus = 0;
        loopMode = 1;
      }     

    if ( markRx == 75 )   // Kx - choose of slave station : 1 = A, 2 = B"
          {
            String pomString = Serial.readString();
            slaveNumber = pomString.toInt();
            if (serialPrintOn)
             {
              Serial.print("slaveNumber = ");     
              Serial.println(slaveNumber);
             } 

            if (espnowOK)
              {
               // Set ESP-NOW Role
               esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

               // Once ESPNow is successfully Init, we will register for Send CB to
               // get the status of Trasnmitted packet
               esp_now_register_send_cb(OnDataSent);

               // Register peer
               if (slaveNumber==1)
                {
                  esp_now_add_peer(slaveMacAddressA, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
                  dataAcommand = 75;  
                  loopMode = 1;
                }
  
               if (slaveNumber==2)
                {
                 esp_now_add_peer(slaveMacAddressB, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
                 dataAcommand = 75;  
                 loopMode = 1;
                }

              if (slaveNumber==3)
                {
                 esp_now_add_peer(slaveMacAddressC, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
                 dataAcommand = 75;  
                 loopMode = 1;
                }
 
               // Register for a callback function that will be called when data is received
               esp_now_register_recv_cb(OnDataRecv);
              }
             else 
              {
                if (serialPrintOn)
                 {
                  Serial.println("Error in slave number !");
                 }
                Serial.println("*K,0,#,");
                digitalWrite(yellowLED, HIGH); 
                delay(2000);
                digitalWrite(yellowLED, LOW); 
                loopMode = 0;        
              }
            }

    if ( markRx == 76 )            // L - number of data saved to or readed from slave memory
          {
           if (serialPrintOn)
               {
                Serial.println("Data pocet Osc is not used ");      
                //Serial.println(dataPocetOsc-1);
               }
            markRx = 80; // R = parameters print out
            loopMode = 0;
          }
                         
    if ( markRx == 79 )            // O - data offset
          {
            String pomString = Serial.readString();
            dataOffset = pomString.toInt();
            if (dataOffset > 250)  // byte
             {
              if (serialPrintOn)
               {
                Serial.print("data offset max = ");
                Serial.print(250);
                Serial.println(" !");
               }
              dataOffset = 250;
             }
            dataPar[4] = dataOffset; 
            if (serialPrintOn)
               {
                Serial.print("Data offset = ");      
                Serial.println(dataOffset);
               }
            markRx = 80;
          }

    if (markRx == 80) // P = parametry
      {
        dataAcommand = 80; //  - save parameters to slave
        dataAstatus = 0;
        //dataAindex = 0;
        dataPar[2] = dataAindex;
        dataA1esp = dataPar[1];  //data Step
        dataA2 = dataPar[2];  //data index
        dataA3 = dataPar[3];  //data time
        dataA4 = dataPar[4];  //data offset
        if (serialPrintOn)
           {
            Serial.print("P Parameters: Step = ");
            Serial.print(dataA1esp);
            Serial.print(", data index = ");
            Serial.print(dataA2);
            Serial.print(", data time = ");
            Serial.print(dataA3);
            Serial.print(", Offset = ");      
            Serial.println(dataA4);
          }         
         Serial.print("*P,");
         Serial.print(dataA1esp);
         Serial.print(",");
         Serial.print(dataA2);
         Serial.print(",");
         Serial.print(dataA3);
         Serial.print(",");      
         Serial.print(dataA4);                
         Serial.print(",");   
         Serial.println("#,");       
         loopMode = 0;        
      }
      
     if (markRx == 81)  // Q = print data off
      {
        if (serialPrintOn)
         {
          Serial.println("Serial print is off !");    
         }    
        dataAcommand = 81;
        dataAstatus = 0;
        dataStep = dataPar[1];  //data Step
        dataAindex = dataPar[2];
        dataTime = dataPar[3];  //data time
        dataOffset = dataPar[4];  //data offset
        dataA1esp = dataStep;  //data Step
        dataA2 = dataAindex;  //data pocet
        dataA3 = dataTime;
        dataA4 = dataOffset;  //data offset
        /*
        Serial.print("*Q,");    // Q code is call type without serial answer for compatibility with program SerialPlotter.exe
        Serial.print(dataStep);
        Serial.print(",");
        Serial.print(dataAindex);
        Serial.print(",");
        Serial.print(dataTime);
        Serial.print(",");
        Serial.print(dataOffset);
        Serial.println(",#,");
        */
        serialPrintOn = false;
        loopMode = 1;  
      }


    if (markRx == 82) // R = read parameters
      {
        dataAcommand = 80; //  - save parameters to slave
        //dataAstatus = 0;
        dataStep = dataPar[1];  //data Step
        dataA1esp = dataStep;
        dataAindex = dataPar[2];
        dataA2 = dataPar[2];  //data Index
        dataTime = dataPar[3];
        dataA3 = dataPar[3];  //data time
        dataOffset = dataPar[4];
        dataA4 = dataPar[4];  //data offset
        if (serialPrintOn)
           {
            Serial.print("R Parameters: Step = ");
            Serial.print(dataA1esp);
            Serial.print(", data Index = ");
            Serial.print(dataA2);
            Serial.print(", dataTime = ");
            Serial.print(dataA3);
            Serial.print(", Offset = ");      
            Serial.println(dataA4);
          }          
         Serial.print("*R,");  // confirm R command and repeated last measured time
         Serial.print(dataA1esp);
         Serial.print(",");
         Serial.print(dataA2);
         Serial.print(",");
         Serial.print(dataA3);
         Serial.print(",");      
         Serial.print(dataA4);                
         Serial.print(",");   
         Serial.println("#,");                         
         loopMode = 0;        
      }

     if (markRx == 83)    // S  = stop of data transfer
      {
        dataStopOn = true;
        dataAcommand = 83;
        dataAstatus = 0;
        dataA1esp = dataStep;
        dataStopOn = true;            
        dataPar[1] = dataA1esp;
        dataA2 = dataAindex;  // Nr of all data sended
        dataPar[2] = dataA2;
        dataTime = millis() - timeStart;  // time of all data were transfered
        dataPar[3] = dataTime; 
        dataA3 = dataTime;
        dataA4 = dataPar[4];  //data offset
        if (serialPrintOn)
            {
              Serial.print("Stop => received all dataTime = ");
              Serial.println(dataTime);
            } 
        Serial.print("*S,");
        Serial.print(dataStep);
        Serial.print(",");
        Serial.print(dataAindex);
        Serial.print(",");     
        Serial.print(dataTime);
        Serial.print(",");     
        Serial.print(dataOffset);
        Serial.print(",");
        Serial.println("#,");
        dataAstatus = 0;
        loopMode = 0;  
      }

    if ( markRx == 84 )   // Txxx - espWaitTime
          {
            String pomString = Serial.readString();
            espWaitTime = pomString.toInt();
            if (serialPrintOn)
             {
              Serial.print("espWaitTime = ");     
              Serial.println(espWaitTime);
             } 
            dataAcommand = 84;
            dataAstatus = 0;
            loopMode = 1;
          }

     Serial.flush(); 
     markRx = 0;
   }  // end of Serial Available

 if (loopMode == 0)  
  {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    //digitalWrite(redLED, LOW);
  }

 if (loopMode == 1)   //loop will send data etc.
  {    
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
     //dataSendOK = true;
    if (serialPrintOn)
     {
      Serial.print("loopMode = ");
      Serial.print(loopMode);
      Serial.print(", dataAcommand = ");
      Serial.println(dataAcommand);
     } 

   if (dataAcommand == 65)    // A - slave data test send
     {
      dataAstatus = 0;
      dataAindex = 1;
      dataTime = 0;
      dataPar[3] = 0;
      dataPar[2] = dataAindex;
      dataA1esp = dataStep;  
      dataA2 = dataPar[2];  //data Index
      dataA3 = dataPar[3];  //data time
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }

   if ((dataAcommand == 66) || (dataAcommand == 67) || (dataAcommand == 68))
    // these commands send data in cycle
     {
          dataStep = dataPar[1];
          dataA1esp = dataStep;  //data step
          if (dataStep>0 )
           {
            delay(dataStep);
           }
          dataA2 = dataAindex;
          dataA3 = dataPar[3];  //data time
          dataA4 = dataPar[4];  //data offset                
     }

   if (dataAcommand == 66)    // B - master ask slave data to send in cycle
     {
      if (dataSendOK)
        {
          dataAstatus = 1;
          if (serialPrintOn)
            {
              Serial.print("B asked data in loopmode = 1 dataAindex = ");
              Serial.println(dataAindex);
            }       
          loopMode = 2;
        } 
      else 
        {
         if (serialPrintOn)
            {
              Serial.println("w,");
            }           
        } 
     }

   if (dataAcommand == 67)    // C - master ask slave to send data in cycle
     {
      if (dataSendOK)
        {
          dataAstatus = 1;
          if (serialPrintOn)
            {
              Serial.print("C asked data in loopmode = 1 dataAindex = ");
              Serial.println(dataAindex);
            } 
          loopMode = 2;
        } 
      else 
        {
         if (serialPrintOn)
            {
              Serial.print("w,");
            }           
        } 
     }

   if (dataAcommand == 68)    // D - master ask slave to send data for plotter in cycle
     {
      if (dataSendOK)
        {
          dataAstatus = 1;
          if (serialPrintOn)
            {
              Serial.print("D master asked data in loopmode = 1 dataAindex = ");
              Serial.println(dataAindex);
            } 
          loopMode = 2;
        } 
      else 
        {
         if (serialPrintOn)
            {
              Serial.println("w,");
            }           
        } 
     }

   if (dataAcommand == 70)    // F - slave data print on
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = 1;  // slave print on
      dataA2 = dataPar[2];  //data pocet
      dataA3 = dataPar[3];  //data time
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }

 if (dataAcommand == 71)    // G = Yellow led is On
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = dataPar[1];  // Yellow led is On
      dataA2 = dataPar[2];  //data pocet
      dataA3 = dataPar[3];  //data time
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }

 if (dataAcommand == 72)    // H = Red led is On
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = dataPar[1];  // Red led is On
      dataA2 = dataPar[2];  //data pocet
      dataA3 = dataPar[3];  //data time
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }
      
 if (dataAcommand == 73)    // I - station name
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = dataPar[1];  // this A slave station 
      dataA2 = dataPar[2];  //data pocet
      dataA3 = dataPar[3];  // slave number
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }

 if (dataAcommand == 74)    // J / Green  led is On
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = dataPar[1];  // Green led is On
      dataA2 = dataPar[2];  //data pocet
      dataA3 = dataPar[3];  //data time
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }

 if (dataAcommand == 75)    // K set number of slave station
     {
      dataAstatus = 0;
      dataAindex = 0;
      //dataA1esp = dataPar[1];  // dataStep
      dataA1esp = slaveNumber;  //number of slave station
      //dataA2 = dataPar[2];  //data Nr
      //dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }


  if (dataAcommand == 80)    // P - set parameters to slave
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = dataPar[1];  //data Step
      dataA2 = dataPar[2];  //data pocet
      dataA3 = dataPar[3];
      dataA4 = dataPar[4];  //data offset
      loopMode = 0;
     }


  if (dataAcommand == 81)    // Q - serialPrintOn
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = dataPar[1];  //data Step
      dataA2 = dataPar[2];  //data Index
      dataA3 = dataPar[3];  //data Time
      dataA4 = dataPar[4];  //data offset
      loopMode = 2;
     }


  if (dataAcommand == 84)    // T - espWaitTime
     {
      dataAstatus = 0;
      dataAindex = 0;
      dataA1esp = espWaitTime; //wait time when sendDataOK are false
      loopMode = 2;
     }
      
   if (dataAcommand ==  0) // greenLED off
        {
         digitalWrite(greenLED, HIGH);
         loopMode = 0;         
        }

    espWaitTimeStepDown = espWaitTime;                 
  } // end of loopMode 1

if (loopMode == 2) // data are on send ready
  {
    if (serialPrintOn)
     {
      Serial.println();
      Serial.print("loopMode = ");
      Serial.print(loopMode);
      Serial.print(" dataSendOK = ");
      Serial.println(dataSendOK);
      Serial.print("dataAcommand = ");
      Serial.print(dataAcommand);
      Serial.print(" dataAstatus = ");
      Serial.print(dataAstatus);
      Serial.print(" dataAindex = ");
      Serial.println(dataAindex);
     } 
    if (dataSendOK)
     {
      if (serialPrintOn)
         {
          Serial.print("dataAcommand: ");
          Serial.print(dataAcommand); 
          Serial.print(", dataAStatus: ");
          Serial.print(dataAstatus); 
          Serial.print(", index: ");
          Serial.print(dataAindex); 
          Serial.print(", waittime: ");
          Serial.println(espWaitTimeStepDown);
         }
        digitalWrite(greenLED, LOW);      
        digitalWrite(yellowLED, HIGH);      
        //digitalWrite(redLED,LOW);      
        loopMode = 3;
     }
    else 
     {
       digitalWrite(yellowLED,HIGH);      
       Serial.print("w,");
     } 
  }  // end of loopMode 2

 if (loopMode == 3)
   {
    if (serialPrintOn)
     {
      Serial.print("loopMode = ");
      Serial.println(loopMode);
      Serial.print("dataSendOK = ");
      Serial.println(dataSendOK);
      Serial.print("dataAcommand = ");
      Serial.println(dataAcommand);
     } 
     //digitalWrite(yellowLED, HIGH);    
     if (serialPrintOn)
       {
        PrintEspSendMessage();
       }
     LoadSendMessage();
     // Send message via ESP-NOW
     
     if (slaveNumber==1)
      {
        esp_now_send(slaveMacAddressA, (uint8_t *) &espSendMessage, sizeof(espSendMessage));
      }
      
     if (slaveNumber==2)
      {
        esp_now_send(slaveMacAddressB, (uint8_t *) &espSendMessage, sizeof(espSendMessage));
      }  
     if (slaveNumber==3)
      {
        esp_now_send(slaveMacAddressC, (uint8_t *) &espSendMessage, sizeof(espSendMessage));
      }  
    
     loopMode = 4;
     //delay(espWaitTime);
      delay(1);
      //digitalWrite(greenLED, LOW);
   } // end of loopMode 3  
   
  if (loopMode == 4)
   {
    //digitalWrite(redLED, HIGH);    
    if (dataSendOK)
      {
       digitalWrite(yellowLED,LOW);      
       //digitalWrite(redLED,LOW);      
       digitalWrite(greenLED, HIGH);    
       if (serialPrintOn)
        {
          Serial.print("dataSendOK !");
          Serial.println();
        }
       if (dataAstatus == 0)
        {  
          loopMode = 0;  //
        }
       else 
        {
         loopMode = 1;
        }   
      }
     else 
      {
       if (serialPrintOn)
        {
          Serial.println("w,");
          //Serial.println();
        }
        delay(espWaitTime);
      } 
   } // end of loopMode  4

 if (loopMode == 7)
  {
    if (serialPrintOn)
            {
              Serial.print("loopMode = ");
              Serial.print(loopMode);
              Serial.print(", dataAcommand = ");
              Serial.println(dataAcommand);
            } 

    if (dataAcommand == 65)
         {
            dataTime = millis() - timeStart;
            dataPar[3] = dataTime;
            dataPar[2] = 1;
            Serial.print("*A,");
            dataAnalog = dataA1esp + dataOffset; 
            Serial.print(dataAnalog);  // analogData
            Serial.println(",#,");
            digitalWrite(greenLED, LOW);
            digitalWrite(yellowLED, LOW);
            loopMode = 0;
         }

    if (dataAcommand == 66)
         {
           digitalWrite(greenLED, LOW);           
           digitalWrite(yellowLED,HIGH);
           if (serialPrintOn)
            {
              Serial.print("B received of dataAindex = ");
              Serial.print(dataAindex);
              //Serial.print(",");
              //Serial.print(dataA1esp);  // LoPlus
              //Serial.print(",");
              //Serial.print(dataA2);  // LoMinus
              Serial.println(",");
             }
           dataAnalog = dataA1esp + dataOffset;
           Serial.print("*B,");
           Serial.print(dataAnalog);
           Serial.println(",#,");
           dataAstatus = 1;
           loopMode = 1;
           if (dataStopOn)
            {
             dataAstatus = 0; 
             loopMode = 0;
            }
           if (serialPrintOn)
                  {
                    Serial.print("B loopMode 7 => dataAindex asked = ");
                    Serial.println(dataAindex);
                  } 
         }
      

    if (dataAcommand == 67)   // C data *,data,#,
      {
        digitalWrite(greenLED, LOW);           
        digitalWrite(yellowLED,HIGH);
        if (serialPrintOn)
          {
              Serial.print("C received dataAindex = ");
              Serial.println(dataAindex);
          } 
        dataAnalog = dataA1esp + dataOffset;
        Serial.print("*,");
        Serial.print(dataAnalog);  // analogData
        Serial.println(",#,");
        dataAstatus = 1;
        loopMode = 1;
        if (dataStopOn)
             {
              dataAstatus = 0; 
              loopMode = 0;
             }        
        if (serialPrintOn)
                  {
                    Serial.print("C loopMode 7 => dataAindex asked = ");
                    Serial.println(dataAindex);
                  }           
      }

    if (dataAcommand == 68)     // D - dataAnalog CRLF
      {
        digitalWrite(greenLED, LOW);           
        digitalWrite(yellowLED,HIGH);
        if (serialPrintOn)
          {
              Serial.print("D received dataAindex = ");
              Serial.println(dataAindex);
          } 
        dataAnalog = dataA1esp + dataOffset;
        Serial.println(dataAnalog);  // data direct on serial plotter
        dataAstatus = 1;
        loopMode = 1;
        if (dataStopOn)
             {
              dataAstatus = 0; 
              loopMode = 0;
             }
        if (serialPrintOn)
                  {
                    Serial.print("D loopMode 7 => dataAindex asked = ");
                    Serial.println(dataAindex);
                  }         
        }       

    if (dataAcommand == 75)    // K set number of slave station
       {
         dataSendOK = true; 
         Serial.print("*K,");
         slaveNumber = dataA1esp;
         Serial.print(dataA1esp);  //confirmed slave number
         Serial.println(",#,");
         loopMode= 0;
       }


        if (serialPrintOn)
                  {
                    Serial.print("loopMode 7  ");
                    Serial.print("dataSendOK = ");
                    Serial.println(dataSendOK);
                  }
                
    } //end of loopMode = 7    
    
 }
