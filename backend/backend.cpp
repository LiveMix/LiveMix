/*
 * Copyright 2004 - 2006 Arnold Krille <arnold@arnoldarts.de>
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

#include "backend.h"

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>

#include <sys/time.h>
//#include <jack/midiport.h>


namespace LiveMix
{

float m_fProcessTime = 0.0f;   /// time used in process function
float m_fMaxProcessTime = 0.0f;   /// max ms usable in process with no xrun
Backend* m_pInstance;

Backend::Backend(GuiServer_Interface* g) :  gui(g)
  , _run(false)
  , seq(0)
{
    qDebug() << "JackBackend::JackBackend()";
    client = ::jack_client_new("LiveMix");

    //client = 0;
    if (client) {
        ::jack_set_process_callback(client, LiveMix::process, this);
        qDebug() << "JackBackend::JackBackend() activate";
        ::jack_activate(client);

        qDebug() << "JackBackend::JackBackend()" << (::jack_is_realtime(client) == 1 ? "Realtime ;-)" : "Non real time :(")
        << ", buffer size:" << ::jack_get_buffer_size(client) << ", sample rate:" << ::jack_get_sample_rate(client);
    } else {
        qWarning() << "\n No jack-connection! :(\n\n";
        gui->message(QObject::trUtf8("No Jack-connection :-("),
                     QObject::trUtf8("<qt><p>Sorry, I couldn't connect to Jack. This probably means that <b>no jackd is running</b>. Please start it (for example by using QJackCtl) and try LiveMix again.</p></qt>"));
        exit(-1);
    }
//    midi_in = jack_port_register(client, "control", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
//    midi_out = jack_port_register(client, "control", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
//    snd_midi_event_init(midi_in);
//    snd_midi_event_init(midi_out);
    if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0) >= 0) {
        snd_seq_set_client_name(seq, "LiveMix");
        m_iPort = snd_seq_create_simple_port(seq, "control", 0, 
                SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_SOFTWARE | SND_SEQ_PORT_TYPE_APPLICATION);
/*        midi_in = snd_seq_create_simple_port(seq, "control", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, 
                SND_SEQ_PORT_TYPE_APPLICATION);
        midi_out = snd_seq_create_simple_port(seq, "control", SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, 
                SND_SEQ_PORT_TYPE_APPLICATION);*/
        m_iMidi = snd_seq_create_simple_port(seq, "control", 
                SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, 
                SND_SEQ_PORT_TYPE_APPLICATION);
/*        snd_seq_ev_clear (&SEv);
        snd_seq_ev_set_source (&SEv, port_id);
        snd_seq_ev_set_subs (&SEv);
        snd_seq_ev_set_direct (&SEv);
        snd_midi_event_new (1024, &decoder);
        snd_midi_event_new (64, &encoder);
        snd_midi_event_init (decoder);
        snd_midi_event_init (encoder);

        snd_seq_port_subscribe_t *sub;
        snd_seq_addr_t seq_addr;
        
        snd_seq_port_subscribe_alloca(&sub);

        seq_addr.client = snd_seq_client_id (seq);
        seq_addr.port   = port_id;
        snd_seq_port_subscribe_set_sender(sub, &seq_addr);
        snd_seq_port_subscribe_set_dest(sub, &seq_addr);

        snd_seq_subscribe_port (seq, sub);*/
    } else {
        qDebug() << "The ALSA MIDI system is not available. No ports based on it will be created";
    }

    addOutput(MAIN, true);
    addOutput(MONO, false);
    addOutput(PFL, true);

    _run = true;
    qDebug() << "JackBackend::JackBackend() finished";
}

Backend::~Backend()
{
    qDebug() << "JackBackend::~JackBackend()";
    if (client) {
        qDebug() << " return code" << ::jack_deactivate(client);
        qDebug() << " return code" << ::jack_client_close(client);
    }
}

Backend* Backend::instance()
{
    return m_pInstance;
};
void Backend::init(GuiServer_Interface* p_pGui)
{
    m_pInstance = new Backend(p_pGui);
};

void Backend::run(bool run)
{
    _run = run;
}

uint Backend::getSampleRate()
{
    return ::jack_get_sample_rate(client);
}

float Backend::getCPULoad()
{
    return ::jack_cpu_load(client);
}

uint Backend::getBufferSize()
{
    return ::jack_get_buffer_size(client);
}

float Backend::getProcessTime()
{
    return m_fProcessTime;
}

float Backend::getMaxProcessTime()
{
    return m_fMaxProcessTime;
}


