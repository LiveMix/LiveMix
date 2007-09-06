#message( --==( Building gui )=-- )
include(../features.pri)

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
# precompile_header


DESTDIR = ../libs
INCLUDEPATH += objs ../core ../widget ../ladspafx ../ladspafx/objs

#PRECOMPILED_HEADER  = precompiled.h

DEFINES += $$LMDEFINES
message( LM defines: $$LMDEFINES )

HEADERS += \
		\
		channelselector.h \
		graphicalguiserver.h \
		mixingmatrix.h \


SOURCES += \
		channelselector.cpp \
		graphicalguiserver.cpp \
		mixingmatrix.cpp \
