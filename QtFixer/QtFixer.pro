# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./QtFixer
# Target is an application:  VbrfixGui

FORMS += settings.ui \
         vbrfixui.ui \
         about.ui 
HEADERS += Mp3FileListItem.h \
           VbrfixMain.h \
           VbrfixSettings.h \
           VbrfixThread.h \
           Options.h \
           VbrfixAbout.h \
           ComboManager.h 
SOURCES += Main.cpp \
           Mp3FileListItem.cpp \
           VbrfixMain.cpp \
           VbrfixSettings.cpp \
           VbrfixThread.cpp \
           Options.cpp \
           VbrfixAbout.cpp 
RESOURCES = res.qrc
POST_TARGETDEPS += ../Fixer/lib/libFixer.a
LIBS += ../Fixer/lib/libFixer.a
INCLUDEPATH += ../Fixer
DEFINES += QT_NO_CAST_TO_ASCII
TARGET = VbrfixGui
CONFIG += warn_on \
qt \
thread \
exceptions \
stl
TEMPLATE = app
