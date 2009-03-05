TARGET = formatting-astyle
DEPENDPATH =   .
INCLUDEPATH =   . ../../src
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
CONFIG +=   release \
  plugin
FORMS =   astyle.ui
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
