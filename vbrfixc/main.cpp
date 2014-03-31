/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue Jul 15 19:39:16 BST 2003
    copyright            : (C) |YEAR| by Wi2003Pye
    email                : will@willwap.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <sstream>
#include "vbrfix.h"
using namespace std;
class Fixer:public VbrFix{
  public:
  void fix(char* file,char* outFile,int flags);
};
void Fixer::fix(char* inFile,char* outFile,int flags){
  setflags(flags);
  startFix(inFile,"vbrfix.tmp");
  int pos=0;
  while(isWorking()){
    if(contFix(300)){
      slowFileCopy("vbrfix.tmp",outFile);
    }
    if(getPercent()/10!=pos){
      pos=getPercent()/10;cout << pos*10 << "%..." <<endl;
    }
  }
}
int getFlag(string f){
  if(f=="-ri1")return VBR_RemoveId3v1;
  if(f=="-ri2")return VBR_RemoveId3v2;
  if(f=="-skiplame")return VBR_SkipLAME;
  if(f=="-allways")return VBR_AlwaysWrite;
  if(f=="-makevbr")return VBR_AlwaysWriteVBR;
  if(f=="-log")return VBR_WriteLogFile;
  if(f=="-lameinfo")return VBR_ReplaceLAMEkeepinfo;
  cout <<"UNKNOWN FLAG ::"<<f<<endl;
  return 0;
}
int main(int argc, char *argv[])
{
  cout << "VbrFix Command Line Version" << endl;
  cout << "===========================" << endl;
  cout << "By William Pye, visit www.willwap.co.uk for latest version"<<endl;
  if(argc < 3){
    cout << "No Arguments Given" << endl;
    cout << "Use:" <<endl;
    cout << "vbrfixc -flag1 -flag2 -flagn in.mp3 out.mp3"<<endl;
    cout << "FLAGS::"<<endl;
    cout << "-ri1          removeId3v1 Tag"<<endl;
    cout << "-ri2          removeId3v2 Tag"<<endl;
    cout << "-skiplame     if tag made by lame don't fix it"<<endl;
    cout << "-allways      always write even if not vbr"<<endl;
    cout << "-makevbr      make it vbr(you need -allways also)"<<endl;
    cout << "-log          write a log file"<<endl;
    cout << "-lameinfo     keep the lame info"<<endl;
    return EXIT_SUCCESS;
  }
  Fixer fixer;int flags=0;
  for(int x=1;x<argc-2;x++){
    int f=getFlag(argv[x]);
    if(!(f & flags))flags+=f;
  }
  cout << "FLAGS = " << flags << endl;
  fixer.fix(argv[argc-2],argv[argc-1],flags);
  return EXIT_SUCCESS;
}
