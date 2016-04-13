
TEMPLATE = app

CONFIG += qt stl c++11 exceptions console
QT -= xml network gui widgets

# Using the "console" CONFIG flag above should ensure this happens for
# normal Windows builds, but this may be necessary when cross-compiling
win32-x-g++:QMAKE_LFLAGS += -Wl,-subsystem,console

TARGET = checker

OBJECTS_DIR = o
MOC_DIR = o

HEADERS += \
	plugincandidates.h

SOURCES += \
	plugincandidates.cpp \
	checker.cpp

QMAKE_POST_LINK=make -f Makefile.helper

