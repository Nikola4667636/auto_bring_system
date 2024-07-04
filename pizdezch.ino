#include <EEPROM.h>         // Код написал вареный овощ, так что не судите строго)))

#include <GyverButton.h>

#include <iarduino_RTC.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define PIN_RELAY 7       // пин реле
#define PIN_BUTTON_WEEK 6       
#define PIN_BUTTON_DISPLAY 5        
#define PIN_BUTTON_FIRE 3       
#define PIN_BUTTON_TERRORIST 2

#define DELAY_TIME 5*1000       // время задержки между звонками (в миллисекундах)
#define DELAY_TIME_SOS 1*1000   // время задержки между звонками во время эвакуации

LiquidCrystal_I2C lcd(0x27,20,4); 

GButton butt6(PIN_BUTTON_WEEK);
GButton butt5(PIN_BUTTON_DISPLAY);   
GButton butt3(PIN_BUTTON_FIRE);
GButton butt2(PIN_BUTTON_TERRORIST);
        
iarduino_RTC watch(RTC_DS1307);

struct Bell{
  byte hour;
  byte minute;
  byte bellCount;
};

Bell bellCollectionWeek[] = { {.hour = 8, .minute = 0, .bellCount = 2}, {.hour = 8, .minute = 40, .bellCount = 1},       // 1 смена
                         {.hour = 8, .minute = 50, .bellCount = 2}, {.hour = 9, .minute = 30, .bellCount = 1},           // (час, минута, кол-во звонков)
                         {.hour = 9, .minute = 45, .bellCount = 2}, {.hour = 10, .minute = 25, .bellCount = 1},
                         {.hour = 10, .minute = 40, .bellCount = 2}, {.hour = 11, .minute = 20, .bellCount = 1},
                         {.hour = 11, .minute = 30, .bellCount = 2}, {.hour = 12, .minute = 10, .bellCount = 1},
                         {.hour = 12, .minute = 20, .bellCount = 2}, {.hour = 13, .minute = 0, .bellCount = 1},
                         {.hour = 13, .minute = 30, .bellCount = 2}, {.hour = 14, .minute = 10, .bellCount = 1},         // 2 смена
                         {.hour = 14, .minute = 20, .bellCount = 2}, {.hour = 15, .minute = 0, .bellCount = 1},
                         {.hour = 15, .minute = 10, .bellCount = 2}, {.hour = 15, .minute = 50, .bellCount = 1},
                         {.hour = 16, .minute = 0, .bellCount = 2}, {.hour = 16, .minute = 40, .bellCount = 1},
                         {.hour = 16, .minute = 45, .bellCount = 2}, {.hour = 17, .minute = 25, .bellCount = 1},
                         {.hour = 17, .minute = 30, .bellCount = 2}, {.hour = 18, .minute = 10, .bellCount = 1},
                         };

Bell bellCollectionWeekShort[] = { {.hour = 8, .minute = 0, .bellCount = 2}, {.hour = 8, .minute = 35, .bellCount = 1},     // 1 смена
                                   {.hour = 8, .minute = 45, .bellCount = 2}, {.hour = 9, .minute = 20, .bellCount = 1},
                                   {.hour = 9, .minute = 35, .bellCount = 2}, {.hour = 10, .minute = 5, .bellCount = 1},    // ???
                                   {.hour = 10, .minute = 20, .bellCount = 2}, {.hour = 10, .minute = 55, .bellCount = 1},
                                   {.hour = 11, .minute = 5, .bellCount = 2}, {.hour = 11, .minute = 40, .bellCount = 1},
                                   {.hour = 11, .minute = 50, .bellCount = 2}, {.hour = 12, .minute = 25, .bellCount = 1},
                                   {.hour = 14, .minute = 0, .bellCount = 2}, {.hour = 14, .minute = 35, .bellCount = 1},   // 2 смена
                                   {.hour = 14, .minute = 50, .bellCount = 2}, {.hour = 15, .minute = 25, .bellCount = 1},
                                   {.hour = 15, .minute = 35, .bellCount = 2}, {.hour = 16, .minute = 10, .bellCount = 1},
                                   {.hour = 16, .minute = 15, .bellCount = 2}, {.hour = 16, .minute = 50, .bellCount = 1},
                                   {.hour = 16, .minute = 55, .bellCount = 2}, {.hour = 17, .minute = 30, .bellCount = 1},
                                   {.hour = 17, .minute = 35, .bellCount = 2}, {.hour = 18, .minute = 5, .bellCount = 1},
                                   };

