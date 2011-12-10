
macx-g++ {
	LMDEFINES += LADSPA_SUPPORT
	LMDEFINES += JACK_SUPPORT
}


linux-g++ {
	LMDEFINES += JACK_SUPPORT
	LMDEFINES += LADSPA_SUPPORT
	LMDEFINES += LRDF_SUPPORT
}

linux-g++-64 {
	LMDEFINES += JACK_SUPPORT
	LMDEFINES += LADSPA_SUPPORT
	LMDEFINES += LRDF_SUPPORT
}


win32 {
	LNDEFINES += LADSPA_SUPPORT
}

OBJECTS_DIR = objs
UI_DIR = objs
UI_HEADERS_DIR = objs
UI_SOURCES_DIR = objs
MOC_DIR = objs

INCLUDEPATH += . objs /usr/include/lash-1.0 

QT += svg
QT += xml

QMAKE_CXXFLAGS_RELEASE += -g -O2
QMAKE_CXXFLAGS_DEBUG += -Werror -O2

