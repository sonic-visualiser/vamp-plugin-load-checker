
TEMPLATE = app

CONFIG += stl c++11 exceptions console warn_on
CONFIG -= qt

# Using the "console" CONFIG flag above should ensure this happens for
# normal Windows builds, but this may be necessary when cross-compiling
win32-x-g++:QMAKE_LFLAGS += -Wl,-subsystem,console

!win32* {
    QMAKE_CXXFLAGS += -Werror
}

linux* {
    QMAKE_LFLAGS += -ldl
}

TARGET = plugin-checker-helper

OBJECTS_DIR = o
MOC_DIR = o

SOURCES += \
	src/helper.cpp

