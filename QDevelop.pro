CONFIG += qt uitools warn_on release
DESTDIR += bin
FORMS += ui/about.ui \
 ui/addexistant.ui \
 ui/addnew.ui \
 ui/addnewclass.ui \
 ui/addnewclassmethod.ui \
 ui/addnewclassvariable.ui \
 ui/addscope.ui \
 ui/addsetget.ui \
 ui/exechoice.ui \
 ui/findfiles.ui \
 ui/findwidget.ui \
 ui/gotoline.ui \
 ui/main.ui \
 ui/newimplementation.ui \
 ui/newproject.ui \
 ui/newvariable.ui \
 ui/openfile.ui \
 ui/options.ui \
 ui/parameters.ui \
 ui/projectpropertie.ui \
 ui/replacewidget.ui \
 ui/shortcuts.ui \
 ui/subclassing.ui \
 ui/toolsControl.ui \
 ui/warning.ui \
 ui/breakpointcondition.ui \
 ui/registers.ui
HEADERS += src/InitCompletion.h \
 src/QIComplete/parse.h \
 src/QIComplete/readtags.h \
 src/QIComplete/tree.h \
 src/addexistantimpl.h \
 src/addnewclassimpl.h \
 src/addnewclassmethodimpl.h \
 src/addnewclassvariableimpl.h \
 src/addnewimpl.h \
 src/addscopeimpl.h \
 src/addsetgetimpl.h \
 src/assistant.h \
 src/build.h \
 src/cpphighlighter.h \
 src/debug.h \
 src/designer.h \
 src/editor.h \
 src/findfileimpl.h \
 src/lineedit.h \
 src/linenumbers.h \
 src/logbuild.h \
 src/mainimpl.h \
 src/misc.h \
 src/newprojectimpl.h \
 src/openfileimpl.h \
 src/optionsimpl.h \
 src/parametersimpl.h \
 src/pluginsinterfaces.h \
 src/projectmanager.h \
 src/projectpropertieimpl.h \
 src/selectionborder.h \
 src/shortcutsimpl.h \
 src/stackimpl.h \
 src/subclassingimpl.h \
 src/tabwidget.h \
 src/textEdit.h \
 src/toolscontrolimpl.h \
 src/treeclasses.h \
 src/treeproject.h \
 src/registersimpl.h \
 src/Qsci/Accessor.h \
 src/Qsci/AutoComplete.h \
 src/Qsci/CallTip.h \
 src/Qsci/CellBuffer.h \
 src/Qsci/CharacterSet.h \
 src/Qsci/CharClassify.h \
 src/Qsci/ContractionState.h \
 src/Qsci/Decoration.h \
 src/Qsci/DocumentAccessor.h \
 src/Qsci/Document.h \
 src/Qsci/Editor.h \
 src/Qsci/Indicator.h \
 src/Qsci/KeyMap.h \
 src/Qsci/KeyWords.h \
 src/Qsci/LineMarker.h \
 src/Qsci/ListBoxQt.h \
 src/Qsci/Partitioning.h \
 src/Qsci/Platform.h \
 src/Qsci/PositionCache.h \
 src/Qsci/PropSet.h \
 src/Qsci/qsciabstractapis.h \
 src/Qsci/qsciapis.h \
 src/Qsci/qscicommand.h \
 src/Qsci/qscicommandset.h \
 src/Qsci/qscidocument.h \
 src/Qsci/qsciglobal.h \
 src/Qsci/qscilexerbash.h \
 src/Qsci/qscilexercmake.h \
 src/Qsci/qscilexercpp.h \
 src/Qsci/qscilexerdiff.h \
 src/Qsci/qscilexer.h \
 src/Qsci/qscilexermakefile.h \
 src/Qsci/qscilexersql.h \
 src/Qsci/qscimacro.h \
 src/Qsci/qsciprinter.h \
 src/Qsci/qsciscintillabase.h \
 src/Qsci/qsciscintilla.h \
 src/Qsci/RESearch.h \
 src/Qsci/RunStyles.h \
 src/Qsci/SciClasses.h \
 src/Qsci/SciLexer.h \
 src/Qsci/ScintillaBase.h \
 src/Qsci/Scintilla.h \
 src/Qsci/ScintillaQt.h \
 src/Qsci/ScintillaWidget.h \
 src/Qsci/SplitVector.h \
 src/Qsci/SString.h \
 src/Qsci/StyleContext.h \
 src/Qsci/Style.h \
 src/Qsci/SVector.h \
 src/Qsci/UniConversion.h \
 src/Qsci/ViewStyle.h \
 src/Qsci/WindowAccessor.h \
 src/Qsci/XPM.h \
 include/iproject.h \
 include/imainwindow.h \
 include/ieditor.h
