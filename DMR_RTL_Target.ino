/*
example given:
68 07 01 01 00 00 00 0A 09 00 00 02 31 00 32 00 >>>0A<<< 00 10  [0A means keep on sending for 10 seconds]
*/
const int led = 13;
const int BUFF_SIZE = 300;
unsigned char rec_buff[BUFF_SIZE];
int buff_len = 0;
int last_buff_len=0;

const unsigned char TX_Cmd[13]={0x68, 06, 01, 01, 0x84, 0xF3, 00, 04, 02, 00, 00, 01, 0x10};
const unsigned char RX_Cmd[13]={0x68, 06, 01,0xff, 0x83, 0xF5, 00, 04, 02, 00, 00, 01, 0x10};
const unsigned char Read_SM[10]={0x68, 0x11, 01, 01, 0x95, 0xDC, 00, 01, 01, 0x10};

#define TX true
#define RX (!TX)
bool TX_RX = RX;

void setup() {
  // initialize serial:
  Serial.begin(57600);
  pinMode(led, OUTPUT);
  digitalWrite(led,TX_RX);
  memset(rec_buff, 0, BUFF_SIZE);
}

void clean_buff(void)
{
  memset(rec_buff, 0, BUFF_SIZE);
  buff_len=0;
  last_buff_len=0;
}

bool pattern_match(const unsigned char* match,int pattern_len)
{
  int corr_count = 0;
  if (last_buff_len == buff_len && buff_len >= pattern_len) {  //we have a packet in.
    for (int i = last_buff_len - pattern_len; i<=last_buff_len; i++) {
      if(rec_buff[i] == match[i - (last_buff_len - pattern_len)]){
        corr_count++;
      }
      else{
        break;
      }
    }
    if(corr_count == pattern_len){        
      return true;
    }
  }
  else{
      last_buff_len = buff_len;
  }
  return false;
}

void loop()
{
  static bool wait_sms=false;
  digitalWrite(led, 1-digitalRead(led));
  delay(200);
  const unsigned char match[] = { 0x68, 07, 02, 0x70, 0x85, 0x88 ,00 ,00, 0x10  };
  const int pattern_len = 9;
  if(wait_sms==false){
    if(pattern_match(match,sizeof(match)/sizeof(char))){   //if we have a short message
      clean_buff();
      for(int i=0;i<10;i++){
        Serial.write(Read_SM[i]);
      }
      wait_sms=true;
    }
  }
  else if(buff_len>=15){
    //68 11 00 00 54 84 00 06 00 00 01 31 32 33 10 
    unsigned short delay_us = rec_buff[13]*1000;
    digitalWrite(led, HIGH);
    for(int i=0;i<13;i++)
      Serial.write(TX_Cmd[i]);
    delay(delay_us);
    for(int i=0;i<13;i++)
      Serial.write(RX_Cmd[i]);
    digitalWrite(led, LOW);
    clean_buff();
    wait_sms=false;
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    unsigned char inChar = Serial.read();
    rec_buff[buff_len++] = inChar;
  }
}