Bell bellCollectionSd[] = { {.hour = 8, .minute = 0, .bellCount = 2}, {.hour = 8, .minute = 40, .bellCount = 1},
                            {.hour = 8, .minute = 50, .bellCount = 2}, {.hour = 9, .minute = 30, .bellCount = 1},
                            {.hour = 9, .minute = 40, .bellCount = 2}, {.hour = 10, .minute = 20,  .bellCount = 1},
                            {.hour = 10, .minute = 30, .bellCount = 2}, {.hour = 11, .minute = 10, .bellCount = 1},
                            {.hour = 11, .minute = 20, .bellCount = 2}, {.hour = 12, .minute = 0, .bellCount = 1},
                            {.hour = 12, .minute = 5, .bellCount = 2}, {.hour = 12, .minute = 45, .bellCount = 1},
                          };

Bell bellCollectionSdShort[] = { {.hour = 8, .minute = 0, .bellCount = 2}, {.hour = 8, .minute = 35, .bellCount = 1},
                            {.hour = 8, .minute = 45, .bellCount = 2}, {.hour = 9, .minute = 20, .bellCount = 1},
                            {.hour = 9, .minute = 30, .bellCount = 2}, {.hour = 10, .minute = 5, .bellCount = 1},
                            {.hour = 10, .minute = 40, .bellCount = 2}, {.hour = 10, .minute = 50, .bellCount = 1},
                            {.hour = 11, .minute = 25, .bellCount = 2}, {.hour = 11, .minute = 35, .bellCount = 1},
                            {.hour = 12, .minute = 10, .bellCount = 2}, {.hour = 12, .minute = 45, .bellCount = 1},
                          };

byte mod_array[7];    // Массив недели расписаний
const String mod[5] = {"Week       ", "Week (shrt)", "Std        ", "Std (shrt) ", "No ring     "};        // Массив определения смен звонков
bool bellFlag = false;    // котроль того, что звонок звенит 1 раз в минуту
bool switchFlag = true;   // контроль того, что после нажатия кнопки расписание вернется в начальное положение через опр кол-во времени
unsigned long timer = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
unsigned int localPort = 8888; 
const char timeServer[] = "time.nist.gov"; 
const int NTP_PACKET_SIZE = 48; 
byte packetBuffer[NTP_PACKET_SIZE]; 
EthernetUDP Udp;

void setup() { 
    Ethernet.init(10);
    
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }

    // start Ethernet and UDP
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
        while (true) {
            delay(1);
        }
    }
    Udp.begin(localPort);

    sendNTPpacket(timeServer); // send an NTP packet to a time server
  
    // wait to see if a reply is available
    delay(1000);
    if (Udp.parsePacket()) {
        // We've received a packet, read the data from it
        Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        unsigned long epoch = secsSince1900 - seventyYears;
        // определяем день недели
        int weekDay = (epoch / 86400) % 7;
        if (weekDay - 3 < 0) {
            weekDay = 7 - (3 - weekDay);
        }
        else {
            weekDay -= 3;
        }
        watch.begin();
        watch.settime(epoch % 60, (epoch  % 3600) / 60, (((epoch  % 86400L) / 3600) + 3), -1, -1 , -1, weekDay);
        Ethernet.maintain();
    }
    
    pinMode(PIN_RELAY, OUTPUT);

    lcd.init();            
    lcd.backlight();

    butt6.setDebounce(50);         // настройка антидребезга        
    butt6.setType(HIGH_PULL);
    butt6.setDirection(NORM_OPEN);

    butt5.setDebounce(50);         // настройка антидребезга
    butt5.setTimeout(2000);        // настройка таймаута на удержание
    butt5.setType(HIGH_PULL);
    butt5.setDirection(NORM_OPEN);

    butt3.setDebounce(50);         // настройка антидребезга        
    butt3.setType(HIGH_PULL);
    butt3.setDirection(NORM_OPEN);

    butt2.setDebounce(50);         // настройка антидребезга      
    butt2.setType(HIGH_PULL);
    butt2.setDirection(NORM_OPEN);

    for (byte i = 0; i < 7; i++) {
        mod_array[i] = EEPROM[i];
    }
}

