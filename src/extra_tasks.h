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

#ifndef __EXTRATASKS_H
#define __EXTRATASKS_H

#include "effectmath.h"
#include "display.hpp"

// TaskScheduler
extern Scheduler ts;

//-----------------------------------------------

typedef enum _GAUGETYPE {
    GT_NONE = 0,    // пустой
    GT_VERT,        // вертикальный
    GT_HORIZ        // горизонтальный
} GAUGETYPE;

class GAUGE : public Task {
private:
    static GAUGE *gauge; // статический объект индикатора

    uint16_t xStep; uint16_t xCol; uint16_t yStep; uint16_t yCol; // для индикатора
    unsigned long gauge_time = 0;
    unsigned gauge_val = 0;
    unsigned gauge_max = 0;
    uint8_t gauge_hue = 0;
    CRGB gauge_color = 0;
    GAUGE() = delete;
public:
    GAUGE(unsigned val, unsigned max, uint8_t hue = 0)
    : Task(3*TASK_SECOND, TASK_ONCE, []() {gauge = nullptr;}, &ts, false, nullptr, nullptr, true){
        GAUGE::gauge = this;

        gauge_time = millis();
        gauge_val = val;
        gauge_max = max;
        gauge_hue = hue;

        xStep = display.getCanvas()->w() / 4;
        xCol = 4;
        if(xStep<2) {
          xStep = display.getCanvas()->w() / 3;
          xCol = 3;
        } else if(xStep<2) {
          xStep = display.getCanvas()->w() / 2;
          xCol = 2;
        } else if(xStep<2) {
          xStep = 1;
          xCol = 1;
        }

        yStep = display.getCanvas()->h() / 4;
        yCol = 4;
        if(yStep<2) {
          yStep = display.getCanvas()->h() / 3;
          yCol = 3;
        } else if(yStep<2) {
          yStep = display.getCanvas()->h() / 2;
          yCol = 2;
        } else if(yStep<2) {
          yStep = 1;
          yCol = 1;
        }

        this->enableDelayed();
    };
    ~GAUGE() {GAUGE::gauge = nullptr;}

    void GaugeMix(GAUGETYPE type = GAUGETYPE::GT_NONE) {
        if(GAUGE::gauge==nullptr) return;
        if (gauge_time + 3000 < millis() || millis()<5000) return; // в первые 5 секунд после перезагрузки не показываем :)

        if(type==GAUGETYPE::GT_VERT){
            /*
            uint8_t ind = (uint8_t)((gauge_val + 1) * display.getCanvas()->h() / (float)gauge_max + 1);
            for (uint8_t x = 0; x <= xCol * (xStep - 1); x += xStep) {
                for (uint8_t y = 0; y < display.getCanvas()->h() ; y++) {
                if (ind > y)
                    EffectMath::drawPixelXY(x, y, CHSV(gauge_hue, 255, 255));
                else
                    EffectMath::drawPixelXY(x, y,  0);
                }
            }
            */
            for (uint8_t x = 0; x <= xCol * (xStep - 1); x += xStep) {
                EffectMath::drawLine(x, 0, x, display.getCanvas()->h(), 0, display.getCanvas());   // todo:  get rid of this hack with external obj
                EffectMath::drawLineF(x, 0, x, EffectMath::fmap(gauge_val, 0, gauge_max, 0, display.getCanvas()->h()), gauge_hue ? CHSV(gauge_hue, 255, 255) : CRGB(gauge_color), display.getCanvas());
            }
        } else {
            uint8_t ind = (uint8_t)((gauge_val + 1) * display.getCanvas()->w() / (float)gauge_max + 1);
            for (uint8_t y = 0; y <= yCol * (yStep - 1) ; y += yStep) {
                for (uint8_t x = 0; x < display.getCanvas()->w() ; x++) {
                if (ind > x)
                    display.getCanvas()->at((x + y) % display.getCanvas()->w(), y) = CHSV(gauge_hue, 255, 255);
                else
                    display.getCanvas()->at((x + y) % display.getCanvas()->w(), y) = 0;
                }
            }
        }
    }

    static GAUGE *GetGaugeInstance() {
        return GAUGE::gauge;
    }

