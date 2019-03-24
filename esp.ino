#include <Adafruit_Fingerprint.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include <Wire.h>  
#include "SSD1306Wire.h"
SSD1306Wire  display(0x3c, 13, 12);

char ssid[] = "anand";   //WiFi SSID             
char pass[] = "anand123"; //WiFi Password          
WiFiServer server(80);
int timezone = 5*3600; //TimeZone To +5:00
int dst = 0;
int session = 0;
int teacher = 0;

WiFiClient client;
MySQL_Connection conn((Client *)&client);
IPAddress server_addr(192,168,43,145);          // MySQL server IP
char user[] = "arduino";           // MySQL user
char password[] = "arduino";       // MySQL password

 
SoftwareSerial mySerial(4,5);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
int lcount = 0;

void setup()  
{
   display.init();
   display.flipScreenVertically();
   display.setFont(ArialMT_Plain_10);
   Serial.begin(9600); 
  // mySerial.begin(115200);
   delay(1000);  
   display.setTextAlignment(TEXT_ALIGN_LEFT);   
   display.setFont(ArialMT_Plain_10);
   display.drawString(0, 0, "Class Cube" );   
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
   display.drawString(10, 128, String(millis()));
   display.display();
   connection();
   timeconfig();
   hardwareinit(); 
}

void loop()                    
{
   
  if(lcount == 0)
  {
    scanteacher();
  }
  if(lcount == 1)
  {
    scanstud();  
  }
  if(lcount > 1)
  {  
    lcount = 0;    
  }
  delay(50);           
}














//Function To Scan Students and Marking Attendance -------------------------------------------------------------------------------------------------

int scanstud() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  if(finger.fingerID < 100)
  {
   Serial.print("Roll No: "); Serial.println(finger.fingerID); 
   char INSERT_SQL[] = "INSERT INTO s8.attendance(rollno,date,session,att,subject) VALUES (%d,NOW(),%d,1,%d)";
   char query[128];
   sprintf(query, INSERT_SQL, finger.fingerID,session,teacher);  
   display.clear();
   display.setTextAlignment(TEXT_ALIGN_LEFT);   
   display.setFont(ArialMT_Plain_10);
   display.drawString(0, 0, "Class Cube" );
   display.drawString(0, 24, "Attendance Marked" );
   display.drawString(0, 34, "Roll No:" + String(finger.fingerID));
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
   display.drawString(10, 128, String(millis()));
   display.display();
   gettime();  
   delay(500);
   MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);  
   cur_mem->execute(query);
   delete cur_mem;  
   display.clear();
   display.setTextAlignment(TEXT_ALIGN_LEFT);   
   display.setFont(ArialMT_Plain_10);
   display.drawString(0, 0, "Class Cube" );
   display.drawString(0, 24, "Waiting For  " );
   display.drawString(0, 34, "Next Person" );
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
   display.drawString(10, 128, String(millis()));
   display.display();
   gettime();

   return finger.fingerID; 
  }
  else if(finger.fingerID > 100)
  {
   lcount = 0;
   display.clear();
   display.setTextAlignment(TEXT_ALIGN_LEFT);   
   display.setFont(ArialMT_Plain_10);
   display.drawString(0, 0, "Class Cube" );
   display.drawString(0, 24, "Session "+String(session)+" Over! " );
   
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
   display.drawString(10, 128, String(millis()));
   display.display();  
   gettime();
  }
  
}

//Function To Scan Teachers and Marking Attendance -------------------------------------------------------------------------------------------------


int scanteacher() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  if(finger.fingerID > 100)
  {
    teacher = finger.fingerID;
    Serial.begin(9600);  
    WiFi.begin(ssid, pass);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);   
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Class Cube" );
    display.drawString(0, 24, "Session Start" );
    display.drawString(0, 34, "Teacher ID: " + String(teacher) );
    display.drawString(0, 44, "Session: " + String(session) );
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(10, 128, String(millis()));
    display.display();
    gettime();
    gettime();
    char UPDATE_SQL[] = "UPDATE s8.subject SET count=count+1 WHERE id = %d";
    char query[128];
    sprintf(query, UPDATE_SQL,teacher);   
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);  
    cur_mem->execute(query);
    delete cur_mem; 
    Serial.println(session);
    lcount++;    
    return finger.fingerID;    
  }
  else  
  {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);   
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Class Cube" );
    display.drawString(0, 24, "Ask Your Teacher" );
    display.drawString(0, 34, "To Start Session" );
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(10, 128, String(millis()));
    display.display();  
    gettime();
  }
}


//Function To initialize hardware -------------------------------------------------------------------------------------------------


void hardwareinit(void)
{
  finger.begin(57600);  
  if (finger.verifyPassword()) 
  {
    Serial.println("Attendance Begin");   
  } 
  else 
  {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);   
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Class Cube" );
    display.drawString(0, 24, "error 101 " );
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(10, 128, String(millis()));
    display.display();       
    gettime();
  } 
}

void connection(void)
{
  Serial.begin(9600);  
  WiFi.begin(ssid, pass);
  display.setTextAlignment(TEXT_ALIGN_LEFT);   
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Class Cube" );
  display.drawString(0, 24, "Connecting To WiFi" );
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(10, 128, String(millis()));
  display.display();
  //gettime();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);  
  }
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);   
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Class Cube" );
  display.drawString(0, 24, "Connecting To Internet " );
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(10, 128, String(millis()));
  display.display();
  //gettime();
  while (conn.connect(server_addr, 3306, user, password) != true) 
  {
    delay(200);  
  }
}  

//Function To initialize time from NTP -------------------------------------------------------------------------------------------------

void timeconfig(void)
{
  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  while(!time(nullptr))
  {     
      delay(1000);
   }
   
   display.clear();
   display.setTextAlignment(TEXT_ALIGN_LEFT);   
   display.setFont(ArialMT_Plain_10);
   display.drawString(0, 0, "Class Cube" );
   display.drawString(0, 24, "Start Attendance " );
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
   display.drawString(10, 128, String(millis()));
   display.display();    
   gettime();
}

//Function To get time from NTP -------------------------------------------------------------------------------------------------

void gettime(void)
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int hour = p_tm->tm_hour;
  int minute = p_tm->tm_min+30;
  if (minute > 59 && hour < 24)
  {
    hour = hour+1;
    minute = minute - 60;  
  }
  if (minute > 59 && hour > 23)
  {
    hour = 0;
    minute = minute - 60;  
  }
    display.setTextAlignment(TEXT_ALIGN_LEFT);   
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Class Cube" );
    display.drawString(0, 54, "Time: " + String(hour)+":" + String(minute) + ":"+String(p_tm->tm_sec) );
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(10, 128, String(millis()));
    display.display();
    Serial.print(hour);  
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(p_tm->tm_sec); 
  if(hour == 9) 
  {
    session = 1;
  }
  if(hour == 10) 
  {
    session = 2;
  }
  if(hour == 11) 
  {
    session = 3;
  }
  if(hour == 13) 
  {
    session = 4;
  }
  if(hour == 14) 
  {
    session = 5;
  }
  if(hour == 15) 
  {
    session = 6;
  }
}