bool Backend::addOutput(jack_port_t** l, jack_port_t** r, QString name, QString prefix, bool stereo)
{
    if (client) {
        if (stereo) {
            *l = jack_port_register(client, (prefix + "_" + name + "_l").toStdString().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
            *r = jack_port_register(client, (prefix + "_" + name + "_r").toStdString().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        } else {
            *l = jack_port_register(client, (prefix + "_" + name + "_m").toStdString().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
            *r = NULL;
        }
        return true;
    }
    return false;
}
bool Backend::addInput(jack_port_t** l, jack_port_t** r, QString name, QString prefix, bool stereo)
{
    if (client) {
        if (stereo) {
            *l = jack_port_register(client, (prefix + "_" + name + "_l").toStdString().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
            *r = jack_port_register(client, (prefix + "_" + name + "_r").toStdString().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        } else {
            *l = jack_port_register(client, (prefix + "_" + name + "_m").toStdString().c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
            *r = NULL;
        }
        return true;
    }
    return false;
}

bool Backend::removeOutput(jack_port_t* l, jack_port_t* r, QString /*name*/, QString /*prefix*/, bool stereo)
{
    if (stereo) {
        jack_port_unregister(client, l);
        jack_port_unregister(client, r);
    } else {
        jack_port_unregister(client, l);
    }
    return true;
}
bool Backend::removeInput(jack_port_t* l, jack_port_t* r, QString /*name*/, QString /*prefix*/, bool stereo)
{
    if (stereo) {
        jack_port_unregister(client, l);
        jack_port_unregister(client, r);
    } else {
        jack_port_unregister(client, l);
    }
    return true;
}

jack_default_audio_sample_t Backend::getInPeak(QString ch, bool left)
{
    //qDebug() << "Backend::getInPeak(QString " << ch << " )";
    if (left) {
        return ins[ch]->peak_l;
    } else {
        return ins[ch]->peak_r;
    }
}
jack_default_audio_sample_t Backend::getOutPeak(QString ch, bool left)
{
    //qDebug() << "Backend::getOutPeak(QString " << ch << " )";
    if (left) {
        return outs[ch]->peak_l;
    } else {
        return outs[ch]->peak_r;
    }
}
jack_default_audio_sample_t Backend::getPrePeak(QString ch, bool left)
{
    //qDebug() << "Backend::getInPeak(QString " << ch << " )";
    if (left) {
        return pres[ch]->peak_l;
    } else {
        return pres[ch]->peak_r;
    }
}
jack_default_audio_sample_t Backend::getPostPeak(QString ch, bool left)
{
    //qDebug() << "Backend::getInPeak(QString " << ch << " )";
    if (left) {
        return posts[ch]->peak_l;
    } else {
        return posts[ch]->peak_r;
    }
}
jack_default_audio_sample_t Backend::getSubPeak(QString ch, bool left)
{
    //qDebug() << "Backend::getInPeak(QString " << ch << " )";
    if (left) {
        return subs[ch]->peak_l;
    } else {
        return subs[ch]->peak_r;
    }
}

void Backend::sendMidiEvent(unsigned char p_iChannel, unsigned int p_iController, signed int p_iValue) {
//    qDebug()<<111<<p_iChannel<<p_iController<<p_iValue;
    snd_seq_event_t ev;
    
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, m_iMidi);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);
    snd_seq_ev_set_controller(&ev,p_iChannel,p_iController,p_iValue);
    
    snd_seq_event_output(seq, &ev);
    snd_seq_drain_output(seq);
}

bool Backend::hasMidiEvent() {
    return snd_seq_event_input_pending(seq, m_iMidi) > 0;
}

snd_seq_event_t* Backend::readMidiEvent() {
/*            while (snd_seq_event_input_pending(backend->seq, backend->midi_in) > 0) {
                snd_seq_event_t *ev;
                snd_seq_event_input(backend->seq, &ev);
                if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
//                    qDebug()<<ev->source.client<<ev->source.port<<ev->dest.client<<ev->dest.port;
                    qDebug()<<ev->data.control.channel<<ev->data.control.param<<ev->data.control.value;
                    snd_seq_free_event(ev);
                }
            }*/
    snd_seq_event_t *ev;
    snd_seq_event_input(seq, &ev);
    snd_seq_free_event(ev);
    return ev;
}

int process(jack_nframes_t nframes, void* arg)
{
// qDebug() << "JackMix::process( jack_nframes_t " << nframes << ", void* )";
    Backend* backend = static_cast<Backend*>(arg);

    if (backend->_run) {
        try {
//qDebug()<<__FILE__<<__LINE__<<"lock";
            backend->_lock.lock();
            
/*            void* jack_buffer = jack_port_get_buffer(backend->midi_in, nframes);
            const jack_nframes_t event_count = jack_midi_get_event_count(jack_buffer, nframes);
 
            for (jack_nframes_t i=0; i < event_count; ++i) {
                jack_midi_event_t ev;
                jack_midi_event_get(&ev, jack_buffer, i, nframes);

                qDebug()<<ev.time<<ev.size<<ev.buffer;
            }*/
            
            foreach(out* elem, backend->outs) {
                if (elem->stereo) {
                    elem->out_s_l = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_l, nframes);
                    elem->out_s_r = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_r, nframes);
                } else {
                    elem->out_s_l = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_l, nframes);
                    elem->out_s_r = elem->out_s_l;
                }
                for (jack_nframes_t n=0; n<nframes; n++) elem->out_s_l[ n ] = 0;
                for (jack_nframes_t n=0; n<nframes; n++) elem->out_s_r[ n ] = 0;
            }
    
            // init listner
            out* pfl_elem = backend->outs[PFL];
            jack_default_audio_sample_t* pfl_l = pfl_elem->out_s_l;
            jack_default_audio_sample_t* pfl_r = pfl_elem->out_s_r;    
            bool pflOn = false;
    
            struct timeval start;
            bool calculate_pk = false;
    //        if (++backend->count % ( 5000 / nframes ) == 0  && backend->getCPULoad() < 95 && backend->count / backend->getCPULoad() > (300 / nframes)) {
            if (backend->getCPULoad() < 95 && ++backend->count * (100 - backend->getCPULoad()) > (300000.0 / nframes)) {
                gettimeofday(&start, NULL);
                backend->count = 0;
                calculate_pk = true;
            }
    
    //  JackMix::ports_it it;
            //qDebug() << "Calculate pre and post in levels.";
            foreach(in* elem, backend->ins) {
                if (calculate_pk) {
                    elem->peak_l = elem->calculate_peak_l;
                    elem->peak_r = elem->calculate_peak_r;
                    elem->calculate_peak_l = 0;
                    elem->calculate_peak_r = 0;
                }
                if (!elem->mute || elem->pfl) {
                    elem->sample_l = (jack_default_audio_sample_t*) jack_port_get_buffer(elem->in_l, nframes);
                    elem->sample_r = elem->stereo ? (jack_default_audio_sample_t*) jack_port_get_buffer(
                                         elem->in_r, nframes) : elem->sample_l;
                    float gain = elem->gain;
                    if (elem->stereo) {
                        for (jack_nframes_t n=0; n<nframes; n++) {
                            elem->pre_l[n] = elem->sample_l[n] * gain;
                            elem->pre_r[n] = elem->sample_r[n] * gain;
                        }
                    } else {
                        for (jack_nframes_t n=0; n<nframes; n++) {
                            elem->pre_l[n] = elem->sample_l[n] * gain;
                            elem->pre_r[n] = elem->pre_l[n];
                        }
                    }
                    /// Effect.
                    foreach(effect* effect, elem->effects) {
                        backend->prossesLadspaFX(effect, elem->pre_l, elem->pre_r, nframes, calculate_pk);
                    }
                    float bal_l = 1-elem->bal;
                    float bal_r = 1+elem->bal;
                    float volume = elem->mute ? 0 : elem->volume;
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        elem->pre_l[n] *= bal_l;
                        elem->pre_r[n] *= bal_r;
                        elem->post_l[n] = elem->pre_l[n] * volume;
                        elem->post_r[n] = elem->pre_r[n] * volume;
//                        if (calculate_pk) {
                            if (elem->pre_l[n] > elem->calculate_peak_l) {
                                elem->calculate_peak_l = elem->pre_l[n];
                            }
                            if (elem->pre_r[n] > elem->calculate_peak_r) {
                                elem->calculate_peak_r = elem->pre_r[n];
                            }
//                        }
                    }
                    /*    if (calculate_pk) {
                         elem->peak_l *= 1-elem->bal;
                         elem->peak_r *= 1+elem->bal;
                        }*/

                    jack_default_audio_sample_t* inl = elem->pre_l;
                    jack_default_audio_sample_t* inr = elem->pre_r;
                    if (elem->pfl) {
                        pflOn = true;
                        if (elem->mute) {
                            for (jack_nframes_t n=0; n<nframes; n++) {
                                pfl_l[ n ] += inl[ n ];
                                pfl_r[ n ] += inr[ n ];
                                inl[n] = 0;
                                inr[n] = 0;
                            }
                            elem->calculate_peak_l = 0;
                            elem->calculate_peak_r = 0;
                        } else {
                            for (jack_nframes_t n=0; n<nframes; n++) {
                                pfl_l[ n ] += inl[ n ];
                                pfl_r[ n ] += inr[ n ];
                            }
                        }
                    }
                } else {
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        elem->pre_l[n] = 0;
                        elem->pre_r[n] = 0;
                        elem->post_l[n] = 0;
                        elem->post_r[n] = 0;
                    }
                }
            }

            //qDebug() << "Blank outports...";
            foreach(pre* elem, backend->pres) {
                elem->pre_l = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_l, nframes);
                if (elem->stereo) {
                    elem->pre_r = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_r, nframes);
                } else {
    
                    elem->pre_r = elem->pre_l;
                }
                for (jack_nframes_t n=0; n<nframes; n++) elem->pre_l[ n ] = 0;
                if (elem->stereo) {
                    for (jack_nframes_t n=0; n<nframes; n++) elem->pre_r[ n ] = 0;
                }
            }
    
            foreach(post* elem, backend->posts) {
                if (elem->external) {
                    elem->post_l = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_l, nframes);
                    elem->post_r = elem->stereo ? (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_r, nframes) : elem->post_l;
                }
                /*   else {
                    elem->post_l = new jack_default_audio_sample_t[nframes];
                    elem->post_r = elem->stereo ? new jack_default_audio_sample_t[nframes] : elem->post_l;
                   }*/
    
                for (jack_nframes_t n=0; n<nframes; n++) elem->post_l[ n ] = 0;
                if (elem->stereo) {
                    for (jack_nframes_t n=0; n<nframes; n++) elem->post_r[ n ] = 0;
                }
            }
    
            foreach(sub* elem, backend->subs) {
                if (elem->stereo) {
                    elem->sub_l = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_l, nframes);
                    elem->sub_r = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_r, nframes);
                } else {
                    elem->sub_l = (jack_default_audio_sample_t*)jack_port_get_buffer(elem->out_l, nframes);
                    elem->sub_r = elem->sub_l;
                }
                for (jack_nframes_t n=0; n<nframes; n++) elem->sub_l[ n ] = 0;
                for (jack_nframes_t n=0; n<nframes; n++) elem->sub_r[ n ] = 0;
            }
    
            //qDebug() << "The actual pre.";
            //qDebug() << "Adjust prelevels.";
            foreach(pre* pre_elem, backend->pres) {
                if (calculate_pk) {
                    pre_elem->peak_l = pre_elem->calculate_peak_l;
                    pre_elem->peak_r = pre_elem->calculate_peak_r;
                    pre_elem->calculate_peak_l = 0;
                    pre_elem->calculate_peak_r = 0;
                }
                if (!pre_elem->mute) {
                    /// Effect.
                    foreach(effect* effect, pre_elem->effects) {
                        backend->prossesLadspaFX(effect, pre_elem->pre_l, pre_elem->pre_r, nframes, calculate_pk);
                    }
    
                    float vol_l = pre_elem->volume * (1-pre_elem->bal);
                    float vol_r = pre_elem->volume * (1+pre_elem->bal);
                    foreach(in* in_elem, backend->ins) {
                        float volume = in_elem->pre[ pre_elem->name ];
                        float vl = volume * vol_l;
                        float vr = volume * vol_r;
                        for (jack_nframes_t n=0; n<nframes; n++) {
                            pre_elem->pre_l[ n ] += in_elem->pre_l[ n ] * vl;
                            pre_elem->pre_r[ n ] += in_elem->pre_r[ n ] * vr;
//                            if (calculate_pk) {
                                if (pre_elem->pre_l[ n ] > pre_elem->calculate_peak_l) {
                                    pre_elem->calculate_peak_l = pre_elem->pre_l[ n ];
                                }
                                if (pre_elem->pre_r[ n ] > pre_elem->calculate_peak_r) {
                                    pre_elem->calculate_peak_r = pre_elem->pre_r[ n ];
                                }
//                            }
                        }
                    }
                }
            }
    
            //qDebug() << "The actual post.";
            foreach(post* post_elem, backend->posts) {
                if (calculate_pk) {
                    post_elem->peak_l = post_elem->calculate_peak_l;
                    post_elem->peak_r = post_elem->calculate_peak_r;
                    post_elem->calculate_peak_l = 0;
                    post_elem->calculate_peak_r = 0;
                }
    
                if (!post_elem->mute) {
                    jack_default_audio_sample_t* outl = post_elem->post_l;
                    jack_default_audio_sample_t* outr = post_elem->post_r;
                    float vol_l = post_elem->prevolume;
                    float vol_r = post_elem->prevolume;
    
                    foreach(in* in_elem, backend->ins) {
                        jack_default_audio_sample_t* inl = in_elem->post_l;
                        jack_default_audio_sample_t* inr = in_elem->post_r;
                        float volume = in_elem->post[ post_elem->name ];
                        float vl = volume * vol_l;
                        float vr = volume * vol_r;
                        for (jack_nframes_t n=0; n<nframes; n++) {
                            outl[ n ] += inl[ n ] * vl;
                            outr[ n ] += inr[ n ] * vr;
                        }
                    }
    
                    // plf
                    if (post_elem->m_bPfl) {
                        pflOn = true;
                        jack_default_audio_sample_t* inl = post_elem->post_l;
                        jack_default_audio_sample_t* inr = post_elem->post_r;
                        for (jack_nframes_t n=0; n<nframes; n++) {
                            pfl_l[ n ] += inl[ n ];
                            pfl_r[ n ] += inr[ n ];
                        }
                    }
    
                    /// Effect.
                    foreach(effect* effect, post_elem->effects) {
                        backend->prossesLadspaFX(effect, outl, outr, nframes, calculate_pk);
                    }
    
                    //qDebug() << "Calculate return levels.";
    
                    /*    if (post_elem->return_l == NULL) {
                         post_elem->return_l = new jack_default_audio_sample_t[nframes];
                         post_elem->return_r = new jack_default_audio_sample_t[nframes];
                        }*/
    
                    /*    jack_default_audio_sample_t* sample_l;
                        jack_default_audio_sample_t* sample_r;*/
                    if (post_elem->external) {
                        post_elem->return_sample_l = (jack_default_audio_sample_t*) jack_port_get_buffer(post_elem->in_l, nframes);
                        post_elem->return_sample_r = post_elem->stereo ? (jack_default_audio_sample_t*)
                                                     jack_port_get_buffer(post_elem->in_r, nframes) : post_elem->return_sample_l;
                    } else {
                        post_elem->return_sample_l = post_elem->post_l;
                        post_elem->return_sample_r = post_elem->post_r;
                    }
                    float volume_l = post_elem->postvolume * (1-post_elem->bal);
                    float volume_r = post_elem->postvolume * (1+post_elem->bal);
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        post_elem->return_l[n] = post_elem->return_sample_l[n] * volume_l;
                        post_elem->return_r[n] = post_elem->return_sample_r[n] * volume_r;
//                        if (calculate_pk) {
                            if (post_elem->return_l[ n ] > post_elem->calculate_peak_l) {
                                post_elem->calculate_peak_l = post_elem->return_l[ n ];
                            }
                            if (post_elem->return_r[ n ] > post_elem->calculate_peak_r) {
                                post_elem->calculate_peak_r = post_elem->return_r[ n ];
                            }
//                        }
                    }
                } else {
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        post_elem->return_l[n] = 0;
                        post_elem->return_r[n] = 0;
                    }
                }
            }
            //qDebug() << "The actual sub.";
            //qDebug() << "in";
            foreach(sub* sub_elem, backend->subs) {
                if (calculate_pk) {
                    sub_elem->peak_l = sub_elem->calculate_peak_l;
                    sub_elem->peak_r = sub_elem->calculate_peak_r;

                    sub_elem->peak_l *= 1-sub_elem->bal;
                    sub_elem->peak_r *= 1+sub_elem->bal;
                    
                    sub_elem->calculate_peak_l = 0;
                    sub_elem->calculate_peak_r = 0;
                }
                if (!sub_elem->mute) {
                    jack_default_audio_sample_t* outl = sub_elem->sub_l;
                    jack_default_audio_sample_t* outr = sub_elem->sub_r;
    
                    foreach(in* in_elem, backend->ins) {
                        jack_default_audio_sample_t* inl = in_elem->post_l;
                        jack_default_audio_sample_t* inr = in_elem->post_r;
                        bool on = in_elem->sub[ sub_elem->name ];
                        if (on) {
                            for (jack_nframes_t n=0; n<nframes; n++) {
                                outl[ n ] += inl[ n ];
                                outr[ n ] += inr[ n ];
                            }
                        }
                    }
                    //qDebug() << "post return";
                    foreach(post* post_elem, backend->posts) {
                        jack_default_audio_sample_t* inl = post_elem->return_l;
                        jack_default_audio_sample_t* inr = post_elem->return_r;
                        bool on = post_elem->sub[ sub_elem->name ];
                        if (on) {
                            for (jack_nframes_t n=0; n<nframes; n++) {
                                outl[ n ] += inl[ n ];
                                outr[ n ] += inr[ n ];
                            }
                        }
                    }
    
                    //qDebug() << "Effect.";
                    foreach(effect* effect, sub_elem->effects) {
                        backend->prossesLadspaFX(effect, outl, outr, nframes, calculate_pk);
                    }
                    /// Adjust sublevels.
                    float vol_l = sub_elem->volume;
                    float vol_r = sub_elem->volume;
                    if (sub_elem->stereo) { // elsa calculate later
                        vol_l *= 1-sub_elem->bal;
                        vol_r *= 1+sub_elem->bal;
                    }
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        outl[ n ] *= vol_l;
                        outr[ n ] *= vol_r;
//                        if (calculate_pk) {
                            if (outl[ n ] > sub_elem->calculate_peak_l) {
                                sub_elem->calculate_peak_l = outl[ n ];
                            }
                            if (outr[ n ] > sub_elem->calculate_peak_r) {
                                sub_elem->calculate_peak_r = outr[ n ];
                            }
//                        }
                    }
                }
            }
            //qDebug() << "The actual main.";
            out* main_elem = backend->outs[MAIN];
    
            {
                if (calculate_pk) {
                    main_elem->peak_l = main_elem->calculate_peak_l;
                    main_elem->peak_r = main_elem->calculate_peak_r;

                    main_elem->calculate_peak_l = 0;
                    main_elem->calculate_peak_r = 0;
                }
                if (!main_elem->mute) {
                    jack_default_audio_sample_t* main_l = main_elem->out_s_l;
                    jack_default_audio_sample_t* main_r = main_elem->out_s_r;
                    // in
                    foreach(in* in_elem, backend->ins) {
                        jack_default_audio_sample_t* inl = in_elem->post_l;
                        jack_default_audio_sample_t* inr = in_elem->post_r;
                        if (in_elem->main) {
                            for (jack_nframes_t n=0; n<nframes; n++) {
                                main_l[ n ] += inl[ n ];
                                main_r[ n ] += inr[ n ];
                            }
                        }
                    }
                    //qDebug() << "post return";
                    foreach(post* post_elem, backend->posts) {
                        jack_default_audio_sample_t* inl = post_elem->return_l;
                        jack_default_audio_sample_t* inr = post_elem->return_r;
                        if (post_elem->main) {
                            for (jack_nframes_t n=0; n<nframes; n++) {
                                main_l[ n ] += inl[ n ];
                                main_r[ n ] += inr[ n ];
                            }
                        }
                    }
                    //qDebug() << "sub";
                    foreach(sub* sub_elem, backend->subs) {
                        jack_default_audio_sample_t* inl = sub_elem->sub_l;
                        jack_default_audio_sample_t* inr = sub_elem->sub_r;
                        if (sub_elem->main) {
                            if (sub_elem->stereo || sub_elem->bal == 0) {
                                for (jack_nframes_t n=0; n<nframes; n++) {
                                    main_l[ n ] += inl[ n ];
                                    main_r[ n ] += inr[ n ];
                                }
                            } else { // calcualte bal
                                float vol_l = 1-sub_elem->bal;
                                float vol_r = 1+sub_elem->bal;
                                for (jack_nframes_t n=0; n<nframes; n++) {
                                    main_l[ n ] += inl[ n ] * vol_l;
                                    main_r[ n ] += inr[ n ] * vol_r;
                                }
                            }
                        }
                    }
                    //qDebug() << "Effect.";
                    {
                        foreach(effect* effect, main_elem->effects) {
                            backend->prossesLadspaFX(effect, main_l, main_r, nframes, calculate_pk);
                        }
                    }
                    /// Adjust outlevels.
                    out* mono_elem = backend->outs[MONO];
                    jack_default_audio_sample_t* mono = mono_elem->out_s_l;
                    {
                        float vol_l = main_elem->volume * (1-main_elem->bal);
                        float vol_r = main_elem->volume * (1+main_elem->bal);
                        float vol_m = main_elem->volume * mono_elem->volume;
                        for (jack_nframes_t n=0; n<nframes; n++) {
                            mono[ n ] = (main_l[ n ] + main_r[ n ]) * vol_m;
                            main_l[ n ] *= vol_l;
                            main_r[ n ] *= vol_r;
//                            if (calculate_pk) {
                                if (main_l[ n ] > main_elem->calculate_peak_l) {
                                    main_elem->calculate_peak_l = main_l[ n ];
                                }
                                if (main_r[ n ] > main_elem->calculate_peak_r) {
                                    main_elem->calculate_peak_r = main_r[ n ];
                                }
//                            }
                        }
                    }
                }
            }
            //qDebug() << "plf / alf";
            // in
