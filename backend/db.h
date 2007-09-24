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
#ifndef DB_H_
#define DB_H_

#include <QtCore/QString>

namespace LiveMix
{

float db2lin(float db);

/*
 * This fonction will switch fron log to lin for having 0 when db == min.
 * db is in db
 * min is in db
 */
float db2lin(float db, float min);

float lin2db(float lin);

/*
 * This fonction is the invert of db2lin(float db, float min).
 * lin is in lineary
 * min is in db
 */
float lin2db(float lin, float min);

QString displayDb(float db, float min);

QString displayDbShort(float db, float min);

}
; //LiveMix

#endif /*DB_H_*/
