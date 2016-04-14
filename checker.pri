
CONFIG += qt stl c++11 exceptions console warn_on
QT -= xml network gui widgets

!win32 {
    QMAKE_CXXFLAGS += -Werror
}

OBJECTS_DIR = o
MOC_DIR = o

HEADERS += \
	plugincandidates.h \
	knownplugins.h

SOURCES += \
	plugincandidates.cpp \
	knownplugins.cpp

        
