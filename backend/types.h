/*
 * LiveMix
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TYPES_H_
#define TYPES_H_

namespace LiveMix
{

enum ChannelType {IN, OUT, PRE, POST, SUB};
enum ElementType {GAIN, MUTE, PAN_BAL, TO_PRE, TO_POST, TO_SUB, TO_MAIN, FADER, TO_ALF, TO_PLF, PRE_VOL, MUTE_EFFECT};

#define MAIN "main"
#define MONO "mono"
#define PLF "plf/alf"

}
; // LiveMix

#endif /*TYPES_H_*/
