#message( --==( Building gui )=-- )
include(../features.pri)

TEMPLATE = lib
CONFIG += qt warn_on thread staticlib
# precompile_header

DESTDIR = ../libs
INCLUDEPATH += ../core ../widget ../ladspafx ../ladspafx/objs

#PRECOMPILED_HEADER  = precompiled.h

DEFINES += $$LMDEFINES
message( LM defines: $$LMDEFINES )

HEADERS += \
		\
		channelselector.h \
		graphicalguiserver.h \
		mixingmatrix.h \
		AssigneToPannel.h \


SOURCES += \
		channelselector.cpp \
		graphicalguiserver.cpp \
		mixingmatrix.cpp \
		AssigneToPannel.cpp \
