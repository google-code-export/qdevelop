# qdevelop CMakeLists.txt
# 
# To build qdevelop, you will need cmake 2.4.3 (should also work with 2.4.0 and above
# but not tested). To build the application run this commands:
# 
#	mkdir cbuild
#	cd cbuild
#	cmake ../
#	make
# 
# Please indent this file using tabs, and set the tabs to 8 spaces :)
# 
# If you don't konw cmake, here are some links for you:
# 	http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
# 	http://qtnode.net/wiki/Qt_with_cmake#cmake_for_Qt_4

PROJECT(QDevelop)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)
SET(QDEV	"qdevelop"	CACHE STRING "Name of the executeable")

# verbose - off
# SET( CMAKE_VERBOSE_MAKEFILE  ON )

# do optimizations on GCC based compilers
# if you want to speed up compilations, set this to -O0,
# which reduces compilation time from 2:30 to 1:30 on my system
SET( OPTIMIZATION_FLAGS "-O2")

## TODO mmmm....
# CHECK_C_COMPILER_FLAG( ${OPTIMIZATION_FLAGS} HAS_OPTIMIZATION_FLAGS )
# if (HAS_OPTIMIZATION_FLAGS)
# 	ADD_DEFINITIONS( ${OPTIMIZATION_FLAGS} )
# endif (HAS_OPTIMIZATION_FLAGS)

ADD_DEFINITIONS( ${OPTIMIZATION_FLAGS} )

# input files
SET( qdevelop_UIS
	ui/about.ui 
	ui/addexistant.ui
	ui/addnew.ui
	ui/addnewclass.ui
	ui/addnewclassmethod.ui
	ui/addnewclassvariable.ui
	ui/addsetget.ui
	ui/addscope.ui
	ui/breakpointcondition.ui
	ui/exechoice.ui
	ui/findfiles.ui
	ui/findwidget.ui
	ui/gotoline.ui
	ui/main.ui
	ui/newimplementation.ui
	ui/newproject.ui
	ui/newvariable.ui
	ui/options.ui
	ui/openfile.ui
	ui/parameters.ui
	ui/projectpropertie.ui
	ui/registers.ui
	ui/replacewidget.ui
	ui/shortcuts.ui
	ui/subclassing.ui
	ui/toolsControl.ui
	ui/warning.ui
)

