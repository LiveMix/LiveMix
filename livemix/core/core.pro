#message( --==( Building core )=-- )
include(../features.pri)

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
# precompile_header


DESTDIR = ../libs
INCLUDEPATH += . objs

#PRECOMPILED_HEADER  = precompiled.h

DEFINES += $$LMDEFINES
message( LM defines: $$LMDEFINES )

# precompiled.h \

HEADERS += \
		\
		backend.h \
		Effects.h \
		guiserver_interface.h \
		LadspaFX.h \
		db.h \


SOURCES += \
		backend.cpp \
		effects.cpp \
		ladspa_fx.cpp \
		db.cpp \
		