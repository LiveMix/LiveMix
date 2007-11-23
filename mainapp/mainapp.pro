#message( --==( Building livemix )=-- )
include(../features.pri)

#TEMPLATE = lib
TARGET = livemix
CONFIG += qt warn_on thread
# precompile_header

LIBS += ../libs/libgui.a ../libs/libwidget.a ../libs/libladspafx.a ../libs/libbackend.a

PRE_TARGETDEPS = ../libs/libgui.a ../libs/libwidget.a ../libs/libladspafx.a ../libs/libbackend.a

DESTDIR = ..
INCLUDEPATH += ../backend ../gui ../widget

contains(LMDEFINES, LRDF_SUPPORT ){
	LIBS += -llrdf
}

contains(LMDEFINES, JACK_SUPPORT ) {
	LIBS += -ljack
}

LIBS += -llash

#PRECOMPILED_HEADER  = precompiled.h

DEFINES += $$LMDEFINES
message( LM defines: $$LMDEFINES )

TRANSLATIONS = \
	../i18n/livemix_fr.ts

RESOURCES = ../livemix.qrc

HEADERS += \
		\
		mainwindow.h \


SOURCES += \
		main.cpp \
		mainwindow.cpp \

