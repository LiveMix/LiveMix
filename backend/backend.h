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

#ifndef BACKEND_INTERFACE_H
#define BACKEND_INTERFACE_H

#include "guiserver_interface.h"
#include "channels.h"
#include "types.h"

#ifdef LADSPA_SUPPORT
#include "ladspa_fx.h"
#endif

#include <QList>
#include <QHash>
//#include <QDebug>
#include <QMutex>

#include <jack/jack.h>
#include <alsa/asoundlib.h>

class QDomElement;
class QDomDocument;

namespace LiveMix
{

int process(::jack_nframes_t, void*);

/**
 * @brief Abstract interface for backends
 *
 * A backend has to implement this functions...
 */
class Backend : public QObject
{
    friend int process(::jack_nframes_t, void*);

    Q_OBJECT
public:
    ~Backend();

    static Backend* instance();
    static void init(GuiServer_Interface* p_pGui);

    void sendMidiEvent(unsigned char p_iChannel, unsigned int p_iController, signed int p_iValue);
    bool hasMidiEvent();
    snd_seq_event_t* readMidiEvent();

    void run(bool run);
    uint getSampleRate();
    float getCPULoad();
    uint getBufferSize();
    float getProcessTime();
    float getMaxProcessTime();

    /**
     * @brief Return the current list of input channels.
     */
    const QList<QString>& inchannels();
    /**
     * @brief Return the current list of output channels.
     */
    const QList<QString>& outchannels();
    const QList<QString>& prechannels();
    const QList<QString>& postchannels();
    const QList<QString>& subchannels();

    channel* getChannel(ChannelType p_eType, QString p_rName);

    /**
     * @brief Get the peek of the named out node.
     */
    float getOutPeak(QString, bool left);
    /**
     * @brief Get the peek of the named in node.
     */
    float getInPeak(QString, bool left);
    /**
     * @brief Get the peek of the named out node.
     */
    float getPrePeak(QString, bool left);
    /**
     * @brief Get the peek of the named out node.
     */
    float getPostPeak(QString, bool left);
    /**
     * @brief Get the peek of the named out node.
     */
    float getSubPeak(QString, bool left);

    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addInput(QString, bool);
    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addPre(QString, bool);
    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addPost(QString, bool, bool);
    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addSub(QString, bool);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
//  bool removeOutput( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removeInput(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removePre(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removePost(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removeSub(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const out* getOutput(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const in* getInput(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const pre* getPre(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const post* getPost(QString);
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const sub* getSub(QString);

    bool moveEffect(ChannelType p_eType, QString p_rName, effect *p_pEffect, bool p_bLeft);

    void saveConnexions(QString p_rFile);
    void restoreConnexions(QString p_rFile);

public slots:
    /**
     * @brief Set the volume of the named node.
     */
    void setInVolume(QString, float);
    void setInGain(QString, float);
    void setInBal(QString, float);
    void setInMute(QString, bool);
    void setInPfl(QString, bool);
    void setInMain(QString, bool);
    void setInPreVolume(QString, QString, float);
    void setInPostVolume(QString, QString, float);
    void setInSub(QString, QString, bool);
    effect* addInEffect(QString, LadspaFX* fx);
    void removeInEffect(QString, effect*);
    QList<effect*>* getInEffects(QString);
//  void setInEffectMute( QString, int, bool );
//  void setInEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setOutVolume(QString, float);
    void setOutBal(QString, float);
    void setOutMute(QString, bool);
    void setOutAfl(QString, bool);
    effect* addOutEffect(QString, LadspaFX* fx);
    void removeOutEffect(QString, effect*);
    QList<effect*>* getOutEffects(QString);
//  void setOutEffectMute( QString, int, bool );
//  void setOutEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setPreVolume(QString, float);
    void setPreBal(QString, float);
    void setPreMute(QString, bool);
    void setPreAfl(QString, bool);
    effect* addPreEffect(QString, LadspaFX* fx);
    void removePreEffect(QString, effect*);
    QList<effect*>* getPreEffects(QString);
//  void setPreEffectMute( QString, int, bool );
//  void setPreEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setPostPreVolume(QString, float);
    void setPostPostVolume(QString, float);
    void setPostBal(QString, float);
    void setPostMute(QString, bool);
    void setPostPfl(QString, bool);
    void setPostAfl(QString, bool);
    void setPostMain(QString, bool);
    void setPostSub(QString, QString, bool);
    effect* addPostEffect(QString, LadspaFX* fx);
    void removePostEffect(QString, effect*);
    QList<effect*>* getPostEffects(QString);
//  void setPostEffectMute( QString, int, bool );
//  void setPostEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setSubVolume(QString, float);
    void setSubBal(QString, float);
    void setSubMute(QString, bool);
    void setSubAfl(QString, bool);
    void setSubMain(QString, bool);
    effect* addSubEffect(QString, LadspaFX* fx);
    void removeSubEffect(QString, effect*);
    QList<effect*>* getSubEffects(QString);
//  void setSubEffectMute( QString, int, bool );
//  void setSubEffectAttribute( QString, int, QString, float );

signals:
    // not all due performances !
    // on peak calculation
    void processed();
//  void xrun();

private:
    GuiServer_Interface* gui;
    int count;
    bool _run;
    QMutex _lock;

    snd_seq_t* seq;
    int m_iClient;
    int m_iPort;
    int m_iMidi;

    jack_client_t *client;

//    jack_port_t *midi_in;
//    jack_port_t *midi_out;

    QHash<QString, in*> ins;
    QHash<QString, out*> outs;
    QHash<QString, pre*> pres;
    QHash<QString, post*> posts;
    QHash<QString, sub*> subs;

    QStringList ins_order;
    QStringList outs_order;
    QStringList pres_order;
    QStringList posts_order;
    QStringList subs_order;

    Backend() {};
    Backend(GuiServer_Interface*);

    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addOutput(QString, bool);
    bool removeOutput(QString);

    void prossesLadspaFX(effect* pFX, float* left_channel, float* right_channel, uint nframes, bool bCalculatePk);

    bool addOutput(jack_port_t** l, jack_port_t** r, QString name, QString prefix, bool stereo);
    bool addInput(jack_port_t** l, jack_port_t** r, QString name, QString prefix, bool stereo);
    bool removeInput(jack_port_t* l, jack_port_t* r, QString name, QString prefix, bool stereo);
    bool removeOutput(jack_port_t* l, jack_port_t* r, QString name, QString prefix, bool stereo);

};

}
; // LiveMix

#endif // BACKEND_INTERFACE_H