void delayTime() {
    uint32_t now = millis();
    while (millis () - now < DELAY_TIME) {  // Во время ожидания опрашиваем кнопку и выводим время
        showTime();
        BUTTON_WEEK();
      }
}

void ring(){
    Serial.println(watch.gettime("H:i:s - D"));
    Serial.println(mod[mod_array[indexFunction(watch.weekday)]]);
    digitalWrite(PIN_RELAY, HIGH);        // Включаем  реле
    delayTime();
    digitalWrite(PIN_RELAY, LOW);         // Отключаем реле
    delayTime();
}

void switchModule() {
    byte value = mod_array[indexFunction(watch.weekday)];
    if ((value < 4) ? (value++) : (value = 0));
    EEPROM.update(indexFunction(watch.weekday), value);
    mod_array[indexFunction(watch.weekday)] =  value;
    
}

void BUTTON_WEEK(){
  butt6.tick();
  if (butt6.isPress()){
    switchFlag = false;
    switchModule();
  }
}

void BUTTON_DISPLAY() {
  butt5.tick();
  if (butt5.isPress()){
    showInterface();
  }
}

void BUTTON_FIRE() {
  butt3.tick();
  if (butt3.isPress()){
    uint32_t now;
    while (true){
      now = millis();
      digitalWrite(PIN_RELAY, HIGH); 
      while (millis() - now < DELAY_TIME_SOS) {
        butt3.tick();
          showTime();
          BUTTON_WEEK();
          if (butt3.isPress()) {
            digitalWrite(PIN_RELAY, LOW);
            return;
          }
      }
      now = millis();
      digitalWrite(PIN_RELAY, LOW);
      while (millis () - now < DELAY_TIME_SOS) {
          butt3.tick();
          showTime();
          BUTTON_WEEK();
          if (butt3.isPress()) {
            return;
          }
      }
    }
  }
}

void BUTTON_TERRORIST() {
  butt2.tick();
  if (butt2.isPress()) {
    digitalWrite(PIN_RELAY, HIGH);
    while (true) {
      butt2.tick();
      showTime();
      BUTTON_WEEK();
      if (butt2.isPress()) {
        break;
      }
    }
    digitalWrite(PIN_RELAY, LOW);
  }
}

void showTime() {             // Вывод времени на экран
    lcd.setCursor(4, 0); lcd.print(watch.gettime("H:i:s"));    
    lcd.setCursor(0, 1); lcd.print(watch.gettime("D"));
    lcd.setCursor(5, 1); lcd.print(mod[mod_array[indexFunction(watch.weekday)]]);
}

void chooseRingCollection() { // выбор соответствующего расписания
    if (switchFlag) {
        if (indexFunction(watch.weekday) == 5) {
            EEPROM.update(indexFunction(watch.weekday), 2);
            mod_array[indexFunction(watch.weekday)] = 2;
        }
        else if (indexFunction(watch.weekday) == 6) {
            EEPROM.update(indexFunction(watch.weekday), 4);
            mod_array[indexFunction(watch.weekday)] = 4;
        }
        else {
            EEPROM.update(indexFunction(watch.weekday), 0);
            mod_array[indexFunction(watch.weekday)] = 0;
        }
    }
    switchFlag = false;
}

byte indexFunction(byte week_Day) {                 // перевод индексов дней недели с американского (вс = 0, ... сб = 6)
    if ((week_Day != 0) ? (week_Day--) : (week_Day = 6));    // на более удобный (пн = 1, вт = 2, ... вс = 6)
    return (week_Day);
}

