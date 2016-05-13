/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is derived from deviationTx project for Arduino.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 see <http://www.gnu.org/licenses/>
*/

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <SPI.h>

#include "common.h"
#include "utils.h"
#include "RCRcvrERSkySerial.h"

typedef enum
{
    STATE_IDLE,
    STATE_BODY,
} STATE_T;

u8 RCRcvrERSkySerial::getChCnt(void)
{
    return CH_CNT;
}

void RCRcvrERSkySerial::init(void)
{
    mState = STATE_IDLE;
}

void RCRcvrERSkySerial::close(void)
{

}

u32 RCRcvrERSkySerial::loop(void)
{
    u32 ret = 0;
    u8  rxSize = SerialProtocol::available();

    if (rxSize == 0)
        return ret;

    while (rxSize--) {
        u8 ch = SerialProtocol::read();

        switch (mState) {
            case STATE_IDLE:
                if (ch == 0x55) {
                    mState = STATE_BODY;
                    mOffset = 0;
                    mDataSize = 25;
                }
                break;

            case STATE_BODY:
                if (mOffset < mDataSize) {
                    mRxPacket[mOffset++] = ch;
                } else {
                    ret = handlePacket(mRxPacket, mDataSize);
                    mState = STATE_IDLE;
                    rxSize = 0;             // no more than one command per cycle
                }
                break;
        }
    }

    return ret;
}

u32 RCRcvrERSkySerial::handlePacket(u8 *data, u8 size)
{
    u32 ret = 0;

    if (data[0] & 0x20) {       // check range

    } else {

    }

    if (data[0] & 0xc0) {       // check autobind(0x40) & bind(0x80) together

    } else {

    }

    if (data[1] & 0x80) {       // low power

    } else {

    }

    u8 proto  = data[0] & 0x1f;          // 5 bit
    u8 sub    = (data[1] >> 4) & 0x07;   // 3 bit
    u8 rxnum  = data[1] & 0x0f;          // 4 bit
    u8 option = data[2];

    u8 *p  = &data[2];
    u8 dec = -3;

    // 11 bit * 16 channel
    for (u8 i = 0; i < 8; i++) {
        dec += 3;
        if (dec >= 8) {
            dec -= 8;
            p++;
        }
        p++;

        u32 val = *(u32*)p;
        val = ((val >> dec) & 0x7ff);
        sRC[i] =  map(val, 204, 1844, CHAN_MIN_VALUE, CHAN_MAX_VALUE);
    }

    if (proto != mProto) {
        mProto = proto;
        ret |= (u32)proto << 16;
    }

    if (sub != mSubProto) {
        mSubProto = sub;
        ret |= (u32)proto << 8;
        ret |= option;
    }

    return ret;
}