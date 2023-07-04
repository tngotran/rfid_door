/*
 * 
 * All the resources for this project: http://randomnerdtutorials.com/
 * Modified by Rui Santos
 * 
 * Created by FILIPEFLOP
 * 
 */
/*
  * Typical pin layout used: MFRC522
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15


AT24Cxx diagram
VCC -> 5V
WP (write protection )-> GND
SCL -> D19 (SCL)
SDA -> D18 (SDA)
SDA -----/\/\/\/(1k resitor brown black red)----- 5V

 */
 
#include <SPI.h>
#include <MFRC522.h>
#include <AT24Cxx.h>

#define i2c_address 0x50
 
#define SS_PIN_1 10
#define SS_PIN_2 8
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN_1, RST_PIN);   // Create MFRC522 instance.
MFRC522 mfrc522_1(SS_PIN_2, RST_PIN);   // Create MFRC522 instance.

// String ar[10] = {};
bool flag_write = false;
const int max_card =800;//save 200 card * 4 bytes for each card = 200
bool history[max_card/4] = {};
byte active_card[max_card] = {};
int address = 0;
AT24Cxx eep(i2c_address, 8);
byte master_write[4] = {67, 233, 147, 174}; //use a phone application to read UID (serial number) of a card and use online tool (https://www.rapidtables.com/convert/number/hex-to-decimal.html) to convert it to decimal
byte master_write1[4] = {67,241,02,17};
byte master_clear[4] = {179, 97, 22, 173};
byte master_clear1[4] = {51, 160, 223, 16};

const int buzzer = 15; //buzzer to arduino pin 15
const int motor = 14; //buzzer to arduino pin 15
const int motor1 = 17; //buzzer to arduino pin 15

 
void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output
  digitalWrite(buzzer, HIGH);
  pinMode(motor1, OUTPUT); 
  digitalWrite(motor1,LOW);
  pinMode(motor, OUTPUT); 
  digitalWrite(motor,LOW);
  peep(200,1000);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  mfrc522_1.PCD_Init();   // Initiate MFRC522  
  delay(500);
  clear_history();
  read_from_rom();

}
void loop(){ 
  rfid();
  rfid1();  
} 

void main_stage(byte current_UID[]){
  // Check if the card is the Master_Write
  if (compare_UID(current_UID, master_write) || compare_UID(current_UID, master_write1)){
    peep(120,1000);
      peep(120,1000);
    if (flag_write == true){
      Serial.println("Hoan thanh viec ghi the");
      read_from_rom();
      flag_write = false;
    }else{
      Serial.println("Bat dau ghi the");
      flag_write = true; 
    }
  }else if (flag_write){
    if (find_UID_rom(current_UID) != -1){
      peep(120,1000);
      peep(120,1000);
      peep(120,1000);
      peep(120,1000);
      Serial.println("The da duoc luu");
    }else{
      int add_te = find_empty_mem();
      if (add_te != -1){ // if there is an available slot in memory to write       
        peep(200,1000);
        Serial.print("Ghi the ");
        print_UID(current_UID);
        Serial.println(" vao bo nho");
        write_card(add_te, current_UID);
      }else{
        peep(500,500);        
        peep(500,500);
        peep(500,500);
        peep(500,500);
        peep(500,500);
        peep(500,500);
        peep(500,500);
      }
      // Serial.println("debug-----address:");
      // Serial.println(address);
    }    
  }else if(compare_UID(current_UID, master_clear) || compare_UID(current_UID, master_clear1)){
    // Check if the card is the master_delete
    // print_history();
    peep(120,1000);
    peep(120,1000);

    // counting how many card is untapped
    int count = 0;
    for (int i = 0; i < max_card/4; i = i + 1) {
      if (history[i] == false){
        count = count + 1 ;
      }
    }
    //Serial.println(count);
    //Serial.println(max_card/4);
    // if number of untapped cards is less than 50% of total card, then we start to delete the unused/broken card
    // This is to avoid user tap master_delete_card twice.
    if (count != max_card/4){
      tone(buzzer,31);
      Serial.println("Bat dau xoa the");
      for (int i = 0; i < max_card/4; i = i + 1) {
            if (history[i] == false){
              byte temp[4] = {NULL, NULL, NULL, NULL};        
              write_card(i*4, temp);
            }
          }
          Serial.println("hoan thanh viec xoa the khong hoat dong");
          read_from_rom();
          clear_history();
    }
  }else{
    int temp = find_UID_rom(current_UID);
    if (temp != -1){
      history[temp/4] = true;
      // Serial.println("DUOC Vao");
      peep(200,1000);

      //digitalWrite(motor, HIGH);
      //digitalWrite(motor1, LOW);
      //delay(500);
      //digitalWrite(motor, LOW);
      //delay(3000);      
      //digitalWrite(motor1, HIGH);
      //delay(500);
      //digitalWrite(motor1, LOW);


      digitalWrite(motor, HIGH);
      digitalWrite(motor1, HIGH);
      delay(10000);
      digitalWrite(motor, LOW);
      digitalWrite(motor1, LOW);

    }else{
      // Serial.println("KHONG Duoc Vao");
      peep(120,1000);
      peep(120,1000);
      peep(120,1000);
      peep(120,1000);
    }
  }
  delay(500);

}

