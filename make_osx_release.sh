# The steps to building are as follows:
# 
# 1. Download and install XCode.  This is the MacOS development environment:
#    http://developer.apple.com/tools/xcode/
# 
# 2. Download QT source tarball (http://www.trolltech.com/developer/downloads/qt/mac)
# 
# 3. Unpack and install qt:
# 	# tar xjf qt-mac-opensource-4.2.3.tar.bz2
#	# cd qt-mac-opensource-4.2.3
# 	# ./configure <options>
# 	# gmake
# 	# sudo gmake install
#
#       - We are assuming you are building a universal binary, so make sure you have
#         '-universal' and '-sdk /Developer/SDKs/MacOSX10.4u.sdk' among your
#	   configuration options.  If you do not want to make a univeral binary,
#	   you may need to edit the .pro files to remove reference to the appropriate
#	   lines.
#
# 	- If you're planning to build a release version of vbrfix, make sure
# 	  you have '-static' as one of your configure options.  Otherwise,
# 	  it will link with the dynamic libs which users won't have.
# 
# 4. Build and unpack vbrfix source.
# 
# 5. Execute macos build script.  This will compile all the applications and build the dmg.

rm -rf vbrfix.dmg vbrfix_release.d

qmake -config release
gmake

mkdir vbrfix_release.d
cp -R AUTHORS COPYING README ConsoleFixer/VbrfixConsole QtFixer/VbrfixGui.app vbrfix_release.d
hdiutil create -srcfolder vbrfix_release.d -format UDBZ -volname "VBRFix" vbrfix.dmg
