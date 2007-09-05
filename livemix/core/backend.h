/*
    Copyright 2005 - 2007 Arnold Krille <arnold@arnoldarts.de>
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

#ifndef BACKEND_INTERFACE_H
#define BACKEND_INTERFACE_H

#include "guiserver_interface.h"
#include "LadspaFX.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtCore/QDebug>

#include <jack/jack.h>

class QDomElement;
class QDomDocument;

namespace JackMix
{

#define MAIN "main"
#define MONO "mono"
#define PLF "plf/alf"

class LadspaFXProperties;

class effect
{
public:
    effect(LadspaFX *p_fx, jack_nframes_t p_nframes);
    ~effect();

// wrorking data:
    LadspaFX *fx;
    LadspaFXProperties *gui;
// jack_default_audio_sample_t* eff_l;
// jack_default_audio_sample_t* eff_r;
    float m_fCpuUse; // part
    uint m_iCount; // not init => rand !
};
class channel
{
public:
    channel(QString p_name, bool p_stereo, jack_nframes_t p_nframes);
    ~channel();

    QString name;
    QString display_name;
    bool mute;
    bool stereo;
    QList<effect*> effects;
// wrorking data:
    jack_default_audio_sample_t peak_l;
    jack_default_audio_sample_t peak_r;
};
class in : public channel
{
public:
    in(QString p_name, bool p_stereo, jack_nframes_t p_nframes, jack_port_t* l, jack_port_t* r);
    ~in();

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
    ~out();

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
    ~pre();

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
    ~post();

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
    ~sub();

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


int process( ::jack_nframes_t, void* );

/**
 * @brief Abstract interface for backends
 *
 * A backend has to implement this functions...
 */
class Backend : public QObject
{
    friend int process( ::jack_nframes_t, void* );

    Q_OBJECT
public:
    enum ChannelType {IN, OUT, PRE, POST, SUB};

    ~Backend();

    static Backend* instance();
    static void init(GuiServer_Interface* p_pGui);

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

    /**
     * @brief Get the peek of the named out node.
     */
    float getOutPeak( QString, bool left );
    /**
     * @brief Get the peek of the named in node.
     */
    float getInPeak( QString, bool left );
    /**
     * @brief Get the peek of the named out node.
     */
    float getPrePeak( QString, bool left );
    /**
     * @brief Get the peek of the named out node.
     */
    float getPostPeak( QString, bool left );
    /**
     * @brief Get the peek of the named out node.
     */
    float getSubPeak( QString, bool left );

    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addInput( QString, bool );
    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addPre( QString, bool );
    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addPost( QString, bool, bool );
    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addSub( QString, bool );
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
    bool removeInput( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removePre( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removePost( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool removeSub( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const struct out* getOutput( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const struct in* getInput( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const struct pre* getPre( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const struct post* getPost( QString );
    /**
     * @brief Remove a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    const struct sub* getSub( QString );


    // should be in mixingmatrix !
    QWidget* effect_start;

    enum Backend::ChannelType selectType;
    QString selectChannel;
    QList<effect*> visibleEffect;

public slots:
    /**
     * @brief Set the volume of the named node.
     */
    void setInVolume( QString, float );
    void setInGain( QString, float );
    void setInBal( QString, float );
    void setInMute( QString, bool );
    void setInPlf( QString, bool );
    void setInMain( QString, bool );
    void setInPreVolume( QString, QString, float );
    void setInPostVolume( QString, QString, float );
    void setInSub( QString, QString, bool );
    effect* addInEffect( QString, LadspaFX* fx );
    void removeInEffect( QString, effect* );
    QList<effect*>* getInEffects( QString );
//  void setInEffectMute( QString, int, bool );
//  void setInEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setOutVolume( QString, float );
    void setOutBal( QString, float );
    void setOutMute( QString, bool );
    void setOutAlf( QString, bool );
    effect* addOutEffect( QString, LadspaFX* fx );
    void removeOutEffect( QString, effect* );
    QList<effect*>* getOutEffects( QString );
//  void setOutEffectMute( QString, int, bool );
//  void setOutEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setPreVolume( QString, float );
    void setPreBal( QString, float );
    void setPreMute( QString, bool );
    void setPreAlf( QString, bool );
    effect* addPreEffect( QString, LadspaFX* fx );
    void removePreEffect( QString, effect* );
    QList<effect*>* getPreEffects( QString );
//  void setPreEffectMute( QString, int, bool );
//  void setPreEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setPostPreVolume( QString, float );
    void setPostPostVolume( QString, float );
    void setPostBal( QString, float );
    void setPostMute( QString, bool );
    void setPostPlf( QString, bool );
    void setPostAlf( QString, bool );
    void setPostMain( QString, bool );
    void setPostSub( QString, QString, bool );
    effect* addPostEffect( QString, LadspaFX* fx );
    void removePostEffect( QString, effect* );
    QList<effect*>* getPostEffects( QString );
//  void setPostEffectMute( QString, int, bool );
//  void setPostEffectAttribute( QString, int, QString, float );

    /**
     * @brief Set the volume of the named node.
     */
    void setSubVolume( QString, float );
    void setSubBal( QString, float );
    void setSubMute( QString, bool );
    void setSubAlf( QString, bool );
    void setSubMain( QString, bool );
    effect* addSubEffect( QString, LadspaFX* fx );
    void removeSubEffect( QString, effect* );
    QList<effect*>* getSubEffects( QString );
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
    ::jack_client_t *client;

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

    Backend()
    {};
    Backend( GuiServer_Interface* );

    /**
     * @brief Add a channel and return true on success.
     *
     * If the actual backend doesn't support adding and removing, thats
     * okay. Just return false..
     */
    bool addOutput( QString, bool );
    bool removeOutput( QString );

    void prossesLadspaFX(effect* pFX, float* left_channel, float* right_channel, uint nframes, bool bCalculatePk);

    bool addOutput( jack_port_t** l, jack_port_t** r, QString name, QString prefix, bool stereo );
    bool addInput( jack_port_t** l, jack_port_t** r, QString name, QString prefix, bool stereo );
    bool removeInput( jack_port_t* l, jack_port_t* r, QString name, QString prefix, bool stereo );
    bool removeOutput( jack_port_t* l, jack_port_t* r, QString name, QString prefix, bool stereo );

};
};

#endif // BACKEND_INTERFACE_H

