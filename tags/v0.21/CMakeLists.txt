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
# On my system, qmake takes 3.13 seconds to build, and using cmake 1.40
# The final binary is also a little bigger. We shuold test which build system makes the
# fastest final binary, as this is the only thing important to the user.
# 
# Please indent this file using tabs, and set the tabs to 8 spaces. :)
# 
# If you don't konw cmake, here are some links for you:
# 	http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
# 	http://qtnode.net/wiki/Qt_with_cmake#cmake_for_Qt_4

PROJECT(qdevelop)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

# input files
SET( qdevelop_UIS
	ui/about.ui 
	ui/addexistant.ui
	ui/addnew.ui
	ui/addscope.ui
	ui/exechoice.ui
	ui/findfiles.ui
	ui/findwidget.ui
	ui/gotoline.ui
	ui/main.ui
	ui/newimplementation.ui
	ui/newproject.ui
	ui/newvariable.ui
	ui/options.ui
	ui/parameters.ui
	ui/projectpropertie.ui
	ui/promptreplace.ui
	ui/replace.ui
	ui/shortcuts.ui
	ui/stack.ui
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
	src/addscopeimpl.cpp
	src/assistant.cpp
	src/build.cpp
	src/cpphighlighter.cpp
	src/debug.cpp
	src/editor.cpp
	src/findfileimpl.cpp
	src/lineedit.cpp
	src/linenumbers.cpp
	src/main.cpp
	src/mainimpl.cpp
	src/misc.cpp
	src/newprojectimpl.cpp
	src/optionsimpl.cpp
	src/parametersimpl.cpp
	src/projectmanager.cpp
	src/projectpropertieimpl.cpp
	src/promptreplaceimpl.cpp
	src/replaceimpl.cpp
	src/selectionborder.cpp
	src/shortcutsimpl.cpp
	src/stackimpl.cpp
	src/subclassingimpl.cpp
	src/tabwidget.cpp
	src/textEdit.cpp
	src/treeclasses.cpp
	src/treeproject.cpp
	src/toolscontrolimpl.cpp
)

SET( qdevelop_MOC_HDRS	
	src/InitCompletion.h
	src/addexistantimpl.h
	src/addnewimpl.h
	src/addscopeimpl.h
	src/build.h
	src/debug.h
	src/editor.h
	src/findfileimpl.h
	src/linenumbers.h
	src/mainimpl.h
	src/newprojectimpl.h
	src/optionsimpl.h
	src/parametersimpl.h
	src/projectmanager.h
	src/projectpropertieimpl.h
	src/promptreplaceimpl.h
	src/replaceimpl.h
	src/shortcutsimpl.h
	src/stackimpl.h
	src/subclassingimpl.h
	src/tabwidget.h
	src/textEdit.h
	src/treeclasses.h
	src/treeproject.h
	src/toolscontrolimpl.h
)

SET( qdevelop_RCS
	QDevelop.rc
)

SET( qdevelop_RESOUCES
	resources/resources.qrc
)

# verbose - on
# SET( CMAKE_VERBOSE_MAKEFILE  ON )

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
QT4_ADD_RESOURCES ( qtsourceview_RC_SRCS  ${qdevelop_RESOUCES}  )
QT4_AUTOMOC       ( ${qdevelop_SRCS}                            )

# definitions for the project
ADD_DEFINITIONS(
	-Wall 
	${QT_DEFINITIONS}
)

# include directories
INCLUDE_DIRECTORIES( 
	${QT_INCLUDES} 
	${QT_INCLUDE_DIR}/QtUiTools
	${CMAKE_BINARY_DIR}
	.
	src 
	src/ui 
)

# the main build target
ADD_EXECUTABLE(qdevelop	
	${qdevelop_SRCS} 
	${qdevelop_UIS_H} 
	${qdevelop_MOC_SRCS} 
	${qdevelop_UIS} 
	${qtsourceview_RC_SRCS} 
)

# libraries to be linked to the project
IF (WIN32)
	SET( QT_UITOOLSLIB   ${QT_QTUITOOLS_LIBRARY_RELEASE} )
ELSE (WIN32)
# 	SET( QT_UITOOLSLIB   QtUiTools )
	SET( QT_UITOOLSLIB   ${QT_QTUITOOLS_LIBRARY_RELEASE} )
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(qdevelop
	${QT_UITOOLSLIB} 
	${QT_QTSQL_LIBRARY}
	${QT_LIBRARIES}
	${QT_QTXML_LIBRARY}
	${QT_QTNETWORK_LIBRARY}
)