SET( qdevelop_SRCS
	src/InitCompletion.cpp
	src/QIComplete/parse.cpp
	src/QIComplete/readtags.cpp
	src/QIComplete/tree.cpp
	src/addexistantimpl.cpp
	src/addnewimpl.cpp
	src/addnewclassimpl.cpp
	src/addscopeimpl.cpp
	src/addsetgetimpl.cpp 
	src/addnewclassmethodimpl.cpp
	src/addnewclassvariableimpl.cpp
	src/assistant.cpp
	src/build.cpp
	src/cpphighlighter.cpp
	src/debug.cpp
	src/designer.cpp
	src/editor.cpp
	src/findfileimpl.cpp
	src/lineedit.cpp
	src/logbuild.cpp
	src/linenumbers.cpp
	src/main.cpp
	src/mainimpl.cpp
	src/misc.cpp
	src/newprojectimpl.cpp
	src/optionsimpl.cpp
	src/openfileimpl.cpp 
	src/parametersimpl.cpp
	src/projectmanager.cpp
	src/projectpropertieimpl.cpp
	src/registersimpl.cpp
	src/selectionborder.cpp
	src/shortcutsimpl.cpp
	src/stackimpl.cpp
	src/subclassingimpl.cpp
	src/tabwidget.cpp
	src/textEdit.cpp
	src/treeclasses.cpp
	src/treeproject.cpp
	src/toolscontrolimpl.cpp
	
	src/Qsci/src/AutoComplete.cpp
	src/Qsci/src/CallTip.cpp
	src/Qsci/src/CellBuffer.cpp
	src/Qsci/src/CharClassify.cpp
	src/Qsci/src/ContractionState.cpp
	src/Qsci/src/Decoration.cpp
	src/Qsci/src/DocumentAccessor.cpp
	src/Qsci/src/Document.cpp
	src/Qsci/src/Editor.cpp
	src/Qsci/src/Indicator.cpp
	src/Qsci/src/KeyMap.cpp
	src/Qsci/src/KeyWords.cpp
	src/Qsci/src/LexBash.cpp
	src/Qsci/src/LexCmake.cpp
	src/Qsci/src/LexCPP.cpp
	src/Qsci/src/LexInno.cpp
	src/Qsci/src/LexOthers.cpp
	src/Qsci/src/LexSQL.cpp
	src/Qsci/src/LineMarker.cpp
	src/Qsci/src/PositionCache.cpp
	src/Qsci/src/PropSet.cpp
	src/Qsci/src/RESearch.cpp
	src/Qsci/src/RunStyles.cpp
	src/Qsci/src/ScintillaBase.cpp
	src/Qsci/src/StyleContext.cpp
	src/Qsci/src/Style.cpp
	src/Qsci/src/UniConversion.cpp
	src/Qsci/src/ViewStyle.cpp
	src/Qsci/src/WindowAccessor.cpp
	src/Qsci/src/XPM.cpp
	src/Qsci/Qt4/ListBoxQt.cpp
	src/Qsci/Qt4/PlatQt.cpp
	src/Qsci/Qt4/qsciabstractapis.cpp
	src/Qsci/Qt4/qsciapis.cpp
	src/Qsci/Qt4/qscicommand.cpp
	src/Qsci/Qt4/qscicommandset.cpp
	src/Qsci/Qt4/qscidocument.cpp
	src/Qsci/Qt4/qscilexerbash.cpp
	src/Qsci/Qt4/qscilexercmake.cpp
	src/Qsci/Qt4/qscilexer.cpp
	src/Qsci/Qt4/qscilexercpp.cpp
	src/Qsci/Qt4/qscilexerdiff.cpp
	src/Qsci/Qt4/qscilexermakefile.cpp
	src/Qsci/Qt4/qscilexersql.cpp
	src/Qsci/Qt4/qscimacro.cpp
	src/Qsci/Qt4/qsciprinter.cpp
	src/Qsci/Qt4/qsciscintillabase.cpp
	src/Qsci/Qt4/qsciscintilla.cpp
	src/Qsci/Qt4/SciClasses.cpp
	src/Qsci/Qt4/ScintillaQt.cpp

)

SET( qdevelop_MOC_HDRS	
	src/InitCompletion.h
	src/addexistantimpl.h
	src/addnewimpl.h
	src/addnewclassimpl.h
	src/addnewclassmethodimpl.h
	src/addnewclassvariableimpl.h
	src/addscopeimpl.h
	src/addsetgetimpl.h
	src/assistant.h
	src/build.h
	src/debug.h
	src/editor.h
	src/findfileimpl.h
	src/linenumbers.h
	src/logbuild.h
	src/mainimpl.h
	src/newprojectimpl.h
	src/optionsimpl.h
	src/openfileimpl.h
	src/parametersimpl.h
	src/projectmanager.h
	src/projectpropertieimpl.h
	src/registersimpl.h
	src/shortcutsimpl.h
	src/stackimpl.h
	src/subclassingimpl.h
	src/tabwidget.h
	src/textEdit.h
	src/treeclasses.h
	src/treeproject.h
	src/toolscontrolimpl.h
	
	src/Qsci/Accessor.h
	src/Qsci/AutoComplete.h
	src/Qsci/CallTip.h
	src/Qsci/CellBuffer.h
	src/Qsci/CharacterSet.h
	src/Qsci/CharClassify.h
	src/Qsci/ContractionState.h
	src/Qsci/Decoration.h
	src/Qsci/DocumentAccessor.h
	src/Qsci/Document.h
	src/Qsci/Editor.h
	src/Qsci/Indicator.h
	src/Qsci/KeyMap.h
	src/Qsci/KeyWords.h
	src/Qsci/LineMarker.h
	src/Qsci/ListBoxQt.h
	src/Qsci/Partitioning.h
	src/Qsci/Platform.h
	src/Qsci/PositionCache.h
	src/Qsci/PropSet.h
	src/Qsci/qsciabstractapis.h
	src/Qsci/qsciapis.h
	src/Qsci/qscicommand.h
	src/Qsci/qscicommandset.h
	src/Qsci/qscidocument.h
	src/Qsci/qsciglobal.h
	src/Qsci/qscilexerbash.h
	src/Qsci/qscilexercmake.h
	src/Qsci/qscilexercpp.h
	src/Qsci/qscilexerdiff.h
	src/Qsci/qscilexer.h
	src/Qsci/qscilexermakefile.h
	src/Qsci/qscilexersql.h
	src/Qsci/qscimacro.h
	src/Qsci/qsciprinter.h
	src/Qsci/qsciscintillabase.h
	src/Qsci/qsciscintilla.h
	src/Qsci/RESearch.h
	src/Qsci/RunStyles.h
	src/Qsci/SciClasses.h
	src/Qsci/SciLexer.h
	src/Qsci/ScintillaBase.h
	src/Qsci/Scintilla.h
	src/Qsci/ScintillaQt.h
	src/Qsci/ScintillaWidget.h
	src/Qsci/SplitVector.h
	src/Qsci/SString.h
	src/Qsci/StyleContext.h
	src/Qsci/Style.h
	src/Qsci/SVector.h
	src/Qsci/UniConversion.h
	src/Qsci/ViewStyle.h
	src/Qsci/WindowAccessor.h
	src/Qsci/XPM.h

)