    static void GaugeShow(unsigned val, unsigned max, uint8_t hue = 0) {
        if(GAUGE::gauge==nullptr){
            GAUGE::gauge = new GAUGE(val,max,hue);
        } else {
            GetGaugeInstance()->gauge_time = millis();
            GetGaugeInstance()->gauge_val = val;
            GetGaugeInstance()->gauge_max = max;
            GetGaugeInstance()->gauge_hue = hue;
            GetGaugeInstance()->restartDelayed();
        }
    }
    void setGaugeTypeColor(CRGB color) { if(GetGaugeInstance()!=nullptr) GetGaugeInstance()->gauge_color = color;}
};

//-----------------------------------------------

class StringTask : public Task {
    char *_data = nullptr;
    INLINE char *makeCopy(const char *data) {
        if(!data)
          return nullptr;
        size_t size = strlen(data);
        char *storage = new char[size+1];
        strncpy(storage,data,size);
        if(!storage) return nullptr;
        return storage;
    }
    INLINE void setNewData(char *newData) {if(_data) delete []_data; _data=newData;}
public:
    INLINE char *getData() {return _data;}
    StringTask(const char *data = nullptr, unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL, bool sd = false)
    : Task(aInterval, aIterations, aCallback, aScheduler, aEnable, aOnEnable, aOnDisable, sd){
        _data = makeCopy(data);
    }
    ~StringTask() {if(_data) delete[] _data;}
};

//-----------------------------------------------

class WarningTask : public StringTask {
    CRGB _warn_color;
    uint32_t _warn_duration;
    uint16_t _warn_blinkHalfPeriod;
public:
    INLINE uint32_t getWarn_duration() {return _warn_duration;}
    INLINE uint32_t getWarn_blinkHalfPeriod() {return _warn_blinkHalfPeriod;}
    INLINE CRGB& getWarn_color() {return _warn_color;}
    WarningTask(CRGB &warn_color, uint32_t warn_duration, uint16_t warn_blinkHalfPeriod, const char *data = nullptr, unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL, bool sd = false)
    : StringTask(data, aInterval, aIterations, aCallback, aScheduler, aEnable, aOnEnable, aOnDisable, sd)
    , _warn_color(warn_color), _warn_duration(warn_duration), _warn_blinkHalfPeriod(warn_blinkHalfPeriod) {
      //LOG(println, F("WarningTask constructor"));
    }
};

//-----------------------------------------------

class CtrlsTask : public Task {
    DynamicJsonDocument *_data = nullptr;
    INLINE DynamicJsonDocument *makeDoc(JsonObject *data) {
        DynamicJsonDocument *storage = new DynamicJsonDocument(data->size()*2+32);
        if(!storage) return nullptr;
        String tmp; serializeJson(*data,tmp); //LOG(printf_P, PSTR("makeDoc: %s\n"), tmp.c_str());
        deserializeJson((*storage), tmp);
        return storage;
    }
    INLINE void setNewData(DynamicJsonDocument *newData) {if(_data) delete _data; _data=newData;}
public:
    INLINE JsonObject getData() {return (_data ? _data->as<JsonObject>() : JsonObject());}
    bool replaceIfSame(JsonObject *test) {
        // сравниваем предыдущий и текущий ответы на совпадение пар
        if(!test) return false;
        JsonObject obj = getData();
        bool same=!obj.isNull();
        for (JsonPair kv : obj) {
            if(!((*test)).containsKey(kv.key())){
                same=false;
                break;
            }
        }
        if(same){
            DynamicJsonDocument *doc = makeDoc(test);
            setNewData(doc);
        }
        return same;
    }
    INLINE CtrlsTask(JsonObject *data = nullptr, unsigned long aInterval=0, long aIterations=0, TaskCallback aCallback=NULL, Scheduler* aScheduler=NULL, bool aEnable=false, TaskOnEnable aOnEnable=NULL, TaskOnDisable aOnDisable=NULL)
    : Task(aInterval, aIterations, aCallback, aScheduler, aEnable, aOnEnable, aOnDisable){
        _data = makeDoc(data);
    }
    ~CtrlsTask() {if(_data) delete _data;}
};

//-----------------------------------------------

#endif