//            foreach(in* in_elem, backend->ins) {
//            }
    
            //qDebug() << 25;
            // pre
            foreach(pre* elem, backend->pres) {
                if (elem->afl) {
                    jack_default_audio_sample_t* inl = elem->pre_l;
                    jack_default_audio_sample_t* inr = elem->pre_r;
                    pflOn = true;
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        pfl_l[ n ] += inl[ n ];
                        pfl_r[ n ] += inr[ n ];
                    }
                }
            }
            //qDebug() << 26;
            // post return
            foreach(post* elem, backend->posts) {
                //qDebug() << 27;
                if (elem->m_bAfl) {
                    pflOn = true;
                    jack_default_audio_sample_t* inl = elem->return_l;
                    jack_default_audio_sample_t* inr = elem->return_r;
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        pfl_l[ n ] += inl[ n ];
                        pfl_r[ n ] += inr[ n ];
                    }
                }
            }
            //qDebug() << 28;
            // sub
            foreach(sub* elem, backend->subs) {
                if (elem->afl) {
                    pflOn = true;
                    jack_default_audio_sample_t* inl = elem->sub_l;
                    jack_default_audio_sample_t* inr = elem->sub_r;
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        pfl_l[ n ] += inl[ n ];
                        pfl_r[ n ] += inr[ n ];
                    }
                }
            }
            //qDebug() << 29;
            // main
            {
                if ((!pflOn) || main_elem->afl) {
                    jack_default_audio_sample_t* inl = main_elem->out_s_l;
                    jack_default_audio_sample_t* inr = main_elem->out_s_r;
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        pfl_l[ n ] += inl[ n ];
                        pfl_r[ n ] += inr[ n ];
                    }
                }
                //qDebug() << 30;
    
                /// Adjust outlevels.
                {
                    float volume = pfl_elem->volume;
                    for (jack_nframes_t n=0; n<nframes; n++) {
                        pfl_l[ n ] *= volume;
                        pfl_r[ n ] *= volume;
                    }
                }
            }
            //qDebug() << 31;
    
            if (calculate_pk) {
                struct timeval end;
                gettimeofday(&end, NULL);
                m_fProcessTime = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                m_fMaxProcessTime = 1000.0 / (backend->getSampleRate() / nframes);
    
                emit backend->processed();
    //   qDebug()<<__FILE__<<__LINE__<<::jack_cpu_load(backend->client);
            }
    
    //  struct timeval end;
    //  gettimeofday(&end, NULL);
    //  m_fProcessTime = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    //  m_fProcessTime = ::jack_cpu_load(backend->client);
    //  m_fMaxProcessTime = 1000.0 / (backend->getSampleRate() / nframes);
    
            //qDebug() << 32;
    
            //DEBUG
            /*  if ( m_fProcessTime > m_fMaxProcessTime ) {
               qDebug() << "";
               qDebug() << "----XRUN----";
               qDebug() << QString("XRUN of %1 msec (%2 > %3)").arg(m_fProcessTime - m_fMaxProcessTime).arg(m_fProcessTime).arg(m_fMaxProcessTime);
            //   qDebug() << "Playing notes = " + m_nPlayingNotes + ", render time = " + fRenderTime;
            //   qDebug() << "Ladspa process time = " + fLadspaTime;
               qDebug() << "------------";
               qDebug() << "";
               // raise xRun event
               emit backend->xrun();
              }*/
