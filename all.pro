include(features.pri)

TEMPLATE = subdirs
SUBDIRS = core widget ladspafx gui mainapp

# installs the executable
target.path = $$prefix/bin
target.files = livemix

# data
data.path = $$prefix/bin/data
data.files = data/*
INSTALLS += target data

TRANSLATIONS = \
	i18n/livemic.fr.ts
