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

# desktop
desktop.path = $$prefix/share/applications
desktop.files = debian/livemix.desktop

# sample
sample.path = $$prefix/share/livemix/sample
sample.files = default.lm


INSTALLS += target data icon desktop sample

TRANSLATIONS = \
	i18n/livemic.fr.ts
