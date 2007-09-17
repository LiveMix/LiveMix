#message( --==( Building ladspafx )=-- )
include(../features.pri)

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
# precompile_header


DESTDIR = ../libs
INCLUDEPATH += ./objs ../widget ../backend

#PRECOMPILED_HEADER  = precompiled.h

DEFINES += $$LMDEFINES
message( LM defines: $$LMDEFINES )

FORMS    = \
	../ladspafx/LadspaFXSelector_UI.ui

HEADERS += \
		\
		LadspaFXProperties.h \
		LadspaFXSelector.h \


SOURCES += \
		LadspaFXProperties.cpp \
		LadspaFXSelector.cpp \
