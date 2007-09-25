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

#include "channels.h"
#include "backend.h"

#include <QString>
#include <QDebug>

namespace LiveMix
{

effect::effect(LadspaFX *p_fx, jack_nframes_t p_nframes)
        : fx(p_fx)
        , gui(NULL)
        , m_fCpuUse(0)
{
    switch (fx->getInputAudio()) {
	    case 2:
	        fx->m_pInBufferR = new float[p_nframes];
	    case 1:
	        fx->m_pInBufferL = new float[p_nframes];
    }
    switch (fx->getOutputAudio()) {
	    case 2:
	        fx->m_pOutBufferR = new float[p_nframes];
	    case 1:
	        fx->m_pOutBufferL = new float[p_nframes];
    }
    if (fx->getInputAudio() == 1 && fx->getOutputAudio() == 1) {
        fx->m_pInBufferR = new float[p_nframes];
        fx->m_pOutBufferR = new float[p_nframes];
    }
    for (unsigned i = 0; i < p_nframes; ++i) {
        fx->m_pOutBufferL[i] = 0;
        if (fx->m_pOutBufferR != NULL) fx->m_pOutBufferR[i] = 0;
    }

    fx->connectAudioPorts();
    fx->activate();
}
effect::~effect()
{
    fx->deactivate();
    switch (fx->getInputAudio()) {
	    case 2:
	        delete fx->m_pInBufferR;
	    case 1:
	        delete fx->m_pInBufferL;
    }
    switch (fx->getOutputAudio()) {
	    case 2:
	        delete fx->m_pOutBufferR;
	    case 1:
	        delete fx->m_pOutBufferL;
    }
    if (fx->getInputAudio() == 1 && fx->getOutputAudio() == 1) {
        delete fx->m_pInBufferR;
        delete fx->m_pOutBufferR;
    }
    delete fx;
//  if (gui != NULL) delete gui;
}
channel::channel(QString p_name, bool p_stereo, jack_nframes_t /*p_nframes*/)
{
    name = p_name;
    display_name = p_name;
    mute = false;
    stereo = p_stereo;
}
channel::~channel()
{
    while (effects.size() != 0) {
        effect* fx = effects.takeLast();
        effects.removeAll(fx);
        delete fx;
    }
}
in::in(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r) : channel(p_name, p_stereo, p_nframes)
{
    in_l = l;
    in_r = r;
    pre_l = new jack_default_audio_sample_t[p_nframes];
    pre_r = new jack_default_audio_sample_t[p_nframes];
    post_l = new jack_default_audio_sample_t[p_nframes];
    post_r = new jack_default_audio_sample_t[p_nframes];
    name = name;
    gain = 1;
    volume = 1;
    bal = 0;
    plf = false;
    main = true;
}
in::~in()
{
    delete pre_l;
    delete pre_r;
    delete post_l;
    delete post_r;
}
float in::getFloatAttribute(ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case GAIN:
			return gain;
		case PAN_BAL:
			return bal;
		case TO_PRE:
			return pre[p_rToChannel];
		case TO_POST:
			return post[p_rToChannel];
		case FADER:
			return volume;
		default:
			return -1;
	}
}
void in::setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case GAIN:
			gain = p_fValue;
			break;
		case PAN_BAL:
			bal = p_fValue;
			break;
		case TO_PRE:
			pre[p_rToChannel] = p_fValue;
			break;
		case TO_POST:
			post[p_rToChannel] = p_fValue;
			break;
		case FADER:
			volume = p_fValue;
			break;
		default:
			break;
	}
}
bool in::getBoolAttribute(ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case MUTE:
			return mute;
		case TO_SUB:
			return sub[p_rToChannel];
		case TO_MAIN:
			return main;
		case TO_PLF:
			return plf;
		default:
			return false;
	}
}
void in::setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case MUTE:
			mute = p_fValue;
			break;
		case TO_SUB:
			sub[p_rToChannel];
			break;
		case TO_MAIN:
			main = p_fValue;
			break;
		case TO_PLF:
			plf = p_fValue;
			break;
		default:
			break;
	}
}


out::out(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r) : channel(p_name, p_stereo, p_nframes)
{
    out_l = l;
    out_r = r;
    volume = 0.1;
    alf = false;
}
out::~out()
{}
float out::getFloatAttribute(ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case PAN_BAL:
			return bal;
		case FADER:
			if (p_rToChannel == MONO) {
				return Backend::instance()->getOutput(MONO)->volume;
			}
			else if (p_rToChannel == PLF) {
				return Backend::instance()->getOutput(PLF)->volume;
			}
			else {
				return volume;
			}
		default:
			return -1;
	}
}
void out::setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case PAN_BAL:
			bal = p_fValue;
			break;
		case FADER:
			if (p_rToChannel == MONO) {
				Backend::instance()->setOutVolume(MONO, p_fValue);
			}
			else if (p_rToChannel == PLF) {
				Backend::instance()->setOutVolume(PLF, p_fValue);
			}
			else {
				volume = p_fValue;
			}
			break;
		default:
			break;
	}
}
bool out::getBoolAttribute(ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case MUTE:
			return mute;
		case TO_ALF:
			return alf;
		default:
			return false;
	}
}
void out::setBoolAttribute(bool p_fValue, ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case MUTE:
			mute = p_fValue;
			break;
		case TO_ALF:
			alf = p_fValue;
			break;
		default:
			break;
	}
}


