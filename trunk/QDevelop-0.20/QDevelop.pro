CONFIG +=   qt \
  release \
  uitools \
  warn_on
DESTDIR =   bin
FORMS =   ui/about.ui \
  ui/addexistant.ui \
  ui/addnew.ui \
  ui/addscope.ui \
  ui/exechoice.ui \
  ui/findfiles.ui \
  ui/findwidget.ui \
  ui/gotoline.ui \
  ui/main.ui \
  ui/newimplementation.ui \
  ui/newproject.ui \
  ui/newvariable.ui \
  ui/options.ui \
  ui/parameters.ui \
  ui/projectpropertie.ui \
  ui/promptreplace.ui \
  ui/replace.ui \
  ui/shortcuts.ui \
  ui/stack.ui \
  ui/subclassing.ui \
  ui/warning.ui
HEADERS =   src/InitCompletion.h \
  src/QIComplete/parse.h \
  src/QIComplete/readtags.h \
  src/QIComplete/tree.h \
  src/addexistantimpl.h \
  src/addnewimpl.h \
  src/addscopeimpl.h \
  src/assistant.h \
  src/build.h \
  src/cpphighlighter.h \
  src/debug.h \
  src/editor.h \
  src/findfileimpl.h \
  src/lineedit.h \
  src/linenumbers.h \
  src/mainimpl.h \
  src/misc.h \
  src/newprojectimpl.h \
  src/optionsimpl.h \
  src/parametersimpl.h \
  src/projectmanager.h \
  src/projectpropertieimpl.h \
  src/promptreplaceimpl.h \
  src/replaceimpl.h \
  src/selectionborder.h \
  src/shortcutsimpl.h \
  src/stackimpl.h \
  src/subclassingimpl.h \
  src/tabwidget.h \
  src/textEdit.h \
  src/treeclasses.h \
  src/treeproject.h
INCLUDEPATH =   . \
  src \
  src/ui
MOC_DIR =   build/moc
QT +=   core \
  gui \
  network
RCC_DIR =   build/rcc
RC_FILE =   QDevelop.rc
RESOURCES =   resources/resources.qrc
SOURCES =   src/InitCompletion.cpp \
  src/QIComplete/parse.cpp \
  src/QIComplete/readtags.cpp \
  src/QIComplete/tree.cpp \
  src/addexistantimpl.cpp \
  src/addnewimpl.cpp \
  src/addscopeimpl.cpp \
  src/assistant.cpp \
  src/build.cpp \
  src/cpphighlighter.cpp \
  src/debug.cpp \
  src/editor.cpp \
  src/findfileimpl.cpp \
  src/lineedit.cpp \
  src/linenumbers.cpp \
  src/main.cpp \
  src/mainimpl.cpp \
  src/misc.cpp \
  src/newprojectimpl.cpp \
  src/optionsimpl.cpp \
  src/parametersimpl.cpp \
  src/projectmanager.cpp \
  src/projectpropertieimpl.cpp \
  src/promptreplaceimpl.cpp \
  src/replaceimpl.cpp \
  src/selectionborder.cpp \
  src/shortcutsimpl.cpp \
  src/stackimpl.cpp \
  src/subclassingimpl.cpp \
  src/tabwidget.cpp \
  src/textEdit.cpp \
  src/treeclasses.cpp \
  src/treeproject.cpp
TARGET =   QDevelop
TEMPLATE =   app
TRANSLATIONS =   resources/translations/QDevelop_Dutch.ts \
  resources/translations/QDevelop_French.ts \
  resources/translations/QDevelop_German.ts \
  resources/translations/QDevelop_Polish.ts \
  resources/translations/QDevelop_Spanish.ts \
  resources/translations/QDevelop_Chinese.ts
UI_DIR =   build/ui
mac {
  OBJECTS_DIR =     build/o/mac
}
unix {
  OBJECTS_DIR =     build/o/unix
}
win32 {
  OBJECTS_DIR =     build/o/win32
}