SET( qdevelop_RCS
	QDevelop.rc
)

SET( qdevelop_RESOURCES
	resources/resources.qrc
)

SET( qdevelop_TS
	resources/translations/QDevelop_Chinese.ts
	resources/translations/QDevelop_Czech.ts
	resources/translations/QDevelop_Dutch.ts
	resources/translations/QDevelop_French.ts
	resources/translations/QDevelop_German.ts
	resources/translations/QDevelop_Hungarian.ts
	resources/translations/QDevelop_Italian.ts
	resources/translations/QDevelop_Japanese.ts
	resources/translations/QDevelop_Polish.ts
	resources/translations/QDevelop_Portuguese.ts
	resources/translations/QDevelop_Russian.ts
	resources/translations/QDevelop_Spanish.ts
	resources/translations/QDevelop_Turkish.ts
	resources/translations/QDevelop_Ukrainian.ts
	resources/translations/QDevelop_Vietnamese.ts

)


FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE(${QT_USE_FILE})

SET( QT_USE_QTASSISTANT )
SET( QT_USE_QTDESIGNER )
SET( QT_USE_QTNETWORK )
SET( QT_USE_QTSQL )
SET( QT_USE_QTXML )
SET( QT_USE_QTUITOOLS )

# qt4 definitinos
QT4_WRAP_UI       ( qdevelop_UIS_H        ${qdevelop_UIS}       )
QT4_WRAP_CPP      ( qdevelop_MOC_SRCS     ${qdevelop_MOC_HDRS}  )
QT4_ADD_RESOURCES ( qdevelop_RESOURCES    ${qdevelop_RESOURCES} )
QT4_AUTOMOC       ( ${qdevelop_SRCS}                            )
QT4_ADD_TRANSLATION ( qdevelop_QM         ${qdevelop_TS}        )

# definitions for the project
ADD_DEFINITIONS(
	-Wall 
	-DQT_NO_DEBUG
	${QT_DEFINITIONS}
)

# include directories
INCLUDE_DIRECTORIES(
	${QT_INCLUDES}
	${QT_INCLUDE_DIR}/QtUiTools
	${CMAKE_BINARY_DIR}
	.
	${QDevelop_SOURCE_DIR}/src
	${QDevelop_SOURCE_DIR}/src/Qsci
)