pre::pre(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r) : channel(p_name, p_stereo, p_nframes)
{
    out_l = l;
    out_r = r;
    volume = 0.1;
    bal = 0;
    alf = false;
}
pre::~pre()
{}
float pre::getFloatAttribute(ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case PAN_BAL:
			return bal;
		case FADER:
			return volume;
		default:
			return -1;
	}
}
void pre::setFloatAttribute(float p_fValue, ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case PAN_BAL:
			bal = p_fValue;
			break;
		case FADER:
			volume = p_fValue;
			break;
		default:
			break;
	}
}
bool pre::getBoolAttribute(ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case MUTE:
			return mute;
		case TO_ALF:
			return alf;
		default:
			return false;
	}
}
void pre::setBoolAttribute(bool p_fValue, ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case MUTE:
			mute = p_fValue;
			break;
		case TO_ALF:
			alf = p_fValue;
			break;
		default:
			break;
	}
}


post::post(QString p_name, bool p_stereo, bool p_external, jack_nframes_t p_nframes, jack_port_t* s_l, jack_port_t* s_r, jack_port_t* r_l, jack_port_t* r_r)
        : channel(p_name, p_stereo, p_nframes)
        , out_l(s_l)
        , out_r(s_r)
        , in_l(r_l)
        , in_r(r_r)
{
    return_l = new jack_default_audio_sample_t[p_nframes];
    return_r = new jack_default_audio_sample_t[p_nframes];
    prevolume = 1;
    postvolume = 1;
    bal = 0;
    plf = false;
    alf = false;
    main = true;
    external = p_external;
    if (!external) {
        post_l = new jack_default_audio_sample_t[p_nframes];
        post_r = new jack_default_audio_sample_t[p_nframes];
        return_sample_l = post_l;
        return_sample_r = post_r;
    }
}
post::~post()
{
    delete return_l;
    delete return_r;
    if (!external) {
        delete post_l;
        delete post_r;
    }
}
float post::getFloatAttribute(ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case PAN_BAL:
			return bal;
		case FADER:
			return postvolume;
		case PRE_VOL:
			return prevolume;
		default:
			return -1;
	}
}
void post::setFloatAttribute(float p_fValue, ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case PAN_BAL:
			bal = p_fValue;
			break;
		case FADER:
			postvolume = p_fValue;
			break;
		case PRE_VOL:
			prevolume = p_fValue;
			break;
		default:
			break;
	}
}
bool post::getBoolAttribute(ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case MUTE:
			return mute;
		case TO_SUB:
			return sub[p_rToChannel];
		case TO_MAIN:
			return main;
		case TO_ALF:
			return alf;
		case TO_PLF:
			return plf;
		default:
			return false;
	}
}
void post::setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel) {
	switch (p_eType) {
		case MUTE:
			mute = p_fValue;
			break;
		case TO_SUB:
			sub[p_rToChannel];
			break;
		case TO_MAIN:
			main = p_fValue;
			break;
		case TO_ALF:
			alf = p_fValue;
			break;
		case TO_PLF:
			plf = p_fValue;
			break;
		default:
			break;
	}
}


sub::sub(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r) : channel(p_name, p_stereo, p_nframes)
{
    out_l = l;
    out_r = r;
    volume = 0.1;
    bal = 0;
    alf = false;
    main = true;
}
sub::~sub()
{}
float sub::getFloatAttribute(ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case PAN_BAL:
			return bal;
		case FADER:
			return volume;
		default:
			return -1;
	}
}
void sub::setFloatAttribute(float p_fValue, ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case PAN_BAL:
			bal = p_fValue;
			break;
		case FADER:
			volume = p_fValue;
			break;
		default:
			break;
	}
}
bool sub::getBoolAttribute(ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case MUTE:
			return mute;
		case TO_MAIN:
			return main;
		case TO_ALF:
			return alf;
		default:
			return false;
	}
}
void sub::setBoolAttribute(bool p_fValue, ElementType p_eType, QString /*p_rToChannel*/) {
	switch (p_eType) {
		case MUTE:
			mute = p_fValue;
			break;
		case TO_MAIN:
			main = p_fValue;
			break;
		case TO_ALF:
			alf = p_fValue;
			break;
		default:
			break;
	}
}

}; // LiveMix
