# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./Fixer
# Target is a library:  Fixer

HEADERS += BitReader.h \
           FeedBackInterface.h \
           FileBuffer.h \
           FrameObjectChecker.h \
           Mp3FileObject.h \
           Mp3Frame.h \
           Mp3Header.h \
           Mp3Reader.h \
           UnknownDataObject.h \
           VbrFixer.h \
           Mp3FileObjectChecker.h \
           FixerSettings.h \
           Id3Tags.h \
           XingFrame.h \
           EndianHelper.h \
           CrcCalc.h \
           ApeTag.h \
           VbriFrame.h \
           ReadSettings.h \
           GenHelpers.h \
           LyricsTag.h
SOURCES += FileBuffer.cpp \
           Mp3Frame.cpp \
           Mp3Header.cpp \
           Mp3Reader.cpp \
           UnknownDataObject.cpp \
           VbrFixer.cpp \
           FixerSettings.cpp \
           Id3Tags.cpp \
           XingFrame.cpp \
           Mp3FileObject.cpp \
           EndianHelper.cpp \
           CrcCalc.cpp \
           ApeTag.cpp \
           VbriFrame.cpp \
           ReadSettings.cpp \
           Version.cpp \
           LyricsTag.cpp
CONFIG += warn_on \
staticlib \
exceptions \
stl
TEMPLATE = lib
DESTDIR = lib
TARGET = Fixer

mac {
	QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
	QMAKE_MACOSX_DEPLOYMENT_TARGET=10.3
	CONFIG += x86 ppc

	LIBS +=-dead_strip
}

