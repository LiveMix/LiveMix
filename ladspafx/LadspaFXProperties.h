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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef LADSPA_FX_PROPERTIES_H
#define LADSPA_FX_PROPERTIES_H


#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

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
    LadspaFXProperties(QWidget* parent, effect *nLadspaFX);
    ~LadspaFXProperties();

    void updateControls();

    void showEvent ( QShowEvent *ev );
    void closeEvent( QCloseEvent *ev );

    static LadspaFX* getFXSelector(LadspaFX* oldFx);
    ToggleButton* getActivateButton() { return m_pActivateBtn; };
    void setFaderHeight(int p_iHeight);
    
public slots:
    void faderChanged(Volume* ref);
    void toggleChanged(ToggleButton* ref);
//  void selectFXBtnClicked();
    void activateBtnClicked();
    void removeBtnClicked();
    void updateOutputControls();
    void leftBtnClicked();
    void rightBtnClicked();

signals:
    void removeClicked(LadspaFXProperties*, effect*);
    void leftClicked(LadspaFXProperties*, effect*);
    void rightClicked(LadspaFXProperties*, effect*);

private:
    effect* m_nLadspaFX;

    QLabel *m_pNameLbl;

    QList<QWidget*> m_pInputControlFaders;
    QList<FaderName*> m_pInputControlNames;
    QList<LCDDisplay*> m_pInputControlLabel;

    QList<Fader*> m_pOutputControlFaders;
    QList<FaderName*> m_pOutputControlNames;
    QList<LCDDisplay*> m_pOutputControlLabel;

    QScrollArea* m_pScrollArea;
    QWidget *m_pFrame;

//  QPushButton *m_pSelectFXBtn;
    ToggleButton *m_pActivateBtn;
    Button *m_pRemoveBtn;
    Button *m_pLeftBtn;
    Button *m_pRightBtn;

    QTimer *m_pTimer;

	int m_iFaderHeight;
};

}
; //LiveMix

#endif
