/***************************************************************************
                          vbrfix.h  -  description
                             -------------------
    begin                : Tue Feb 25 2003
    copyright            : (C) 2003 by William Pye
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
#ifndef VBRFIX_H
#define VBRFIX_H
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "wputil.h"
struct Frame{
  unsigned long oldptr;
  unsigned long newptr;
  int size; 
};
//options
#define VBR_RemoveId3v2 1
#define VBR_RemoveId3v1 2
#define VBR_SkipLAME 4
#define VBR_AlwaysWrite 32
#define VBR_AlwaysWriteVBR 64
#define VBR_OnlyReportErrors 512
#define VBR_WriteLogFile 1024
#define VBR_ReplaceLAMEkeepinfo 2048

//main class
class VbrFix
{
  public:
    //bool fixMp3(const char* fileName,const char* outFile);//main function
    bool startFix(const char* fileName,const char* outFile);
    bool contFix(const int c);  //returns true only if we have completed fixing the mp3
    int getPercent(){return per;}
    bool isWorking(){return reading || writing;}
    void endFix();
    void setflags(unsigned long f){flags=f;}
    void slowFileCopy(const char* input,const char* output);
    VbrFix();
    virtual ~VbrFix();
    void logf(const char* s){
      if(VBR_WriteLogFile & flags)logFile(s);
    }
    virtual void logInfo(const char* s){if(s==NULL)return;}//already printed to stdout
    virtual void logError(const char* s){if(s==NULL)return;}//already printed to stdout
  private:
    void startWriting();
    bool reading,writing;
    //mp3 working need to be cleared using clearMp3Info
    unsigned char H[4],lastH[4];
    unsigned long flags;//option flags
    bool vbr;Frame *frameptrs;
    long id3v1,id3v2,lamevbr,lamevbrsize,aBrate,lastsm,lastver,lastbr,lastchan,lyrics3Pos,lyrics3Size,apePos,apeSize;
    unsigned long totalBitrate, frameNo,frameArraySize, id3v2Size;
    void clearMp3Info();
    void getBits(bool *b,char C);
    
    //file related
    void openFiles(const char* file,const char* outFile);
    void wread(void* ptr,int bytes);
    unsigned long outFileSize;
    wfile mp3,outmp3;
    unsigned long mp3length; // Eric Benson added this small porformance increase
    void revWrite(long n);

    //checking the mp3
    bool checkFrame();
    bool checkId3v1();
    bool checkId3v2();
    bool checkLyrics3();
    bool checkAPE();

    //writing the mp3
    void writeId3v2();
    void writeVbrHeader();
    bool writeFrame();
    void writeId3v1();
    void writeLyrics3();
    void writeAPE();

    //logging
    unsigned long per,framesWritten;
    FILE* log;
    void logFile(const char* s);
    void logi(const char* s){
      if(VBR_OnlyReportErrors & flags)return;fprintf(stdout,"%s\n",s);logf(s);
      logInfo(s);
    }
    void loge(const char* s){
      fprintf(stderr,"%s\n",s);logf(s);
      logError(s);
    }
    void logi(string s){logi(s.c_str());}
    void loge(string s){loge(s.c_str());}
    void logf(string s){logf(s.c_str());}
    void logi(wps w){logi(w.getStr());}
    void loge(wps w){loge(w.getStr());}
    void logf(wps w){logf(w.getStr());}
};
#endif
