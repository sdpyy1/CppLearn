cmake_minimum_required(VERSION 3.16)

project(ARender VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third/include)
file(GLOB OPENGL_SOURCES
    "openGL/*.h"
    "openGL/*.cpp"
    "openGL/*.qrc"
)
include_directories(component page utils openGL)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ARender
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        component/pushbutton.h component/pushbutton.cpp
        component/framelesswindow.h component/framelesswindow.cpp
        utils/logger.h utils/logger.cpp
        component/lineeditwidget.h component/lineeditwidget.cpp
        component/pagewidget.h component/pagewidget.cpp
        component/roundedcornerwidget.h component/roundedcornerwidget.cpp
        component/scrolllistwidget.h component/scrolllistwidget.cpp
        component/sidebar.h component/sidebar.cpp
        component/slider.h component/slider.cpp
        page/aboutpage.h page/aboutpage.cpp
        mainwindow.qrc
        page/sceneviewer.h page/sceneviewer.cpp
        page/editorpage.h page/editorpage.cpp
        page/modelselector.h page/modelselector.cpp
        page/modelsetter.h page/modelsetter.cpp
        page/modelselectable.h page/modelselectable.cpp
        page/modelthumbnailwidget.h page/modelthumbnailwidget.cpp
        page/modelattributeslide.h page/modelattributeslide.cpp
        {OPENGL_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ARender APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ARender SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ARender
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ARender PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ARender PRIVATE Qt6::OpenGLWidgets)
# target_link_libraries(ARender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/libassimp.dll.a)
# target_link_libraries(ARender ${CMAKE_CURRENT_SOURCE_DIR}/third/lib/glfw3.dll )
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ARender)
endif()
set_target_properties(ARender PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ARender
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ARender)
endif()