//qDebug()<<__FILE__<<__LINE__<<"end";
        }
        catch(...) {
//qDebug()<<__FILE__<<__LINE__<<"error !";
        }
//qDebug()<<__FILE__<<__LINE__<<"unlock";
            backend->_lock.unlock();
    }
    return 0;
}
void Backend::prossesLadspaFX(effect* pFX, float* left_channel, float* right_channel, uint nframes, bool p_bCalculatePk)
{
#ifdef LADSPA_SUPPORT
    // Process LADSPA FX
    if (pFX->fx->isEnabled()) {
        struct timeval start;
        gettimeofday(&start, NULL);
//qDebug() << QString("processLadspaFX - Label: %1, nb in: %2, nb out: %3, in l: %4, in r: %5")
//  .arg(m_sLabel).arg(m_nIAPorts).arg(m_nOAPorts).arg(right_channel[0]).arg(left_channel[0]);

        if (pFX->fx->getInputAudio() >=  2 || pFX->fx->getOutputAudio() == 1 && pFX->fx->getInputAudio() == 1) {
            for (unsigned i = 0; i < nframes; ++i) {
                pFX->fx->m_pInBufferL[i] = left_channel[i];
                pFX->fx->m_pInBufferR[i] = right_channel[i];
            }
        } else {
            if (left_channel != right_channel) {
                for (unsigned i = 0; i < nframes; ++i) {
                    pFX->fx->m_pInBufferL[i] = left_channel[i] + right_channel[i];
                }
            } else {
                for (unsigned i = 0; i < nframes; ++i) {
                    pFX->fx->m_pInBufferL[i] = left_channel[i];
                }
            }
        }
        pFX->fx->processFX(nframes, left_channel != right_channel);
        if (left_channel != right_channel) {
            if (pFX->fx->getOutputAudio() == 2 || pFX->fx->getOutputAudio() == 1 && pFX->fx->getInputAudio() == 1) {
                for (unsigned i = 0; i < nframes; ++i) {
                    left_channel[i] = pFX->fx->m_pOutBufferL[i];
                    right_channel[i] = pFX->fx->m_pOutBufferR[i];
                }
            } else {
                for (unsigned i = 0; i < nframes; ++i) {
                    left_channel[i] = pFX->fx->m_pOutBufferL[i];
                    right_channel[i] = pFX->fx->m_pOutBufferR[i];
                }
            }
        } else {
            if (pFX->fx->getOutputAudio() == 2 || pFX->fx->getOutputAudio() == 1 && pFX->fx->getInputAudio() == 1) {
                for (unsigned i = 0; i < nframes; ++i) {
                    left_channel[i] = pFX->fx->m_pOutBufferL[i];
                }
            } else {
                for (unsigned i = 0; i < nframes; ++i) {
                    left_channel[i] = pFX->fx->m_pOutBufferL[i] + pFX->fx->m_pOutBufferR[i];
                }
            }
        }
        if (p_bCalculatePk && ++pFX->m_iCount % 5 == 0) {
            //  qDebug()<<__FILE__<<__LINE__<<pFX->fx->getPluginName()<<(int)left_channel<<(int)right_channel<<(pFX->fx->getPluginType() == LadspaFX::STEREO_FX);
            pFX->m_iCount = 0;
            struct timeval end;
            gettimeofday(&end, NULL);
            float time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
            pFX->m_fCpuUse = (pFX->m_fCpuUse + (time / m_fMaxProcessTime)) / 2;
        }
    }
#endif
}
const QList<QString>& Backend::inchannels()
{
    return ins_order;
}
const QList<QString>& Backend::outchannels()
{
    return outs_order;
}
const QList<QString>& Backend::prechannels()
{
    return pres_order;
}
const QList<QString>& Backend::postchannels()
{
    return posts_order;
}
const QList<QString>& Backend::subchannels()
{
    return subs_order;
}