void rfid(){
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("Ma UID:");
  // String content= " ";
  byte current_UID[mfrc522.uid.size] = {}; // mfrc522.uid.size = 4

  //uid of the current 13Mhz card is 4 byte. e.g., 43 E9 93 AE. Each byte is 8 bit.
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    //  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //  Serial.print(mfrc522.uid.uidByte[i], HEX);
    //  content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    //  content.concat(String(mfrc522.uid.uidByte[i], HEX));
    current_UID[i] = mfrc522.uid.uidByte[i];
  }

  print_UID(current_UID);
  main_stage(current_UID);
}
void rfid1(){

    // Look for new cards
  if ( ! mfrc522_1.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522_1.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("Ma UID:");
  // String content= " ";
  byte current_UID[mfrc522_1.uid.size] = {}; // mfrc522.uid.size = 4

  //uid of the current 13Mhz card is 4 byte. e.g., 43 E9 93 AE. Each byte is 8 bit.
  for (byte i = 0; i < mfrc522_1.uid.size; i++) 
  {
    //  Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //  Serial.print(mfrc522.uid.uidByte[i], HEX);
    //  content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    //  content.concat(String(mfrc522.uid.uidByte[i], HEX));
    current_UID[i] = mfrc522_1.uid.uidByte[i];
  }

  print_UID(current_UID);
  main_stage(current_UID);
}

void write_card(int add, byte in[]){
  // Serial.println("write_card()");
  eep.write(add, in[0]);	
  eep.write(add+1, in[1]);
  eep.write(add+2, in[2]);
  eep.write(add+3, in[3]);

  active_card[add]= in[0];
  active_card[add+1]= in[1];
  active_card[add+2]= in[2];
  active_card[add+3]= in[3];
  // Serial.println(add);
  add += 4;

  if (add >= eep.length()-10){
		add = 0;
	}

  // eep.write(8000, add);
}

void read_from_rom(){
  Serial.println("coppying rom to ram...");
  tone(buzzer,31);
  for (int i = 0; i < max_card; i = i + 1) {    
    active_card[i] = eep.read(i);
    
    // Serial.println(active_card[i]);
  } 
  // address = eep.read(8000);
  Serial.println("ready");


  peep(200,1000);
  peep(200,1200);
  peep(200,1500);
  // stopSound();
}



// bool find_UID(String input){
//   for (int i = 0; i < 10; i = i + 1) {
//     if (input == ar[i]){
//       return true;      
//     }
//   }
//   return false;
// }

void clear_history(){
  Serial.println("delete history");
  for (int i = 0; i < max_card/4; i = i + 1) {
    history[i] = false;
  }
}

// void print_history(){
//   Serial.println("In lich su the");
//   for (int i = 0; i < max_card/4; i = i + 1) {
//     Serial.println(history[i]);
//   }
// }

void print_UID(byte in[]){
  Serial.print(in[0],HEX);
  Serial.print(in[1],HEX);
  Serial.print(in[2],HEX);
  Serial.println(in[3],HEX);
}

int find_UID_rom(byte in[]){
  
  // Serial.println(input);
  for (int i = 0; i < max_card; i = i + 4) {
    // Serial.println(active_card[i]);
    byte temp[4] = {active_card[i], active_card[i+1], active_card[i+2], active_card[i+3]};
    if (compare_UID(in, temp)){
      Serial.println("The da luu");
      return i;      
    }    
  }
  Serial.println("The chua luu");
  return -1;
}

int find_empty_mem(){
  // Serial.println("Tim vi tri trong bo nho");
  for (int i = 0; i < max_card; i = i + 4) {
    // Serial.println(active_card[i]);   
    if (active_card[i]==NULL && active_card[i+1]==NULL && active_card[i+2]==NULL && active_card[i+3]==NULL){
      Serial.println("slot available");
      return i;
    }    
  }
  Serial.println("Bo nho day");
  return -1;
}


bool compare_UID(byte in1[], byte in2[]){  
  if (in1[0] == in2[0] && in1[1] == in2[1] && in1[2] == in2[2] && in1[3] == in2[3]){
    return true;      
  }else{
    return false;
  }
}

void peep(int howlong,int hi){
  
    tone(buzzer, hi);
    delay(howlong);// ...for 1sec
  
  noTone(buzzer);
  digitalWrite(buzzer, HIGH);
}


