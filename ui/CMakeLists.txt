set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_PREFIX_PATH /home/ntxc/Qt/5.15.2/gcc_64/lib/cmake)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui PrintSupport Multimedia)

file(GLOB PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui)

message(STATUS "Vits include dir: ${VITS_INCLUDE_DIRS}")
# set(PROJECT_SOURCES
#         main.cpp
#         loadingscreen.cpp
#         loadingscreen.h
#         loadingscreen.ui
#         mainwindow.cpp
#         mainwindow.h
#         mainwindow.ui
#         audiodevice.cpp
#         audiodevice.h
#         audiosource.cpp
#         audiosource.h
#         controller.cpp
#         controller.h
#         labelbutton.cpp
#         labelbutton.h
#         qcustomplot.cpp
#         qcustomplot.h
#         style.h
#         switch.cpp
#         switch.h
#         utils.cpp
#         utils.h
# )

include_directories(whisper_a2o
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5PrintSupport_INCLUDE_DIRS}
        ${Qt5Multimedia_INCLUDE_DIRS}
        ${VITS_INCLUDE_DIRS}
        ${SAMPLERATE_INCLUDE_DIRS}
        )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(whisper_a2o
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET whisper_a2o APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(whisper_a2o SHARED
                ${PROJECT_SOURCES}
                )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(whisper_a2o
                ${PROJECT_SOURCES}
                )
    endif()
endif()

target_link_libraries(whisper_a2o
        PRIVATE Qt${QT_VERSION_MAJOR}::Core
        PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        PRIVATE Qt${QT_VERSION_MAJOR}::Gui
        PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport
        PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia
        PRIVATE vits
        PRIVATE samplerate
        PRIVATE ${TORCH_LIBRARIES}
        )

set_target_properties(whisper_a2o PROPERTIES
        WIN32_EXECUTABLE TRUE
        )

install(TARGETS whisper_a2o
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(whisper_a2o)
endif()