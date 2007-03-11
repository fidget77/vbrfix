qmake -config release
gmake

mkdir vbrfix_release.d
cp -R AUTHORS COPYING README ConsoleFixer/VbrfixConsole QtFixer/VbrfixGui.app vbrfix_release.d
hdiutil create -srcfolder vbrfix_release.d -format UDBZ -volname "VBRFix" vbrfix.dmg
