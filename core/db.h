/*
    Copyright 2007 Stéphane Brunner <stephane.brunner@gmail.com>
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    version 2 of the License.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef DB_H_
#define DB_H_

#include <QtCore/QString>

namespace JackMix
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
; //JackMix

#endif /*DB_H_*/
