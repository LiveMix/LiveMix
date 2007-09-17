
macx-g++ {
	LMDEFINES += LADSPA_SUPPORT
	LMDEFINES += JACK_SUPPORT
}


linux-g++ {
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

QT += svg
QT += xml

QMAKE_CXXFLAGS_RELEASE += -fpic -O2
QMAKE_CXXFLAGS_DEBUG += -Wall -Werror -O2 -fpic



