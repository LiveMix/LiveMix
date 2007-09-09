/*
 * Hydrogen
 * Copyright(c) 2002-2007 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * http://www.hydrogen-music.org
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
 *
 */

#ifndef LADSPA_FX_PROPERTIES_H
#define LADSPA_FX_PROPERTIES_H


#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>

#include "Fader.h"
#include "FaderName.h"
#include "LCD.h"
#include "Button.h"

namespace LiveMix
{

class LadspaFXProperties : public QWidget
{
    Q_OBJECT

public:
    LadspaFXProperties(QWidget* parent, struct effect *nLadspaFX);
    ~LadspaFXProperties();

    void updateControls();

    void showEvent ( QShowEvent *ev );
    void closeEvent( QCloseEvent *ev );

    static LadspaFX* getFXSelector(LadspaFX* oldFx);
    ToggleButton* getActivateButton() { return m_pActivateBtn; };
    
public slots:
    void faderChanged(Fader * ref);
    void toggleChanged(ToggleButton* ref);
//  void selectFXBtnClicked();
    void activateBtnClicked();
    void removeBtnClicked();
    void updateOutputControls();

signals:
    void removeClicked(LadspaFXProperties*, struct effect*);

private:
    struct effect* m_nLadspaFX;

    QLabel *m_pNameLbl;

    QList<QWidget*> m_pInputControlFaders;
    QList<FaderName*> m_pInputControlNames;
    QList<LCDDisplay*> m_pInputControlLabel;

    QList<Fader*> m_pOutputControlFaders;
    QList<FaderName*> m_pOutputControlNames;
    QList<LCDDisplay*> m_pOutputControlLabel;

    QScrollArea* m_pScrollArea;
//  QFrame *m_pFrame;
    QWidget *m_pFrame;

//  QPushButton *m_pSelectFXBtn;
    ToggleButton *m_pActivateBtn;
    Button *m_pRemoveBtn;

    QTimer *m_pTimer;
};

}
; //LiveMix

#endif