void Backend::setInVolume(QString ch, float volume)
{
    ins[ ch ]->volume = volume;
}
void Backend::setInGain(QString ch, float gain)
{
    ins[ ch ]->gain = gain;
}
void Backend::setInBal(QString ch, float bal)
{
    ins[ ch ]->bal = bal;
}
void Backend::setInMute(QString ch, bool mute)
{
    ins[ ch ]->mute = mute;
}
void Backend::setInPfl(QString ch, bool pfl)
{
    ins[ ch ]->pfl = pfl;
}
void Backend::setInMain(QString ch, bool main)
{
    ins[ ch ]->main = main;
}
void Backend::setInPreVolume(QString ch, QString pre, float volume)
{
    ins[ ch ]->pre[ pre ] = volume;
}
void Backend::setInPostVolume(QString ch, QString post, float volume)
{
    ins[ ch ]->post[ post ] = volume;
}
void Backend::setInSub(QString ch, QString sub, bool on)
{
    ins[ ch ]->sub[ sub ] = on;
}
effect* Backend::addInEffect(QString ch, LadspaFX* fx)
{
    effect* e = new effect(fx, ::jack_get_buffer_size(client));
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    ins[ ch ]->effects << e;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    return e;
}
void Backend::removeInEffect(QString ch, effect* eff)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    ins[ ch ]->effects.removeAll(eff);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
}
QList<effect*>* Backend::getInEffects(QString ch)
{
    return &(ins[ ch ]->effects);
}

