QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 c++2a

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    att.cpp \
    btsnoop.cpp \
    gatt.cpp \
    hci.cpp \
    l2cap.cpp \
    log.cpp \
    ringbuffer.cpp \
    serial.cpp \
    main.cpp \
    mainwindow.cpp \
    sm.cpp \
    tinycrypt/source/aes_decrypt.c \
    tinycrypt/source/aes_encrypt.c \
    tinycrypt/source/cbc_mode.c \
    tinycrypt/source/ccm_mode.c \
    tinycrypt/source/cmac_mode.c \
    tinycrypt/source/ctr_mode.c \
    tinycrypt/source/ctr_prng.c \
    tinycrypt/source/ecc.c \
    tinycrypt/source/ecc_dh.c \
    tinycrypt/source/ecc_dsa.c \
    tinycrypt/source/ecc_platform_specific.c \
    tinycrypt/source/hmac.c \
    tinycrypt/source/hmac_prng.c \
    tinycrypt/source/sha256.c \
    tinycrypt/source/utils.c

HEADERS += \
    att.h \
    btsnoop.h \
    gatt.h \
    hci.h \
    l2cap.h \
    log.h \
    ringbuffer.h \
    serial.h \
    mainwindow.h \
    sm.h \
    tinycrypt/include/aes.h \
    tinycrypt/include/cbc_mode.h \
    tinycrypt/include/ccm_mode.h \
    tinycrypt/include/cmac_mode.h \
    tinycrypt/include/constants.h \
    tinycrypt/include/ctr_mode.h \
    tinycrypt/include/ctr_prng.h \
    tinycrypt/include/ecc.h \
    tinycrypt/include/ecc_dh.h \
    tinycrypt/include/ecc_dsa.h \
    tinycrypt/include/ecc_platform_specific.h \
    tinycrypt/include/hmac.h \
    tinycrypt/include/hmac_prng.h \
    tinycrypt/include/sha256.h \
    tinycrypt/include/utils.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
