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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef CHANNELS_H_
#define CHANNELS_H_

#include "types.h"

#ifdef LADSPA_SUPPORT
#include "ladspa_fx.h"
#endif

#include <jack/jack.h>

#include <QMap>


namespace LiveMix
{

class LadspaFXProperties;
class Backend;

class effect
{
public:
    effect(LadspaFX *p_fx, jack_nframes_t p_nframes);
    ~effect();

	QString displayname;
	
// wrorking data:
#ifdef LADSPA_SUPPORT
    LadspaFX *fx;
    LadspaFXProperties *gui;
#endif
// jack_default_audio_sample_t* eff_l;
// jack_default_audio_sample_t* eff_r;
    float m_fCpuUse; // part
    uint m_iCount; // not init => rand !
};
class channel
{
public:
    channel(QString p_name, bool p_stereo, jack_nframes_t p_nframes);
    virtual ~channel();

	virtual float getFloatAttribute(ElementType p_eType, QString p_rToChannel ="") =0;
	virtual void setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel ="") =0;
	virtual bool getBoolAttribute(ElementType p_eType, QString p_rToChannel ="") =0;
	virtual void setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel ="") =0;
	
    QString name;
    QString display_name;
    bool mute;
    bool stereo;
    QList<effect*> effects;
    QMap<QString, effect*> effectsMap;
// wrorking data:
    jack_default_audio_sample_t peak_l;
    jack_default_audio_sample_t peak_r;
};
class in : public channel
{
public:
    in(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r);
    virtual ~in();

	virtual float getFloatAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel ="");
	virtual bool getBoolAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel ="");
	
    float gain;
    float volume;
    float bal;
    bool plf;
    bool main;
    QMap<QString, float> pre;
    QMap<QString, float> post;
    QMap<QString, bool> sub;
// wrorking data:
    jack_port_t* in_l;
    jack_port_t* in_r;
    jack_default_audio_sample_t* sample_l;
    jack_default_audio_sample_t* sample_r;
    jack_default_audio_sample_t* pre_l;
    jack_default_audio_sample_t* pre_r;
    jack_default_audio_sample_t* post_l;
    jack_default_audio_sample_t* post_r;
};
class out : public channel
{
public:
    out(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r);
    virtual ~out();

	virtual float getFloatAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel ="");
	virtual bool getBoolAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel ="");
	
    float volume;
    float bal;
    bool alf;
// wrorking data:
    jack_port_t* out_l;
    jack_port_t* out_r;
    jack_default_audio_sample_t* out_s_l;
    jack_default_audio_sample_t* out_s_r;
};
class pre : public channel
{
public:
    pre(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r);
    virtual ~pre();

	virtual float getFloatAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel ="");
	virtual bool getBoolAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel ="");
	
    float volume;
    float bal;
    bool alf;
// wrorking data:
    jack_port_t* out_l;
    jack_port_t* out_r;
    jack_default_audio_sample_t* pre_l;
    jack_default_audio_sample_t* pre_r;
};
class post : public channel
{
public:
    post(QString p_name, bool p_stereo, bool p_external, jack_nframes_t p_nframes, jack_port_t* s_l, jack_port_t* s_r, jack_port_t* r_l, jack_port_t* r_r);
    virtual ~post();

	virtual float getFloatAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel ="");
	virtual bool getBoolAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel ="");
	
    float prevolume;
    float postvolume;
    float bal;
    bool plf;
    bool alf;
    bool main;
    bool external;
    QMap<QString, bool> sub;
// wrorking data:
    jack_port_t* out_l;
    jack_port_t* out_r;
    jack_port_t* in_l;
    jack_port_t* in_r;
    jack_default_audio_sample_t* post_l;
    jack_default_audio_sample_t* post_r;
    jack_default_audio_sample_t* return_sample_l;
    jack_default_audio_sample_t* return_sample_r;
    jack_default_audio_sample_t* return_l;
    jack_default_audio_sample_t* return_r;
};
class sub : public channel
{
public:
    sub(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r);
    virtual ~sub();

	virtual float getFloatAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setFloatAttribute(float p_fValue, ElementType p_eType, QString p_rToChannel ="");
	virtual bool getBoolAttribute(ElementType p_eType, QString p_rToChannel ="");
	virtual void setBoolAttribute(bool p_fValue, ElementType p_eType, QString p_rToChannel ="");
	
    float volume;
    float bal;
    bool alf;
    bool main;
    QList<effect*> effects;
// wrorking data:
    jack_port_t* out_l;
    jack_port_t* out_r;
    jack_default_audio_sample_t* sub_l;
    jack_default_audio_sample_t* sub_r;
};

}
; // LiveMix

#endif /*CHANNELS_H_*/