void showInterface(){            // вывод интерфейса на экран
    lcd.clear();
    byte cursr = 1;
    byte scroll = 0;
        
    EEPROM.update(0, EEPROM[0]--);
    mod_array[0] = EEPROM[0];
        
    while(true){
        butt6.tick(); butt5.tick();
        lcd.setCursor(0, 0);
        if (scroll < 5) {
            lcd.print("Mo Tu We Th Fr");
            lcd.setCursor(0, 1);
            lcd.print(mod_array[0]); lcd.setCursor(3, 1); lcd.print(mod_array[1]); lcd.setCursor(6, 1);
            lcd.print(mod_array[2]); lcd.setCursor(9, 1); lcd.print(mod_array[3]); lcd.setCursor(12, 1);
            lcd.print(mod_array[4]);            
        }
        else {
            lcd.print("Sa Su           ");
            lcd.setCursor(0, 1); lcd.print(mod_array[5]); lcd.setCursor(3, 1); lcd.print(mod_array[6]);
            lcd.setCursor(5, 1); lcd.print("           ");
        }
        if (butt5.isPress()) {       // перенос курсора
            lcd.setCursor(cursr, 1); lcd.print(" ");
            if ((scroll < 6) ? (scroll++) : (scroll = 0));
            if (scroll == 0 or scroll == 5) {
                cursr = 1;
            }
            else if (cursr < 13) {
                cursr += 3;
            }
          }
        if (butt6.isPress()) {      // смена расписания
            if ((mod_array[scroll] < 4) ? (mod_array[scroll]++) : (mod_array[scroll] = 0));
            EEPROM.update(scroll, mod_array[scroll]);
        }
          
        lcd.setCursor(cursr, 1);
        lcd.print("<");
        lcd.setCursor(2, 1); lcd.print(" ");
        if (butt5.isHold()){   // выход из дисплея
            lcd.clear();
            delay(500);
            break;
        }
    }
}

void loop() {
    BUTTON_TERRORIST();     // опрос всех действующих кнопок
    BUTTON_FIRE();
    BUTTON_DISPLAY();
    BUTTON_WEEK();

    chooseRingCollection(); // выбор соответствующего расписания и вывод времени
    showTime();
     
    int h = atoi(watch.gettime("H")); int m = atoi(watch.gettime("i")); int s = atoi(watch.gettime("s"));
    
    if (s == 0) {
        bellFlag = false;
    }
    if (h == 23) {       // сброс расписания по умолчанию
        switchFlag = true;
    }
    if (millis() - timer > 2010){
        timer = millis();
    }
    if (mod_array[indexFunction(watch.weekday)] == 0) {
        for (int i = 0; i < sizeof(bellCollectionWeek) / sizeof(bellCollectionWeek[0]); i++) {
            Bell current = bellCollectionWeek[i];
            if (current.hour == h && current.minute == m && !bellFlag) {
                for (int j = 0; j < current.bellCount; j++) {
                    ring();
                bellFlag = true;    
                }
            }
        }         
    }
    else if (mod_array[indexFunction(watch.weekday)] == 1) {
        for (int i = 0; i < sizeof(bellCollectionWeekShort) / sizeof(bellCollectionWeekShort[0]); i++) {
            Bell current = bellCollectionWeekShort[i];
            if (current.hour == h && current.minute == m && !bellFlag) {
                for (int j = 0; j < current.bellCount; j++) {
                    ring();
                bellFlag = true;    
                }
            }
        } 
    }
    else if (mod_array[indexFunction(watch.weekday)] == 2) {
        for (int i = 0; i < sizeof(bellCollectionSd) / sizeof(bellCollectionSd[0]); i++) {
            Bell current = bellCollectionSd[i];
            if (current.hour == h && current.minute == m && !bellFlag) {
                for (int j = 0; j < current.bellCount; j++) {
                    ring();
                bellFlag = true;    
                }
            }
        } 
    }
    else if (mod_array[indexFunction(watch.weekday)] == 3) {
        for (int i = 0; i < sizeof(bellCollectionSdShort) / sizeof(bellCollectionSdShort[0]); i++) {
            Bell current = bellCollectionSdShort[i];
            if (current.hour == h && current.minute == m && !bellFlag) {
                for (int j = 0; j < current.bellCount; j++) {
                    ring();
                bellFlag = true;    
                }
            }
        }
    }
}   // end loop

void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
