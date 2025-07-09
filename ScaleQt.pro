QT       += core gui serialport bluetooth


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    bluetoothscanner.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    bluetoothscanner.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Windows Executable and macOS Bundle Properties
# Equivalent to:
# set_target_properties(lowenergyscanner PROPERTIES
#     WIN32_EXECUTABLE TRUE
#     MACOSX_BUNDLE TRUE
# )
# For WIN32_EXECUTABLE TRUE:
win32:QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS
# MACOSX_BUNDLE TRUE is default for TEMPLATE=app on macOS, no explicit setting needed.

# Custom Info.plist for macOS/iOS (as discussed before)
# Equivalent to the if (APPLE) block
macx {
    # Using absolute path for shared plist files is a Ninja bug workaround
    # Adjust path if your 'shared' directory is not a sibling of the project dir
    SHARED_PLIST_DIR = $$PWD/../build

    # Conditional for iOS
    ios {
        QMAKE_INFO_PLIST = $$SHARED_PLIST_DIR/Info.cmake.ios.plist
    } else {
        # macOS
        QMAKE_INFO_PLIST = $$PWD/Info.plist
    }
}




# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    ##    $$PWD/../ScaleQt.pro
}