void Backend::setOutVolume(QString ch, float volume)
{
    outs[ ch ]->volume = volume;
}

void Backend::setOutBal(QString ch, float bal)
{
    outs[ ch ]->bal = bal;
}

void Backend::setOutMute(QString ch, bool mute)
{
    outs[ ch ]->mute = mute;
}
void Backend::setOutAfl(QString ch, bool afl)
{
    outs[ ch ]->afl = afl;
}
effect* Backend::addOutEffect(QString ch, LadspaFX* fx)
{
    effect* e = new effect(fx, ::jack_get_buffer_size(client));
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    outs[ ch ]->effects << e;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    return e;
}
void Backend::removeOutEffect(QString ch, effect* eff)
{
    _lock.lock();
    outs[ ch ]->effects.removeAll(eff);
    _lock.unlock();
}
QList<effect*>* Backend::getOutEffects(QString ch)
{
    return &(outs[ ch ]->effects);
}

void Backend::setPreVolume(QString ch, float volume)
{
    pres[ ch ]->volume = volume;
}
void Backend::setPreBal(QString ch, float bal)
{
    pres[ ch ]->bal = bal;
}
void Backend::setPreMute(QString ch, bool mute)
{
    pres[ ch ]->mute = mute;
}
void Backend::setPreAfl(QString ch, bool afl)
{
    pres[ ch ]->afl = afl;
}
effect* Backend::addPreEffect(QString ch, LadspaFX* fx)
{
    effect* e = new effect(fx, ::jack_get_buffer_size(client));
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    pres[ ch ]->effects << e;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    return e;
}
void Backend::removePreEffect(QString ch, effect* eff)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    pres[ ch ]->effects.removeAll(eff);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
}
QList<effect*>* Backend::getPreEffects(QString ch)
{
    return &(pres[ ch ]->effects);
}

void Backend::setPostPreVolume(QString ch, float volume)
{
    posts[ ch ]->prevolume = volume;
}
void Backend::setPostPostVolume(QString ch, float volume)
{
    posts[ ch ]->postvolume = volume;
}
void Backend::setPostBal(QString ch, float bal)
{
    posts[ ch ]->bal = bal;
}
void Backend::setPostMute(QString ch, bool mute)
{
    posts[ ch ]->mute = mute;
}
void Backend::setPostPfl(QString ch, bool pfl)
{
    posts[ ch ]->m_bPfl = pfl;
}
void Backend::setPostAfl(QString ch, bool afl)
{
    posts[ ch ]->m_bAfl = afl;
}
void Backend::setPostMain(QString ch, bool main)
{
    posts[ ch ]->main = main;
}
void Backend::setPostSub(QString ch, QString sub, bool on)
{
    posts[ ch ]->sub[ sub ] = on;
}
effect* Backend::addPostEffect(QString ch, LadspaFX* fx)
{
    effect* e = new effect(fx, ::jack_get_buffer_size(client));
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    posts[ ch ]->effects << e;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    return e;
}
void Backend::removePostEffect(QString ch, effect* eff)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    posts[ ch ]->effects.removeAll(eff);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
}
QList<effect*>* Backend::getPostEffects(QString ch)
{
    return &(posts[ ch ]->effects);
}

void Backend::setSubVolume(QString ch, float volume)
{
    subs[ ch ]->volume = volume;
}
void Backend::setSubBal(QString ch, float bal)
{
    subs[ ch ]->bal = bal;
}
void Backend::setSubMute(QString ch, bool mute)
{
    subs[ ch ]->mute = mute;
}
void Backend::setSubAfl(QString ch, bool afl)
{
    subs[ ch ]->afl = afl;
}
void Backend::setSubMain(QString ch, bool main)
{
    subs[ ch ]->main = main;
}
effect* Backend::addSubEffect(QString ch, LadspaFX* fx)
{
    effect* e = new effect(fx, ::jack_get_buffer_size(client));
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    subs[ ch ]->effects << e;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    return e;
}
void Backend::removeSubEffect(QString ch, effect* eff)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    subs[ ch ]->effects.removeAll(eff);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
}
QList<effect*>* Backend::getSubEffects(QString ch)
{
    return &(subs[ ch ]->effects);
}

