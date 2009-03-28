TARGET = formatting-astyle
HEADERS =   astyle.h \
  astyleplugin.h \
  astyledialogimpl.h
SOURCES =   ASBeautifier.cpp \
  ASEnhancer.cpp \
  ASFormatter.cpp \
  ASResource.cpp \
  astyle_main.cpp \
  astyleplugin.cpp \
  astyledialogimpl.cpp
TRANSLATIONS = translations/AStyle_Russian.ts translations/AStyle_French.ts translations/AStyle_German.ts
FORMS =   astyle.ui

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