IF( MINGW )
	# resource compilation for MinGW
	ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qdevelop_ico.o
		COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/QDevelop.rc 
			-o ${CMAKE_CURRENT_BINARY_DIR}/qdevelop_ico.o )
	SET( qdevelop_SRCS ${qdevelop_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/qdevelop_ico.o)
	SET( QT_UITOOLSLIB QtUiTools QtDesigner4  )
ELSE( MINGW )
	SET(qdevelop_SRCS	${qdevelop_SRCS} QDevelop.rc)
	SET(QT_UITOOLSLIB	${QT_QTUITOOLS_LIBRARY} QtDesigner )
ENDIF( MINGW )

# the main build target
ADD_EXECUTABLE( ${QDEV}
	${qdevelop_SRCS} 
	${qdevelop_UIS_H} 
	${qdevelop_MOC_SRCS} 
	${qdevelop_UIS} 
	${qdevelop_RESOURCES} 
	${qdevelop_QM}
)

IF( MINGW )
	SET_TARGET_PROPERTIES( ${QDEV} PROPERTIES LINK_FLAGS "-mwindows" )
ENDIF( MINGW )

TARGET_LINK_LIBRARIES( ${QDEV}
	${QT_UITOOLSLIB} 
	${QT_QTSQL_LIBRARY}
	${QT_LIBRARIES}
	${QT_QTXML_LIBRARY}
	${QT_QTNETWORK_LIBRARY}
)

# Plugins support

IF (AUTOPLUGINS)
	FILE (GLOB PLUGINS ${QDevelop_SOURCE_DIR}/plugins/*)
	FOREACH (PLG ${PLUGINS})
		STRING (REGEX MATCH \\.svn FAIL ${PLG})
		STRING (COMPARE EQUAL "${FAIL}" "" IS_EMPTY)
		IF (IS_EMPTY)
			SET (PLG_HXX "")
			SET (PLG_CXX "")
			SET (PLG_UI "")
			SET (PLG_MOC "")
			SET (PLG_UIC "")
			SET (PLG_TS "")
			SET (PLG_QM "")
		
			STRING (REPLACE "${QDevelop_SOURCE_DIR}/plugins/" "" PLG_NAME ${PLG})
			MESSAGE (STATUS "Preparing plugin ${PLG_NAME} for building")
		
			FILE (GLOB PLG_HXX ${PLG}/*.h)
			FILE (GLOB PLG_CXX ${PLG}/*.cpp)
			FILE (GLOB PLG_UI ${PLG}/*.ui)
			FILE (GLOB PLG_TS ${PLG}/translations/*.ts)
			QT4_ADD_TRANSLATION (PLG_QM ${PLG_TS})
		
			QT4_WRAP_CPP (PLG_MOC ${PLG_HXX})
			QT4_WRAP_UI (PLG_UIC ${PLG_UI})
		
			ADD_LIBRARY (${PLG_NAME} SHARED ${PLG_CXX} ${PLG_MOC} ${PLG_UIC} ${PLG_QM})
			TARGET_LINK_LIBRARIES (${PLG_NAME} ${QT_UITOOLSLIB} 
				${QT_QTSQL_LIBRARY}
				${QT_LIBRARIES}
				${QT_QTXML_LIBRARY}
				${QT_QTNETWORK_LIBRARY})
			SET_TARGET_PROPERTIES (${PLG_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY plugins LIBRARY_OUTPUT_DIRECTORY plugins)
			ADD_DEPENDENCIES (${PLG_NAME} ${QDEV})
		
			IF (WIN32)
				INSTALL (TARGETS ${PLG_NAME} DESTINATION bin/plugins)
				INSTALL (FILES ${PLG_QM} DESTINATION translations/${PLG_NAME})
			ELSE (NOT WIN32)
				INSTALL (TARGETS ${PLG_NAME} DESTINATION lib/qdevelop/plugins)
				INSTALL (FILES ${PLG_QM} DESTINATION lib/qdevelop/translations/${PLG_NAME})
			ENDIF (WIN32)
		ENDIF (IS_EMPTY)
	ENDFOREACH (PLG)
ENDIF (AUTOPLUGINS)

INSTALL (TARGETS ${QDEV} RUNTIME DESTINATION bin)
IF (WIN32)
	INSTALL (FILES ${qdevelop_QM} DESTINATION translations)
ELSE (NOT WIN32)
	INSTALL (FILES ${qdevelop_QM} DESTINATION lib/qdevelop/translations)
ENDIF (WIN32)