bool Backend::addInput(QString name, bool stereo)
{
    jack_port_t** l = new jack_port_t*;
    jack_port_t** r = new jack_port_t*;
    bool result = addInput(l, r, name, "in", stereo);
    if (result) {
//qDebug()<<__FILE__<<__LINE__<<"lock";
        _lock.lock();
        ins[ name ] = new in(name, stereo, ::jack_get_buffer_size(client), *l, *r);
        ins_order << name;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
        _lock.unlock();
    }
    return result;
}
bool Backend::addOutput(QString name, bool stereo)
{
    jack_port_t** l = new jack_port_t*;
    jack_port_t** r = new jack_port_t*;
    bool result = addOutput(l, r, name, "out", stereo);
    if (result) {
//qDebug()<<__FILE__<<__LINE__<<"lock";
        _lock.lock();
        outs[ name ] = new out(name, stereo, ::jack_get_buffer_size(client), *l, *r);
        outs_order << name;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
        _lock.unlock();
    }
    return result;
}
bool Backend::addPre(QString name, bool stereo)
{
    jack_port_t** l = new jack_port_t*;
    jack_port_t** r = new jack_port_t*;
    bool result = addOutput(l, r, name, "pre", stereo);
    if (result) {
//qDebug()<<__FILE__<<__LINE__<<"lock";
        _lock.lock();
        pres[ name ] = new pre(name, stereo, ::jack_get_buffer_size(client), *l, *r);
        pres_order << name;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
        _lock.unlock();
    }
    return result;
}
bool Backend::addPost(QString name, bool stereo, bool external)
{
    jack_port_t** s_l = new jack_port_t*;
    jack_port_t** s_r = new jack_port_t*;
    jack_port_t** r_l = new jack_port_t*;
    jack_port_t** r_r = new jack_port_t*;
    bool result = true;
    if (external) {
        result &= addOutput(s_l, s_r, name, "post", stereo);
        result &= addInput(r_l, r_r, name, "return", stereo);
    }
    if (result) {
//qDebug()<<__FILE__<<__LINE__<<"lock";
        _lock.lock();
        posts[ name ] = new post(name, stereo, external, ::jack_get_buffer_size(client), *s_l, *s_r, *r_l, *r_r);
        posts_order << name;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
        _lock.unlock();
    }
    return result;
}
bool Backend::addSub(QString name, bool stereo)
{
    jack_port_t** l = new jack_port_t*;
    jack_port_t** r = new jack_port_t*;
    bool result = addOutput(l, r, name, "sub", stereo);
    if (result) {
//qDebug()<<__FILE__<<__LINE__<<"lock";
        _lock.lock();
        subs[ name ] = new sub(name, stereo, ::jack_get_buffer_size(client), *l, *r);
        subs_order << name;
//qDebug()<<__FILE__<<__LINE__<<"unlock";
        _lock.unlock();
    }
    return result;
}

bool Backend::removeInput(QString name)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    ins_order.removeAll(name);
    in* elem = ins[name];
    ins.remove(name);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    removeInput(elem->in_l, elem->in_r, name, "in", elem->stereo);
    delete elem;
    return true;
}

bool Backend::removeOutput(QString name)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    outs_order.removeAll(name);
    out* elem = outs[name];
    outs.remove(name);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    removeOutput(elem->out_l, elem->out_r, name, "out", elem->stereo);
    delete elem;
    return true;
}

bool Backend::removePre(QString name)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    pres_order.removeAll(name);
    pre* elem = pres[name];
    pres.remove(name);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    removeOutput(elem->out_l, elem->out_r, name, "pre", elem->stereo);
    delete elem;
    return true;
}
bool Backend::removePost(QString name)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    posts_order.removeAll(name);
    post* elem = posts[name];
    posts.remove(name);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    if (elem->external) {
        removeOutput(elem->out_l, elem->out_r, name, "post", elem->stereo);
        removeInput(elem->in_l, elem->in_r, name, "return", elem->stereo);
    }
    delete elem;
    return true;
}
bool Backend::removeSub(QString name)
{
//qDebug()<<__FILE__<<__LINE__<<"lock";
    _lock.lock();
    subs_order.removeAll(name);
    sub* elem = subs[name];
    subs.remove(name);
//qDebug()<<__FILE__<<__LINE__<<"unlock";
    _lock.unlock();
    removeOutput(elem->out_l, elem->out_r, name, "sub", elem->stereo);
    delete elem;
    return true;
}

channel* Backend::getChannel(ChannelType p_eType, QString p_rName)
{
    switch (p_eType) {
    case IN:
        return ins[p_rName];
    case OUT:
        return outs[p_rName];
    case PRE:
        return pres[p_rName];
    case POST:
        return posts[p_rName];
    case SUB:
        return subs[p_rName];
    default:
        return NULL;
    }
}
const in* Backend::getInput(QString name)
{
    return ins[ name ];
}
const out* Backend::getOutput(QString name)
{
    return outs[ name ];
}
const pre* Backend::getPre(QString name)
{
    return pres[ name ];
}
const post* Backend::getPost(QString name)
{
    return posts[ name ];
}
const sub* Backend::getSub(QString name)
{
    return subs[ name ];
}

bool Backend::moveEffect(ChannelType p_eType, QString p_rName, effect* p_pEffect, bool p_bLeft)
{
    channel *ch = getChannel(p_eType, p_rName);
    int index = ch->effects.indexOf(p_pEffect);
    int indexTo = index + (p_bLeft ? -1 : 1);
    if (indexTo >= 0 && indexTo < ch->effects.size()) {
qDebug()<<__FILE__<<__LINE__<<1248;
        _lock.lock();
        ch->effects.move(index, indexTo);
qDebug()<<__FILE__<<__LINE__<<1251;
        _lock.unlock();
        return true;
    }
    return false;
}

