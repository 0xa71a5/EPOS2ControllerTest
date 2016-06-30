/*
Author:At1a5
Date:2016/04/16
Description:
This is a test code for Maxon`s product--EPOS2 motor controller.If you want to run it with no error,you must read over this product`s datasheet and application note.
The whole communication process is described clearly in the datasheet,which names 'EPOS2-Communication-Guide-En.pdf'.
This programe runs in Arduino Mega2560 as this type of Arduino has 4 serial communication coms.You need at least 2 serial coms to do the whole thing.
*/
//#define DEBUG
uint8_t node_id=0x01;
void debug_print(String ss = "")
{
#ifdef DEBUG
  Serial.print(ss);
#endif
}
void debug_println(String ss = "")
{
#ifdef DEBUG
  Serial.println(ss);
#endif
}
bool writeSpeed(int32_t speedNum = 0);
void initMotor();
bool sendMessageToEPOS(byte data[], int lengths);
uint16_t CRC_XModem(byte bytes[], int lengths);
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  
  Serial.println("Enter any key to continue...");
  initMotor();
  int32_t fushu=-2000;
  int32_t zhengshu=1000;
 // Serial.println(fushu,HEX);
 // writeSpeed(-1000);
  while (Serial.available() == 0);
  char tempdata = Serial.read();

}
int32_t numTest = 0;
int32_t sign=1;
void loop() {
  if (Serial.available())
  {
    numTest = 0;
    char dataTemp2=Serial.read();
    char data;
    delay(2);
   if(dataTemp2=='-')
      sign=-1;
   else
      {sign=1;data=dataTemp2;goto mark3;}
      
    while (Serial.available())
    {
    
      data = Serial.read();
      delay(2);
     mark3:;
      numTest *= 10;
      numTest += data - '0';
    }
    numTest*=sign;
  }

  Serial.print("Speed:   ");
  Serial.println(numTest);
  writeSpeed(numTest);
  delay(100);
}
void initMotor()
{
  byte dataDefalutDealyTime[]= {0x11,0x02,0x20,0x05,node_id,0x00,0x00,0x0B};
  byte dataVelocityMode[]=     {0x11,0x03,0x60,0x60,node_id,0x00,0x00,0xFE,0x00,0x00};
  byte dataMaxSpeed[]=         {0x11,0x03,0x60,0x7f,node_id,0x00,0x61,0xA8,0x00,0x00};
  byte dataStopMotor[]=        {0x11,0x03,0x60,0x40,node_id,0x00,0x00,0x06,0x00,0x00};
  byte dataStartMotor[]=       {0x11,0x03,0x60,0x40,node_id,0x00,0x00,0x0F,0x00,0x00};
  byte dataMaxAcc[]=           {0x11,0x03,0x60,0xc5,node_id,0x00,0x27,0x10,0x00,0x00};
  
  sendMessageToEPOS(dataDefalutDealyTime,sizeof(dataDefalutDealyTime));
  delay(600);
  sendMessageToEPOS(dataVelocityMode,sizeof(dataVelocityMode));
  delay(100);
  sendMessageToEPOS(dataMaxSpeed,sizeof(dataMaxSpeed));
  delay(100);
  sendMessageToEPOS(dataMaxAcc,sizeof(dataMaxAcc));
  delay(100);
  sendMessageToEPOS(dataStopMotor,sizeof(dataStopMotor));
  delay(500);
  sendMessageToEPOS(dataStartMotor,sizeof(dataStartMotor));
}

