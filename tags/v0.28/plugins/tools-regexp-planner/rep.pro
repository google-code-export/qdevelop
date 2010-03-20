TARGET = rep
SOURCES += replugin.cpp qpjregexpplannerdialog.cpp qpjchecktextedit.cpp qpjvalidindicatorlabel.cpp
HEADERS += replugin.h qpjregexpplannerdialog.h qpjchecktextedit.h qpjvalidindicatorlabel.h
TRANSLATIONS += translations/RePlanner_Russian.ts translations/RePlanner_French.ts translations/RePlanner_German.ts

CONFIG +=   release \
  plugin
DEPENDPATH =   .
INCLUDEPATH =   . ../../src
TEMPLATE =   lib
DESTDIR =   ../../bin/plugins
MOC_DIR =   ../../build/moc
UI_DIR =   ../../build/ui
macx {
  OBJECTS_DIR =     ../../build/o/mac
}
unix {
  OBJECTS_DIR =     ../../build/o/unix
}
win32 {
  OBJECTS_DIR =     ../../build/o/win32
}
