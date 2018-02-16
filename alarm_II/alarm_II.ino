#include <DS1307RTC.h>
#include <TimeLib.h>

#include <Wire.h>
#include <Keypad_I2C.h>
#include<Keypad.h>

#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>

#include <Servo.h>
#include <Password.h>

#include <SPI.h>
#include <RFID.h>

#define BACKLIGHT_PIN 3
#define dioda 2
#define buzzer 3
#define serv 4
#define ile_czujnikow 5
#define SS_PIN 10
#define RST_PIN 9

RFID rfid(SS_PIN, RST_PIN); 



LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7);
Servo myservo;

int czas=10;
int pos = 0;
int czujnik[5];
int pozycja=0;
bool zagrozenie = false;
bool alarm_on = false;
bool door_open=true;
//String password;
 


Password password = Password( "1234" );
Password password_door = Password( "1111" );

const byte ROWS = 4; 
const byte COLS = 4; 
 
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
 
byte rowPins[ROWS] = {0,1,2,3}; 
byte colPins[COLS] = {4,5,6,7};
 
int i2caddress = 0x20;
 
Keypad_I2C keypad = Keypad_I2C( makeKeymap(keys), rowPins, colPins, ROWS, COLS, i2caddress );


void start();
void check_password();
void alarm(); 
void Lock();
void unLock();  
void wlaczenie();
void zazbrajanie();


// Setup variables:
    int serNum0;
    int serNum1;
    int serNum2;
    int serNum3;
    int serNum4;

int caly;

void setup()
{
//Serial.begin(9600);
 czujnik[0]=5; //ustaw piny 
 czujnik[1]=6;
 czujnik[2]=7;
 czujnik[3]=8;
 czujnik[4]=13;



  keypad.begin();
  
  pinMode(dioda, OUTPUT);
  pinMode(buzzer, OUTPUT);
  myservo.attach(serv);
  myservo.write(pos);
  pinMode(czujnik[0], INPUT);
  pinMode(czujnik[1], INPUT);
  pinMode(czujnik[2], INPUT);
  pinMode(czujnik[3], INPUT);
  pinMode(czujnik[4], OUTPUT);//edytuj
  
  digitalWrite(czujnik[4], LOW);//potrzebny czyjnik nr 5
  
  lcd.begin(20,4);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home();
  
   SPI.begin(); 
  rfid.init();
  keypad.addEventListener(keypadEvent);
  
  

}
 void loop() 
 {
  tmElements_t tm;
  
  RTC.read(tm);
  lcd.setCursor (11, 0);
  LCDprint2digits(tm.Hour);
    lcd.print(':');
    LCDprint2digits(tm.Minute);
    lcd.print(':');
    LCDprint2digits(tm.Second);
    
  start();

  
  
  if (alarm_on != true)             // alarm wylaczony
   {
    if(door_open!=true) //jezeli drzwi byly zamkniete, to je otworz
      unLock();
     
    digitalWrite(dioda, LOW);     
    }
  else       // alarm wlaczony           
  {   
    if(door_open==true)//jezeli drzwi byly otwarte, to je zamknij
    Lock(); 
    
    digitalWrite(dioda, HIGH);             
    
    for(int i = 0; i<ile_czujnikow; i++)
      if(digitalRead(czujnik[i])==1)
        zagrozenie = true;
    
    if( zagrozenie == true )     //wlamanie !
          alarm();   
 }

 
  
  keypad.getKey();

  if (rfid.isCard()) {
        if (rfid.readCardSerial()) {
            if (rfid.serNum[0] != serNum0
                && rfid.serNum[1] != serNum1
                && rfid.serNum[2] != serNum2
                && rfid.serNum[3] != serNum3
                && rfid.serNum[4] != serNum4
            ) {
                
                serNum0 = rfid.serNum[0];
                serNum1 = rfid.serNum[1];
                serNum2 = rfid.serNum[2];
                serNum3 = rfid.serNum[3];
                serNum4 = rfid.serNum[4];
                caly = serNum0 + serNum1 + serNum2 + serNum3 + serNum4   ;
                }
             if (caly == 534 || caly== 664)
             {
               analogWrite(buzzer,50);
               delay(400);
               analogWrite(buzzer,0);
               if(alarm_on==true)
              {
                alarm_on=false; 
                zagrozenie=false;
                door_open=false;
                delay(500);
                }
                else {
                alarm_on=true;
                door_open=true;
                wlaczanie();
                delay(500);
                }
             
             lcd.clear();
             }
             
          }
    }
    
    rfid.halt();
 
  //check_password();
  //czujniki_ruchu();
}

 void start()
 {
  //lcd.clear();
  if(zagrozenie == true)
  {
    lcd.setCursor(0,0);
    lcd.print("Wlamanie!  ");
    lcd.setCursor(0,1);
    lcd.print("Podaj haslo:");
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print("Door:");
    lcd.setCursor(5,0);
    
    if(pos != 90)
    lcd.print("Open ");  
    else
    lcd.print("Close");
       
    lcd.setCursor(0,1);
   
    if( alarm_on == true )
      lcd.print("Alarm: ON");
     else 
      lcd.print("Alarm: OFF");
  }
  
  lcd.setCursor(0,2);
  lcd.print("Room:");
  lcd.setCursor(0,3);
  lcd.print("Movement:");
  
  for(int i = 0; i<ile_czujnikow ; i++)
  {
    lcd.setCursor(10+2*i,2);
    lcd.print(i+1); 
    lcd.setCursor(10+2*i,3);  
    if(digitalRead(czujnik[i])==HIGH)
    lcd.print('1');
    else
    lcd.print('0');
  }
 }

 void alarm()
 {
  digitalWrite(dioda, HIGH);
  digitalWrite(buzzer, HIGH);
  keypad.getKey();
  delay(100);
  keypad.getKey();
  digitalWrite(dioda, LOW);
  digitalWrite(buzzer, LOW);
  keypad.getKey();
  delay(100);
  keypad.getKey();
 }

  void unLock()
  {
    if ( pos !=0)
    {
      pos = 0;
      myservo.write(pos);
      door_open=true; 
     // delay(1500); 
    }
  }

  void Lock()
  {
    if(pos !=90)
    {
       pos = 90;
       myservo.write(pos);
       door_open=false; 
       //delay(1500);    
    }
  }


  void check_password()
  {
     if(password.evaluate())// jezeli haslo na wlacznie/wylaczenie alarmu sie zgadza
      { 
        if(alarm_on == false)//jezeli wylaczony, to wlacz
         {
         alarm_on = true;
         door_open=true;// zostana zamkniete  w kolejene iteracji; patrz loop
         wlaczanie();
         }
        else 
        {
          alarm_on = false;
          door_open=false; //wylaczamy alarm, w kolejnej iteracji zostana otwarte;patrz loop
        }
       lcd.clear();
       zagrozenie = false;
      }
      
      else //jezeli haslo na na wlaczenie/wylaczenie alarmu sie nie zgadza
        if(password_door.evaluate()) // to jezeli haslo na zamkniecie/otwarcie dzrwi sie zgadza
        {
         if(pos!=90 )// po pozycji serva; patrz kolejny komentarz
           {
          Lock();
          lcd.clear();
          lcd.setCursor(3,1);
          lcd.print("Drzwi zamkniete");
          delay(2000);
          lcd.clear();
          door_open=true;//rozbrojony uklad otwiera "zamkniete"drzwi; drzwi beda zamkniete przy rozbrojonym ukladzie
         } 
         else //jezeli dzrwi sa zamkniete
         {
          if(alarm_on==false)// to jezeli alarm byl wylaczony, mozna je otworzyc
          {
            unLock();
            lcd.clear();
            lcd.setCursor(4,1);
            lcd.print("Drzwi otwarte");
            delay(2000);
            lcd.clear();
            door_open=true;
          }
          else // jezeli alram byl wlaczony, to dzwi nie mozna otworzyc
          {
            lcd.clear();
            lcd.setCursor(2,1);
            lcd.print("Drzwi zamkniete!");
            lcd.setCursor(3,2);
            lcd.print("Wylacz alarm!");
            delay(2000);
            lcd.clear();
          } 
         }
        }
        else //jezeli zadne haslo sie nie zgadza
        {
         lcd.clear();
         lcd.setCursor(6,1);
         lcd.print("Zle Haslo");
         delay(1000);
         lcd.clear();
        }
     
      password.reset();//resetuj hasla
      password_door.reset();
     }



