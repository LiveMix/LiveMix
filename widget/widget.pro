#message( --==( Building widget )=-- )
include(../features.pri)

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
# precompile_header

DESTDIR = ../libs
INCLUDEPATH += ../backend

#PRECOMPILED_HEADER  = precompiled.h

DEFINES += $$LMDEFINES
message( LM defines: $$LMDEFINES )

HEADERS += \
		\
		ClickableLabel.h \
		CpuLoadWidget.h \
		Fader.h \
		PixmapWidget.h \
		FaderName.h \
		LCD.h \
		Rotary.h \
		Button.h \
		Action.h \
		GetKeyField.h \


SOURCES += \
		ClickableLabel.cpp \
		CpuLoadWidget.cpp \
		Fader.cpp \
		PixmapWidget.cpp \
		FaderName.cpp \
		LCD.cpp \
		Rotary.cpp \
		Button.cpp \
		Action.cpp \
		GetKeyField.cpp \
		