INCLUDEPATH += . src src/ui include
MOC_DIR += build/moc
QT += core gui network sql
RCC_DIR += build/rcc
RC_FILE += QDevelop.rc
RESOURCES += resources/resources.qrc
SOURCES += src/InitCompletion.cpp \
 src/QIComplete/parse.cpp \
 src/QIComplete/readtags.cpp \
 src/QIComplete/tree.cpp \
 src/addexistantimpl.cpp \
 src/addnewclassimpl.cpp \
 src/addnewclassmethodimpl.cpp \
 src/addnewclassvariableimpl.cpp \
 src/addnewimpl.cpp \
 src/addscopeimpl.cpp \
 src/addsetgetimpl.cpp \
 src/assistant.cpp \
 src/build.cpp \
 src/cpphighlighter.cpp \
 src/debug.cpp \
 src/designer.cpp \
 src/editor.cpp \
 src/findfileimpl.cpp \
 src/lineedit.cpp \
 src/linenumbers.cpp \
 src/logbuild.cpp \
 src/main.cpp \
 src/mainimpl.cpp \
 src/misc.cpp \
 src/newprojectimpl.cpp \
 src/openfileimpl.cpp \
 src/optionsimpl.cpp \
 src/parametersimpl.cpp \
 src/projectmanager.cpp \
 src/projectpropertieimpl.cpp \
 src/selectionborder.cpp \
 src/shortcutsimpl.cpp \
 src/stackimpl.cpp \
 src/subclassingimpl.cpp \
 src/tabwidget.cpp \
 src/textEdit.cpp \
 src/toolscontrolimpl.cpp \
 src/treeclasses.cpp \
 src/treeproject.cpp \
 src/registersimpl.cpp \
 src/Qsci/src/AutoComplete.cpp \
 src/Qsci/src/CallTip.cpp \
 src/Qsci/src/CellBuffer.cpp \
 src/Qsci/src/CharClassify.cpp \
 src/Qsci/src/ContractionState.cpp \
 src/Qsci/src/Decoration.cpp \
 src/Qsci/src/DocumentAccessor.cpp \
 src/Qsci/src/Document.cpp \
 src/Qsci/src/Editor.cpp \
 src/Qsci/src/Indicator.cpp \
 src/Qsci/src/KeyMap.cpp \
 src/Qsci/src/KeyWords.cpp \
 src/Qsci/src/LexBash.cpp \
 src/Qsci/src/LexCmake.cpp \
 src/Qsci/src/LexCPP.cpp \
 src/Qsci/src/LexInno.cpp \
 src/Qsci/src/LexOthers.cpp \
 src/Qsci/src/LexSQL.cpp \
 src/Qsci/src/LineMarker.cpp \
 src/Qsci/src/PositionCache.cpp \
 src/Qsci/src/PropSet.cpp \
 src/Qsci/src/RESearch.cpp \
 src/Qsci/src/RunStyles.cpp \
 src/Qsci/src/ScintillaBase.cpp \
 src/Qsci/src/StyleContext.cpp \
 src/Qsci/src/Style.cpp \
 src/Qsci/src/UniConversion.cpp \
 src/Qsci/src/ViewStyle.cpp \
 src/Qsci/src/WindowAccessor.cpp \
 src/Qsci/src/XPM.cpp \
 src/Qsci/Qt4/ListBoxQt.cpp \
 src/Qsci/Qt4/PlatQt.cpp \
 src/Qsci/Qt4/qsciabstractapis.cpp \
 src/Qsci/Qt4/qsciapis.cpp \
 src/Qsci/Qt4/qscicommand.cpp \
 src/Qsci/Qt4/qscicommandset.cpp \
 src/Qsci/Qt4/qscidocument.cpp \
 src/Qsci/Qt4/qscilexerbash.cpp \
 src/Qsci/Qt4/qscilexercmake.cpp \
 src/Qsci/Qt4/qscilexer.cpp \
 src/Qsci/Qt4/qscilexercpp.cpp \
 src/Qsci/Qt4/qscilexerdiff.cpp \
 src/Qsci/Qt4/qscilexermakefile.cpp \
 src/Qsci/Qt4/qscilexersql.cpp \
 src/Qsci/Qt4/qscimacro.cpp \
 src/Qsci/Qt4/qsciprinter.cpp \
 src/Qsci/Qt4/qsciscintillabase.cpp \
 src/Qsci/Qt4/qsciscintilla.cpp \
 src/Qsci/Qt4/SciClasses.cpp \
 src/Qsci/Qt4/ScintillaQt.cpp
TEMPLATE = app
TRANSLATIONS += resources/translations/QDevelop_Chinese.ts \
 resources/translations/QDevelop_Czech.ts \
 resources/translations/QDevelop_Dutch.ts \
 resources/translations/QDevelop_French.ts \
 resources/translations/QDevelop_German.ts \
 resources/translations/QDevelop_Italian.ts \
 resources/translations/QDevelop_Polish.ts \
 resources/translations/QDevelop_Portuguese.ts \
 resources/translations/QDevelop_Russian.ts \
 resources/translations/QDevelop_Spanish.ts \
 resources/translations/QDevelop_Turkish.ts \
 resources/translations/QDevelop_Ukrainian.ts \
 resources/translations/QDevelop_Hungarian.ts \
 resources/translations/QDevelop_Japanese.ts \
 resources/translations/QDevelop_Vietnamese.ts
UI_DIR += build/ui
macx {
 TARGET =  QDevelop
 ICON +=  resources/images/qdevelop.icns
 OBJECTS_DIR +=  build/o/mac
}
unix {
 TARGET =  qdevelop
 OBJECTS_DIR +=  build/o/unix
 target.path +=  /usr/bin/
 INSTALLS +=  target
}
win32 {
 TARGET =  QDevelop
 OBJECTS_DIR +=  build/o/win32
 CONFIG -=  debug_and_release
}
!exists(resources/translations/QDevelop_Russian.qm) { \
error(Please run \"lrelease QDevelop.pro\" before building the project)
