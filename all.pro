include(features.pri)

TEMPLATE = subdirs
SUBDIRS = core widget ladspafx gui mainapp

# installs the executable
target.path = $$prefix/bin
target.files = livemix

# data
data.path = $$prefix/bin/data
data.files = data/*

# icon
icon.path = $$prefix/share/pixmaps
icon.files = debian/livemix.xpm debian/livemix-16.xpm

# desktop
desktop.path = $$prefix/share/applications
desktop.files = debian/livemix.desktop

INSTALLS += target data icon desktop

TRANSLATIONS = \
	i18n/livemic.fr.ts
