#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define Nrelay 1
#define Nmosfet 1
#define Nservo 2
#define Nchannel (Nservo + Nmosfet + Nrelay)

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
//RF24 radio(9,53); // для Меги

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

byte button[Nrelay] = {3}; // кнопки для управления реле
byte slider[Nmosfet] = {1}; // потенциометры или слайдеры для управления мосфетами
byte potent[Nservo] = {0, 2}; // потенциометры для управления сервами

byte transmit_data[Nchannel]; // массив, хранящий передаваемые данные
byte latest_data[Nchannel]; // массив, хранящий последние переданные данные
boolean flag; // флажок отправки данных

void setup() {
    Serial.begin(9600); //открываем порт для связи с ПК

    for (int i = 0; i < Nrelay; i++) {
        pinMode(button[i], INPUT_PULLUP); // настроить пины кнопки
    }

    radio.begin(); //активировать модуль
    radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
    radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
    radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
    radio.setPayloadSize(32);     //размер пакета, в байтах

    radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
    radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

    radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
    radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
    //должна быть одинакова на приёмнике и передатчике!
    //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

    radio.powerUp(); //начать работу
    radio.stopListening();  //не слушаем радиоэфир, мы передатчик
}

void loop() {
    for (int i = 0; i < Nrelay; i++) {
        transmit_data[i] = !digitalRead(button[i]); // инвертированный (!) сигнал с кнопки
    }

    for (int i = 0; i < Nmosfet; i++) {
        transmit_data[i + Nrelay] = map(analogRead(slider[i]), 0, 1023, 0, 255); // сигнал с слайдера
    }

    for (int i = 0; i < Nservo; i++) {
        transmit_data[i + Nrelay + Nmosfet] = map(analogRead(potent[i]), 0, 1023, 0, 180); // сигнал с потенциометра
    }

    for (int i = 0; i < Nchannel; i++) { // в цикле от 0 до числа каналов
        if (transmit_data[i] != latest_data[i]) { // если есть изменения в transmit_data
            flag = 1; // поднять флаг отправки по радио
            latest_data[i] = transmit_data[i]; // запомнить последнее изменение
        }
    }

    if (flag == 1) {
        radio.powerUp(); // включить передатчик
        radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
        flag = 0; //опустить флаг
        radio.powerDown(); // выключить передатчик
    }

}
