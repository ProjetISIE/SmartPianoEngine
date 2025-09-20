QT += core gui network multimedia multimediawidgets
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
DEFINES += __LINUX_ALSA__
CONFIG += debug
QMAKE_CXXFLAGS_DEBUG += -g
QT_DEBUG_PLUGINS=1



# Inclure les chemins pour les fichiers d'en-tete
INCLUDEPATH += include
INCLUDEPATH += $$PWD/rtmidi
DEPENDPATH += $$PWD/rtmidi

# Fichiers source
SOURCES += \
    src/main.cpp \
    src/GameManager.cpp \
    src/SocketManager.cpp \
    src/GenererNoteAleatoire.cpp \
    src/ValidationNote.cpp \
    src/LectureNoteJouee.cpp \
    src/GestionSon.cpp \
    src/BaseAccords.cpp \
    $$PWD/rtmidi/RtMidi.cpp 

# Fichiers d'en-tete
HEADERS += \
    include/GameManager.h \
    include/SocketManager.h \
    include/GenererNoteAleatoire.h \
    include/ValidationNote.h \
    include/LectureNoteJouee.h \
    include/GestionSon.h \
    include/BaseAccords.h \
    $$PWD/rtmidi/RtMidi.h \
    include/Logger.h

# Lier la bibliotheque ALSA pour le support MIDI
LIBS += -L/home/vivien/Desktop/PRI/pianotrainer/PianoTrainerMDJV1/rtmidi -lrtmidi -L/lib/aarch64-linux-gnu -lasound -lstdc++ -lpthread -lpulse
LIBS += -L$$PWD/rtmidi 

 
message("LIBS : $$LIBS")
