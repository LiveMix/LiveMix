include(features.pri)

TEMPLATE = subdirs
SUBDIRS = core widget ladspafx gui mainapp

# installs the executable
target.path = $$prefix/bin
target.files = livemix

# icon
icon.path = $$prefix/share/pixmaps
icon.files = dist/livemix.xpm dist/livemix-16.xpm

# desktop
desktop.path = $$prefix/share/applications
desktop.files = dist/livemix.desktop

# sample
sample.path = $$prefix/share/livemix/sample
sample.files = default.lm

# mime
#mime.path = $$prefix/lib/mime/packages
#mime.files = dist/livemix.mime

# mime2
#mime2.path = $$prefix/share/mime/packages
#mime2.files = dist/livemix-mime.xml


#INSTALLS += target data icon desktop sample
INSTALLS += target icon desktop sample
#mime mime2