void Backend::saveLash(QString p_rFile)
{
    QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml += "<connexions>";
    foreach(QString name, inchannels()) {
        in *c = (in*)getChannel(IN, name);
        const char **names = jack_port_get_connections(c->in_l);
        if (names) {
            xml +=  QString("  <my type=\"in_l\" name=\"%1\">").arg(name);
            for (int i = 0 ; names[i] != NULL ; i++) {
                xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
            }
            xml +=  "  </my>";
        }
        if (c->stereo) {
            names = jack_port_get_connections(c->in_r);
            if (names) {
                xml +=  QString("  <my type=\"in_r\" name=\"%1\">").arg(name);
                for (int i = 0 ; names[i] != NULL ; i++) {
                    xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                }
                xml +=  "  </my>";
            }
        }
    }
    foreach(QString name, outchannels()) {
        out *c = (out*)getChannel(OUT, name);
        const char **names = jack_port_get_connections(c->out_l);
        if (names) {
            xml +=  QString("  <my type=\"out_l\" name=\"%1\">").arg(name);
            for (int i = 0 ; names[i] != NULL ; i++) {
                xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
            }
            xml +=  "  </my>";
        }
        if (c->stereo) {
            names = jack_port_get_connections(c->out_r);
            if (names) {
                xml +=  QString("  <my type=\"out_r\" name=\"%1\">").arg(name);
                for (int i = 0 ; names[i] != NULL ; i++) {
                    xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                }
                xml +=  "  </my>";
            }
        }
    }
    foreach(QString name, prechannels()) {
        pre *c = (pre*)getChannel(PRE, name);
        const char **names = jack_port_get_connections(c->out_l);
        if (names) {
            xml +=  QString("  <my type=\"pre_l\" name=\"%1\">").arg(name);
            for (int i = 0 ; names[i] != NULL ; i++) {
                xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
            }
            xml +=  "  </my>";
        }
        if (c->stereo) {
            names = jack_port_get_connections(c->out_r);
            if (names) {
                xml +=  QString("  <my type=\"pre_r\" name=\"%1\">").arg(name);
                for (int i = 0 ; names[i] != NULL ; i++) {
                    xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                }
                xml +=  "  </my>";
            }
        }
    }
    foreach(QString name, postchannels()) {
        post *c = (post*)getChannel(POST, name);
        if (c->external) {
            const char **names = jack_port_get_connections(c->out_l);
            if (names) {
                xml +=  QString("  <my type=\"post_l\" name=\"%1\">").arg(name);
                for (int i = 0 ; names[i] != NULL ; i++) {
                    xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                }
                xml +=  "  </my>";
            }
            if (c->stereo) {
                names = jack_port_get_connections(c->out_r);
                if (names) {
                    xml +=  QString("  <my type=\"post_r\" name=\"%1\">").arg(name);
                    for (int i = 0 ; names[i] != NULL ; i++) {
                        xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                    }
                    xml +=  "  </my>";
                }
            }
            names = jack_port_get_connections(c->in_l);
            if (names) {
                xml +=  QString("  <my type=\"return_l\" name=\"%1\">").arg(name);
                for (int i = 0 ; names[i] != NULL ; i++) {
                    xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                }
                xml +=  "  </my>";
            }
            if (c->stereo) {
                names = jack_port_get_connections(c->in_r);
                if (names) {
                    xml +=  QString("  <my type=\"return_r\" name=\"%1\">").arg(name);
                    for (int i = 0 ; names[i] != NULL ; i++) {
                        xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                    }
                    xml +=  "  </my>";
                }
            }
        }
    }
    foreach(QString name, subchannels()) {
        sub *c = (sub*)getChannel(SUB, name);
        const char **names = jack_port_get_connections(c->out_l);
        if (names) {
            xml +=  QString("  <my type=\"sub_l\" name=\"%1\">").arg(name);
            for (int i = 0 ; names[i] != NULL ; i++) {
                xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
            }
            xml +=  "  </my>";
        }
        if (c->stereo) {
            names = jack_port_get_connections(c->out_r);
            if (names) {
                xml +=  QString("  <my type=\"sub_r\" name=\"%1\">").arg(name);
                for (int i = 0 ; names[i] != NULL ; i++) {
                    xml +=  QString("    <to name=\"%1\" />").arg(names[i]);
                }
                xml +=  "  </my>";
            }
        }
    }
    
/*    snd_seq_port_info_t *port_info;
    snd_seq_port_info_alloca (&port_info);
    snd_seq_port_info_set_client(port_info, seq);
    snd_seq_port_info_set_port(port_info, midi_in);
    qDebug<<snd_seq_client_info_get_name(client_info);*/
    
    xml += "</connexions>";
    QFile file(p_rFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << xml.replace(">", ">\n");
        file.close();
    }
}

void Backend::restoreLash(QString p_rFile)
{
    QFile file(p_rFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument doc("connexions");
        doc.setContent(&file);

        QDomElement connexions = doc.documentElement();
        for (QDomElement my = connexions.firstChildElement("my"); !my.isNull(); my = my.nextSiblingElement("my")) {
            QString type = my.attribute("type");
            QString name = my.attribute("name");
            qDebug()<<__FILE__<<__LINE__<<type<<name;
            jack_port_t *src = NULL;
            if (type == "in_l") {
                in *c = (in*)getChannel(IN, name);
                src = c->in_l;
            }
            if (type == "in_r") {
                in *c = (in*)getChannel(IN, name);
                src = c->in_r;
            }
            if (type == "out_l") {
                out *c = (out*)getChannel(OUT, name);
                src = c->out_l;
            }
            if (type == "out_r") {
                out *c = (out*)getChannel(OUT, name);
                src = c->out_r;
            }
            if (type == "pre_l") {
                pre *c = (pre*)getChannel(PRE, name);
                src = c->out_l;
            }
            if (type == "pre_r") {
                pre *c = (pre*)getChannel(PRE, name);
                src = c->out_l;
            }
            if (type == "post_l") {
                post *c = (post*)getChannel(POST, name);
                src = c->out_l;
            }
            if (type == "post_r") {
                post *c = (post*)getChannel(POST, name);
                src = c->out_r;
            }
            if (type == "return_l") {
                post *c = (post*)getChannel(POST, name);
                src = c->in_l;
            }
            if (type == "return_r") {
                post *c = (post*)getChannel(POST, name);
                src = c->in_l;
            }
            if (type == "sub_l") {
                sub *c = (sub*)getChannel(SUB, name);
                src = c->out_l;
            }
            if (type == "sub_r") {
                sub *c = (sub*)getChannel(SUB, name);
                src = c->out_r;
            }
            for (QDomElement to = my.firstChildElement("to"); !to.isNull(); to = to.nextSiblingElement("to")) {
                QString name = to.attribute("name");
//                jack_port_t *dst = jack_port_by_name(client, name.toStdString().c_str());
                if (jack_port_flags(src) & JackPortIsOutput) {
                    jack_connect(client, jack_port_name(src), name.toStdString().c_str());
                } else {
                    jack_connect(client, name.toStdString().c_str(), jack_port_name(src));
                }
            }
        }
//midi
//int snd_seq_connect_from(snd_seq_t *seq, int my_port, int src_client, int src_port);
//int snd_seq_connect_to(snd_seq_t *seq, int my_port, int dest_client, int dest_port);
//int snd_seq_get_port_info(snd_seq_t *handle, int port, snd_seq_port_info_t *info);
    }
}

}; // LiveMix