void keypadEvent(KeypadEvent eKey)
{
   switch (keypad.getState())
   {
    case PRESSED:
      if(eKey !='C' and eKey !='#' and pozycja<4)
        for(int i=0;i<=pozycja;i++)
        { 
          lcd.setCursor(14+i,1);
          lcd.print('*');
           analogWrite(buzzer,0);
           digitalWrite(dioda,LOW);
        }
      delay(500);
      // lcd.clear();
    switch (eKey)
    {
      case '#': 
        pozycja=0;
        check_password(); 
        break;
      
      case 'C': 
        password.reset(); 
        password_door.reset();
        lcd.clear();
        pozycja=0;
        break;
      
      default: 
      if(pozycja<=3)
      {
        password.append(eKey);
        password_door.append(eKey);
        pozycja++;
      }
     }
    }
}
void LCDprint2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.write('0');
  }
  lcd.print(number);  
}

void wlaczanie()
{
  delay(200);
  lcd.clear();
  
  for(int i=czas;i>0;i--)
  {
    lcd.setCursor(1,1);
    lcd.print("Wlaczanie alarmu");
    lcd.setCursor(3,2);
    lcd.print("Odliczanie:");
    lcd.print(i);
    analogWrite(buzzer,50);
    digitalWrite(dioda,HIGH);
    delay(100);
    analogWrite(buzzer,0);
    digitalWrite(dioda,LOW);
    delay(900);
    lcd.clear();
  }
  analogWrite(buzzer,50);
  delay(400);
  analogWrite(buzzer,0);
}

/*void czujniki_ruchu()    // na razie nie uzywane
{
  for(int i=0; i<2; i++) // "i" zależy od ilości aktywowanych czujników
      {
        if( digitalRead(czujnik[i]) == 1 )
           {
            Serial.print("Wykryto ruch w  ");
            Serial.print( i+1 );
            Serial.print(" pokoju.");
           }
      };
};*/

/*void check_password()
  {
     password =  Serial.readStringUntil('\n');          // <----- do zamiany na klawiature
 
    if(password == "111")   // przykładowe hasło
      { 
      if( alarm_offon == 0)
      alarm_offon = 1;
      else 
       {
        alarm_offon = 0;
       }
       password = "";
       zagrozenie = 0;
      }
};*/








