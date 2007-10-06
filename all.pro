include(features.pri)

TEMPLATE = subdirs
SUBDIRS = backend widget ladspafx gui mainapp

# installs the executable
target.path = $$prefix/bin
target.files = livemix

icon.path = $$prefix/share/pixmaps
icon.files = dist/livemix.xpm
#dist/livemix-16.xpm

desktop.path = $$prefix/share/applications
desktop.files = dist/livemix.desktop

sample.path = $$prefix/share/livemix/sample
sample.files = default.lm

mimelnk.path = $$prefix/share/mimelnk/application
mimelnk.files = dist/x-livemix.desktop

#application-registry.path = $$prefix/share/application-registry
#application-registry.files = dist/livemix.applications

mime.path = $$prefix/share/mime-info
mime.files = dist/livemix.mime dist/livemix.keys

# mime2
#mime2.path = $$prefix/share/mime/packages
#mime2.files = dist/livemix-mime.xml


INSTALLS += target icon sample desktop mimelnk mime
#mime2 data mime application-registry

