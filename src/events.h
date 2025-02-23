/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#ifndef _EVENTS_H
#define _EVENTS_H

#include "ArduinoJson.h"
#include "timeProcessor.h"
#include "LList.h"

#define EVENT_TSTAMP_LENGTH 17  // для строки вида "YYYY-MM-DDThh:mm"

// закрепляем за событиями номера, чтобы они не ломались после сборки с другими дефайнами
typedef enum _EVENT_TYPE {
    ON                      = 1,
    OFF                     = 2,
    ALARM                   = 3,
    DEMO                    = 4,
    LAMP_CONFIG_LOAD        = 5,
    EFF_CONFIG_LOAD         = 6,
    EVENTS_CONFIG_LOAD      = 7,
    SEND_TEXT               = 8,
    SEND_TIME               = 9,
    PIN_STATE               = 10,
    AUX_ON                  = 11,
    AUX_OFF                 = 12,
    AUX_TOGGLE              = 13,
    SET_EFFECT              = 14,
    SET_WARNING             = 15,
    SET_GLOBAL_BRIGHT       = 16,
    SET_WHITE_HI            = 17,
    SET_WHITE_LO            = 18,
#ifdef ESP_USE_BUTTON
    BUTTONS_CONFIG_LOAD     = 19
#endif
} EVENT_TYPE;

static const char T_EVENT_DAYS[] PROGMEM = "ПНВТСРЧТПТСБВС";
class EVENT_MANAGER;
class DEV_EVENT {
    friend EVENT_MANAGER;
private:
    uint8_t repeat;
    uint8_t stopat;
    time_t unixtime;    // timestamp для события в локальной часовой зоне
    EVENT_TYPE event;
    String message;
public:
    union {
        struct {
            bool isEnabled:1;
            bool d1:1;
            bool d2:1;
            bool d3:1;
            bool d4:1;
            bool d5:1;
            bool d6:1;
            bool d7:1;
        };
        uint8_t raw_data;
    };
    const EVENT_TYPE getEvent() {return event;}
    void setEvent(EVENT_TYPE _event) {event = _event;}

    const uint8_t getRepeat() {return repeat;}
    void setRepeat(uint8_t _repeat) {repeat = _repeat;}
    
    const uint8_t getStopat() {return stopat;}
    void setStopat(uint8_t _stopat) {stopat = _stopat;}
    
    void setUnixtime(time_t _unixtime) {unixtime = _unixtime;}
    const String&getMessage() {return message;}
    void setMessage(const String& _message) {message = _message;}
    DEV_EVENT(const DEV_EVENT &event) {this->raw_data=event.raw_data; this->repeat=event.repeat; this->stopat=event.stopat; this->unixtime=event.unixtime; this->event=event.event; this->message=event.message;}
    DEV_EVENT() {this->raw_data=0; this->isEnabled=true; this->repeat=0; this->stopat=0; this->unixtime=0; this->event=_EVENT_TYPE::ON; this->message = "";}
    const bool operator==(const DEV_EVENT&event) {return (this->raw_data==event.raw_data && this->event==event.event && this->unixtime==event.unixtime);}

    String getDateTime() {
        String tmpBuf;
        TimeProcessor::getDateTimeString(tmpBuf, unixtime);
        return tmpBuf;
    }
    String getName();
};

class EVENT_MANAGER {
private:
    EVENT_MANAGER(const EVENT_MANAGER&);  // noncopyable
    EVENT_MANAGER& operator=(const EVENT_MANAGER&);  // noncopyable
    LList<DEV_EVENT *> *events = nullptr;

    void(*cb_func)(DEV_EVENT *) = nullptr; // функция обратного вызова

    void check_event(DEV_EVENT *event);
    void clear_events();

public:
    EVENT_MANAGER() { events = new LList<DEV_EVENT *>; }
    ~EVENT_MANAGER() { if(events) delete events; }

    LList<DEV_EVENT *> *getEvents() {return events;}
    
    DEV_EVENT *addEvent(const DEV_EVENT&event);
    void delEvent(const DEV_EVENT&event);
    bool isEnumerated(const DEV_EVENT&event); // проверка того что эвент в списке

    void setEventCallback(void(*func)(DEV_EVENT *)){ cb_func = func; }
    
    void events_handle();
    
    // конфиги событий
    void loadConfig(const char *cfg = nullptr);
    void saveConfig(const char *cfg = nullptr);
};

// обработка эвентов лампы
void event_worker(DEV_EVENT *);

#endif