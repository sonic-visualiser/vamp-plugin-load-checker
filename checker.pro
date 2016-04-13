
TEMPLATE = app

CONFIG += qt stl c++11 exceptions console warn_on
QT -= xml network gui widgets

# Using the "console" CONFIG flag above should ensure this happens for
# normal Windows builds, but this may be necessary when cross-compiling
win32-x-g++:QMAKE_LFLAGS += -Wl,-subsystem,console

QMAKE_CXXFLAGS += -Werror

TARGET = checker

OBJECTS_DIR = o
MOC_DIR = o

HEADERS += \
	plugincandidates.h \
	knownplugins.h

SOURCES += \
	plugincandidates.cpp \
	knownplugins.cpp \
	checker.cpp

QMAKE_POST_LINK=make -f Makefile.helper

