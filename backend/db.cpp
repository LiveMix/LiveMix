/*
 * Copyright 2007 Stéphane Brunner <stephane.brunner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "db.h"

#include <math.h>
#include <QtCore/QDebug>

namespace LiveMix
{

//private static float ln10_20 = log(10) / 20.0;
//private static float 20_ln10 = 20.0 / log(10);
//log(10) / 20.0;
//#define LN10_20 0.115129254649702;
//20.0 / log(10);
//#define 20_LN10 8.68588963806504;

float db2lin(float db)
{
    return pow(10, db / 20.0);
};

/*
 * This fonction will switch fron log to lin for having 0 when db == min.
 * db is in db
 * min is in db
 */
float db2lin(float db, float min)
{
    if (db >= min + 8.68588963806504) { // point to switch from log in lin lineary.
        return db2lin(db);
    } else {
        return (db - min) * db2lin(min + 8.68588963806504) / 8.68588963806504;
    }
};

float lin2db(float lin)
{
    return 20.0 * log10(lin);
};

/*
 * This fonction is the invert of db2lin(float db, float min).
 * lin is in lineary
 * min is in db
 */
float lin2db(float lin, float min)
{
    float db = lin == 0 ? min : lin2db(lin);
    if (db < min + 8.68588963806504) { // point to switch from log in lin lineary.
        db = lin * 8.68588963806504 / db2lin(min + 8.68588963806504) + min;
    }
    return db;
};

QString displayDb(float db, float min)
{
    char tmp[20];
    if (db >= min + 8.68588963806504) {
        sprintf(tmp, "%#.1f", db);
        return QString(tmp) + " db";
    } else {
        float milli = db2lin(db, min) * 1000;
        return QString("%1 milli").arg(milli, 0, 'f', 2);
    }
};

QString displayDbShort(float db, float min)
{
    char tmp[20];
    if (db >= min + 8.68588963806504) {
        sprintf(tmp, "%#.1f", db);
        return QString(tmp);
    } else {
        float milli = db2lin(db, min) * 1000;
        return QString("%1m").arg(milli, 0, 'f', 2);
    }
};

}
; //LiveMix
