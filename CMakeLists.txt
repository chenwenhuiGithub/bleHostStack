cmake_minimum_required(VERSION 3.5)

project(bleHostStack VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS SerialPort REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        att.cpp
        btsnoop.cpp
        gatt.cpp
        hci.cpp
        l2cap.cpp
        log.cpp
        ringbuffer.cpp
        serial.cpp
        sm.cpp
        att.h
        btsnoop.h
        gatt.h
        hci.h
        l2cap.h
        log.h
        ringbuffer.h
        serial.h
        sm.h
        config.h
    )
set(TINYCRYPT_SOURCES
        tinycrypt/source/aes_decrypt.c
        tinycrypt/source/aes_encrypt.c
        tinycrypt/source/cbc_mode.c
        tinycrypt/source/ccm_mode.c
        tinycrypt/source/cmac_mode.c
        tinycrypt/source/ctr_mode.c
        tinycrypt/source/ctr_prng.c
        tinycrypt/source/ecc.c
        tinycrypt/source/ecc_dh.c
        tinycrypt/source/ecc_dsa.c
        tinycrypt/source/ecc_platform_specific.c
        tinycrypt/source/hmac.c
        tinycrypt/source/hmac_prng.c
        tinycrypt/source/sha256.c
        tinycrypt/source/utils.c
   )
set(PROJECT_SOURCES ${PROJECT_SOURCES} ${TINYCRYPT_SOURCES})
include_directories(tinycrypt/include)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(bleHostStack
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET bleHostStack APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(bleHostStack SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(bleHostStack
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(bleHostStack PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(bleHostStack PRIVATE Qt6::SerialPort)

set_target_properties(bleHostStack PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(bleHostStack)
endif()
