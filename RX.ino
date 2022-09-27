#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <Servo.h>

#define Nrelay 1
#define Nmosfet 1
#define Nservo 2
#define Nchannel (Nservo + Nmosfet + Nrelay)

byte servo[Nservo] = {6,3}; // сервопривод на PWM (цифровые ноги)
byte mosfet[Nmosfet] = {5}; // мосфет на PWM (цифровые ноги)
byte relay[Nrelay] = {2};  // реле (цифровые ноги)

//RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
RF24 radio(9,53); // для Меги

byte recieved_data[Nchannel]; // массив принятых данных

Servo myservo[Nservo];

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

void setup() {
    Serial.begin(9600); //открываем порт для связи с ПК

    for (int i = 0; i < Nrelay; i++) {
        pinMode(relay[i], OUTPUT); // настроить пины реле как выход
    }
    for (int i = 0; i < Nmosfet; i++) {
        pinMode(mosfet[i], OUTPUT); // настроить пины мосфета как выход
    }
    for (int i = 0; i < Nservo; i++) {
        myservo[i].attach(servo[i]);
    }
    for (int i = 0; i < Nservo; i++) {
        delay(100);
        myservo[i].write(30); // повернуть серво на угол 30
        delay(0);
        myservo[i].write(0); // повернуть серво на угол 0
    }

    radio.begin(); //активировать модуль
    radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
    radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
    radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
    radio.setPayloadSize(32);     //размер пакета, в байтах

    radio.openReadingPipe(1, address[0]);     //хотим слушать трубу 0
    radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

    radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    //должна быть одинакова на приёмнике и передатчике!
    //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

    radio.powerUp(); //начать работу
    radio.startListening();  //начинаем слушать эфир, мы приёмный модуль
}

void loop() {
    byte pipeNo;
    while ( radio.available(&pipeNo)) {  // слушаем эфир со всех труб
        radio.read( &recieved_data, sizeof(recieved_data) );         // чиатем входящий сигнал
        for (int i = 0; i < Nrelay; i++) {
            digitalWrite(relay[i], recieved_data[i]); // подать на реле сигнал из массива
        }
        for (int i = 0; i < Nmosfet; i++) {
            analogWrite(mosfet[i], recieved_data[i + Nrelay]); // подать на мосфет ШИМ сигнал из массива, диапазон 0...255
        }
        for (int i = 0; i < Nservo; i++) {
            myservo[i].write(recieved_data[i + Nrelay + Nmosfet]); // повернуть серво на угол 0..180
        }
    }
}
