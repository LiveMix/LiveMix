#message( --==( Building core )=-- )
include(../features.pri)

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
# precompile_header

DESTDIR = ../libs

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
		channels.h \
		qlash.h \


SOURCES += \
		backend.cpp \
		effects.cpp \
		ladspa_fx.cpp \
		db.cpp \
		channels.cpp \
		qlash.cpp \