bool writeSpeed(int32_t speedNum)
{
  byte inputData[] = {0x11, 0x03, 0x20, 0x6b, node_id, 0x00, 0x00, 0x00, 0x00, 0x00};//Original data frame which used in speed control
  uint8_t highByteSpeed1 = (speedNum >> 8) & 0x000000FF;//Get the high byte from 16bit uint
  uint8_t lowByteSpeed1 =         speedNum & 0x000000FF;//Get the low byte from 16bit uint
  uint8_t highByteSpeed2=   (speedNum>>24) & 0x000000FF;
  uint8_t lowByteSpeed2=    (speedNum>>16) & 0x000000FF;
  inputData[6] = highByteSpeed1;
  inputData[7] = lowByteSpeed1;//Replace speed data area in original data frame
  inputData[8] = highByteSpeed2;
  inputData[9] = lowByteSpeed2;
 // Serial.println(highByteSpeed,HEX);
 // Serial.println(lowByteSpeed,HEX);
  debug_println("Going to sendMessage");
  sendMessageToEPOS(inputData, sizeof(inputData));
}

bool sendMessageToEPOS(byte data[], int lengths)
{
  byte opcode = data[0];
  uint16_t crcCode = CRC_XModem(data, lengths);
  byte *dataToSend = new byte [lengths + 2];
  dataToSend[0] = data[0];
  dataToSend[1] = data[1];
  for (int i = 2; i < lengths; i++)//Transmit order is different from oringial order,a word`s high byte is transmit after its low byte
  {
    if (i % 2 == 0)                //Replace the position of each high and low byte in one word.
      dataToSend[i] = data[i + 1];
    else
      dataToSend[i] = data[i - 1];
  }
  uint8_t highByteCrc = (crcCode >> 8) & 0x00FF;
  uint8_t lowByteCrc =  crcCode & 0x00FF;//Get crc code
  dataToSend[lengths] = lowByteCrc;
  dataToSend[lengths + 1] = highByteCrc;

#ifdef DEBUG
  debug_println("Origin data:");
  for (int i = 0; i < lengths; i++)
  {
    debug_println(data[i], HEX);
    debug_println(" ");
  }
  debug_println();
  debug_print("CRCCode:");
  debug_println(crcCode, HEX);
  debug_print("Transfer data order:");
  for (int i = 0; i < lengths + 2; i++)
  {
    debug_print(dataToSend[i], HEX);
    debug_print(" ");
  }
#endif
  char datatemp;
flag1:;
  Serial1.write(opcode);//Firstly,sending the opcode
  debug_println("Write opcode ,waiting for response");
  long int now = millis();
  while (Serial1.available() == 0)
  {
    if (millis() - now > 50)//Go out of the loop in case any error happens in epos
      goto flag1;
  }//wait for  response
  debug_print("Send opcode,Recv:");
  while (Serial1.available() != 0)
  {
    datatemp = Serial1.read();
    delay(1);
    //  debug_print(datatemp);//print return code
    debug_print(",");
  }
  debug_println("");
  if (datatemp == 'O') //Response correct,send other data
  {
    debug_println("Response correct,continue!\nSend data");
    for (int i = 1; i < lengths + 2; i++)
      Serial1.write(dataToSend[i]);//Sending all the data 

    debug_println("Finish sending other data now wait for response...");

    while (Serial1.available() == 0);
    //wait for another respnse
    debug_print("Receive data:");
    while (Serial1.available() != 0)//Here what we hope to get is an "O";
    {
      Serial1.read();
      //debug_print();
      debug_print(",");
      debug_print("");
      delay(1);
    }
    debug_println("Finish sending");

  }
  else
  {
    debug_println("First response is not okay");
  }


}
uint16_t CRC_XModem(byte bytes[], int lengths) {
  uint16_t crc = 0x00;          // initial value
  uint16_t polynomial = 0x1021;
  for (int index = 0 ; index < lengths; index++) {
    byte b = bytes[index];
    for (int i = 0; i < 8; i++) {
      boolean bits = ((b   >> (7 - i) & 1) == 1);
      boolean c15 = ((crc >> 15    & 1) == 1);
      crc <<= 1;
      if (c15 ^ bits) crc ^= polynomial;
    }
  }
  crc &= 0xffff;
  return crc;
}


