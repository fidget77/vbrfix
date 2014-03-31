/***************************************************************************
                          vbrfix.cpp  -  description
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
#include "vbrfix.h"
//#include "stdafx.h"//include in mfc version
int getXHpos(int ver,int chan);
VbrFix::VbrFix(){
  frameptrs=NULL;flags=0;
  reading=false;writing=false;
  log=NULL;
  log=fopen("vbrfix.log","w");
  if(log==NULL)logi("Can't open log file");
}
VbrFix::~VbrFix(){
  endFix();
  if(log!=NULL)fclose(log);
}
void VbrFix::clearMp3Info(){
  vbr=false;id3v1=-1;id3v2=-1;totalBitrate=0;frameNo=0;lastsm=-1;lastver=-1;outFileSize=0;
  lyrics3Pos=-1;lyrics3Size=0;apeSize=0;apePos=-1;
  if(frameptrs!=NULL){free(frameptrs);frameptrs=NULL;}
  frameArraySize=6000;
  frameptrs=(Frame*)malloc(sizeof(Frame)*frameArraySize);//this can be realloced later
  if (frameptrs==NULL)throw("Out Of Memory");
  vbr=false;lastbr=-1;per=0;lastchan=-1;lastsm=-1;
  framesWritten=0;lamevbr=-1;lamevbrsize=0;
}
void VbrFix::logFile(const char* s){//log file info not tested yet
  if(log==NULL)return;
  fprintf(log,s);fprintf(log,"\n");
  fflush(log);
}
void VbrFix::startWriting(){
  logf("Gone through File");
  if(frameNo==0)throw("No frames identified");
  aBrate=int(totalBitrate/frameNo);
  logi(wps("DONE, ABR=") & aBrate & ",Frames=" & frameNo);
  if(vbr)logInfo("is VBR");else logInfo("not VBR");
  if(!vbr && !(flags & VBR_AlwaysWrite))throw("File Is Not VBR No need to do anything to it");
  if(lyrics3Pos!=-1 && id3v1==-1){
    loge("A ID3 Tag MUST be added to this mp3 as there is a lyrics3 tag");
  }
  reading=false;writing=true;
  writeId3v2();
  writeVbrHeader();
}
bool VbrFix::startFix(const char* file,const char* outFile){
  try{
    logi(wps("Fixing ") & file & "->" & outFile);
    logf(wps("flags = ") & flags);
    if(strcmp(file,outFile)==0){throw("inFile can't be the Same as OutFile");}
    clearMp3Info();
    openFiles(file,outFile);
    reading=true;writing=false;
  }
  catch(const char* err){
    if(err!=NULL)loge(err);
    endFix();
    return false;
  }
  return true;
}
void VbrFix::endFix(){
  mp3.closeFile();
  outmp3.closeFile();
  if(frameptrs!=NULL){free(frameptrs);frameptrs=NULL;}
  reading=false;writing=false;
  logf("ended fix");
}
bool VbrFix::contFix(const int c){
  int x;
  try{
    for(x=0;x<c;x++){
      if(reading){
        if(mp3.getpos()<mp3length){
          bool fnd=false;
          if(!fnd){fnd=checkFrame();if(!fnd)logf("not frame");}
          if(!fnd){fnd=checkId3v1();if(!fnd)logf("not id3v1 tag");}
          if(!fnd){fnd=checkId3v2();if(!fnd)logf("not id3v2 tag");}
          if(!fnd){fnd=checkLyrics3();if(!fnd)logf("not lyrics3 tag");}
          if(!fnd){fnd=checkAPE();if(!fnd)logf("not APE tag");}
          if(!fnd){//move to next byte
            H[0]=H[1];H[1]=H[2];H[2]=H[3];mp3.fRead(H+3,1);
          }
          per=(mp3.getpos()*50/mp3length);//percent through file
        }else startWriting();
      }
      if(writing){
        bool b=writeFrame();
        if(b){
          per=90;
          endFix();
          return true;
        }
      }
    }
  }
  catch(const char* err){
    if(err!=NULL)loge(err);
    endFix();
    return false;
  }
  return false;
}

void VbrFix::openFiles(const char* file,const char *outFile){
  logf("start opening of files");
  if(mp3.isopen())throw("There is a file already open");
  mp3.openFile(file,true);
  if (!mp3.isopen())throw("Failed to open input mp3 file");
  outmp3.openFile(outFile,false);
  if (!outmp3.isopen())throw("Failed to open output mp3 file");
  mp3length = mp3.getLength();
  logi(wps("FileLength = ") & mp3length);
  if(mp3length<256)throw("infile < 256 bytes - this must be too small to be a mp3");
  //get 1st 4 bytes
  mp3.fRead(H,4);
  logf("Opened Files");
}
void VbrFix::getBits(bool *B,char C){
  const int b[]={128,64,32,16,8,4,2,1};
  for(int x=0;x<8;x++)
    B[x]=(C & b[x])!=0;
}
bool VbrFix::checkFrame(){
  //Frame sync
  logf(wps("FRAME::") & H[0] & ":" & H[1] & ":" & H[2] & ":" & H[3] & "@" & mp3.getpos()-4);
  if(H[0] != 255 || H[1]<224)return false;//1st 11bits have to be 1s
  logf(wps("Checking Frame") & frameNo & " at " & mp3.getpos()-4);
  unsigned long originalPos=mp3.getpos()-4;
  bool B[33];
  getBits(B+9,H[1]);
  //mpeg version
  int ver=-1,br=0,sm=0,fSize=0;bool pad;
  if(!B[12] && !B[13])ver=3;//actually 2.5
  if(!B[12] &&  B[13]){logf("reserved");return false;}//reserved
  if( B[12] && !B[13])ver=2;
  if( B[12] &&  B[13])ver=1;
  //layer (we only do layer 3 for now)
  if(B[14] || !B[15]){logf("not layer 3");return false;}//not layer 3
  getBits(B+17,H[2]);
  //bit rate
  if(B[17])br+=8;if(B[18])br+=4;if(B[19])br+=2;if(B[20])br++;
  if(br==0){logf("unknown bitrate");return false;}//this is allowed but it confuses this program currently as cant work out framesize
  if(br==15){logf("unknown bitrate");return false;}//bitrate 1111 not allowed
  //sampling frequency
  if(B[21])sm+=2;if(B[22])sm++;
  if(sm==3){logf("reserved sample freq");return false;}//reserved
  //padding
  int padn=0;
  pad=B[23];
  if(pad)padn=1;
  //is it mono/stereo
  getBits(B+25,H[3]);
  int chan=2;if(B[25] && B[26])chan=1;//mono
  //there is more information but we don't really need it
  //some tables
  int brtab[2][15]={{0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320},
    {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}};
  int smtab[3][3] ={{44100,48000,32000},{22050,24000,16000},{11025,12000, 8000}};
  //work out size of frame
  if(ver == 1){fSize = (144000 * brtab[0][br]) / smtab[0][sm];if(pad)fSize++;}
  else {fSize = (72000 * brtab[1][br]) / smtab[ver-1][sm];if(pad)fSize++;}
  logf(wps("fSize=") & fSize & ",BRate=" & brtab[0][br] & ",SFreq=" & smtab[ver-1][sm] & ",Pad=" & padn & ",Chan=" & chan);
  if(fSize<5){logf("framesize < 5");return false;}//can't possibly be that small
  //any throws/returns after here must delete frame 1st
  //frame[0]=H[0];frame[1]=H[1];frame[2]=H[2];frame[3]=H[3];
  char xp[9];
  mp3.setpos(mp3.getpos()+fSize-4);//move to after the frame
  lastH[0]=H[0];lastH[1]=H[1];lastH[2]=H[2];lastH[3]=H[3];
  if(mp3.getpos()+4<mp3length)mp3.fRead(H,4); //make sure we arn't going past the end of the file
  else{H[0]=H[1]=H[2]=H[3]=0;}
  mp3.readAndReturn(xp,getXHpos(ver,chan)+mp3.getpos()-fSize,sizeof(xp));
  xp[8]=0;
  //we have decided it is a frame now
  //is it VBR/FHG
  if(xp[0]=='X' && xp[1]=='i' && xp[2]=='n' && xp[3]=='g'){
    //is it a lame one??
    int offsetLAME=8;
    if(xp[7] & 1)offsetLAME+=4;
    if(xp[7] & 2)offsetLAME+=4;
    if(xp[7] & 4)offsetLAME+=100;
    if(xp[7] & 8)offsetLAME+=4;
    mp3.readAndReturn(xp,getXHpos(ver,chan)+mp3.getpos()+offsetLAME-fSize,sizeof(xp));
    if(xp[0]=='L' && xp[1] == 'A' && xp[2]=='M' && xp[3]=='E'){
      if(lamevbr!=-1)logi("Found more than 1 LAME VBR TAG, using 1st one found");
      else {lamevbr=getXHpos(ver,chan)+mp3.getpos()+offsetLAME-fSize;lamevbrsize=fSize-(getXHpos(ver,chan)+offsetLAME);}
      logi(wps("Found [LAME] Xing tag at ") & mp3.getpos()-3-fSize & " with size " & fSize);
      if(flags & VBR_SkipLAME){
        throw("Skipping This File As Option set to skip files with LAME Vbr tags");
      }
    }
    else{
      logi(wps("Found Xing tag at ") & mp3.getpos()-3-fSize & " with size " & fSize);
    }
    return true;//forget it we dont want it or we have stored the position
  }
  if(xp[0]=='V' && xp[1]=='B' && xp[2]=='R' && xp[3]=='I'){
    logi(wps("Found VBRI tag at ") & mp3.getpos()-3-fSize & " with size " & fSize);
    //free(frame);
    return false;//forget it we dont want it
  }
  //assuming if it was we have left this function already...
  if(frameNo>=frameArraySize){
    logf("needs more memory to store frames");
    logf(wps("original = ") & frameArraySize);
    if(mp3.getpos()>0)
      frameArraySize=int(frameArraySize*(double(mp3length)/double(mp3.getpos()))+512);
    logf(wps("calculated at ") & frameArraySize);
    if(frameArraySize<frameNo+1024)frameArraySize=frameNo+1024;
    logf(wps("set to=") & frameArraySize);
    frameptrs=(Frame*)realloc(frameptrs,frameArraySize*sizeof(Frame));
    if (frameptrs==NULL)throw("Out of memory while dynamically resizing frames");
  }
  frameptrs[frameNo].oldptr = originalPos;
  frameptrs[frameNo].newptr = outFileSize;
  frameptrs[frameNo].size = fSize;
  if(lastbr==-1)lastbr=br;
  if(lastbr!=br)vbr=true;

  if(ver==1)totalBitrate+=brtab[0][br];else totalBitrate+=brtab[1][br];
  if(frameNo==0){//if its the 1st frame write out empty vbr header before it
    outFileSize+=fSize; //should be size of a vbr header
  }
  frameNo++;
  outFileSize+=fSize;//imagine writing frame to file
  //if(frame!=NULL){free(frame);frame=NULL;}
  //have we got a different ver/sm?
  if(lastver==-1)lastver=ver;if(lastsm==-1)lastsm=sm;if(lastchan<1)lastchan=chan;
  if(lastver!=ver){lastver=ver;logi(wps("WARNING:MPEG VERSION CHANGE at ") & frameNo);}//watch out for XING headers but we ignored them earlier so it doesn't matter in this case
  if(lastsm!=sm){lastsm=sm;logi(wps("WARNING:SAMPLE FREQ CHANGE at ") & frameNo);}// "
  if(lastchan!=chan){lastchan=chan;logi(wps("WARNING:Channel Mode CHANGE at ") & frameNo);}// "
  logf(wps("got to return") & chan & ",now at position" & mp3.getpos());
  return true;
}
bool VbrFix::checkId3v1(){
  if(flags & VBR_RemoveId3v1)return false;// don't let the program find the id3v1 if we don't want it
  if(H[0]=='T' && H[1]=='A' && H[2]=='G'){
    logf(wps("checking id3v1 tag at") &mp3.getpos()-4);
    if(id3v1!=-1){//>1 id3v1 tag
      logi("Warning - more than one id3v1 tag found using last found");
    }
    else{
      logi("Id3v1 tag found");
    }
    id3v1=mp3.getpos()-4;
    mp3.setpos(mp3.getpos()-4+128);
    mp3.fRead(H,4);
    return true;
  }
  return false;
}
bool VbrFix::checkAPE(){
  //check for header of APE tag
  if(H[0]=='A' && H[1]=='P' && H[2]=='E' && H[3]=='T'){
    unsigned char E[28];//32-4
    mp3.readAndReturn(E,mp3.getpos(),28);
    if(E[0]=='A' && E[1]=='G' && E[2]=='E' && E[3]=='X'){
      unsigned long apeV=E[4]+(E[5]+long(E[6]+E[7]*256)*256)*256;
      if(apeV!=1000 && apeV!=2000){logf("not a APE tag incorrect version(not 1.00 or 2.00) in header");return false;}
      bool b=true;for(int t=20;t<28;t++){if(E[t]!=0)b=false;}
      if(b==false){logf("not a APE tag 0's in header");return false;}
      apeSize=32+E[8]+(E[9]+long(E[10]+E[11]*256)*256)*256;
      logi(wps("Found APE tag with size") & apeSize);
      apePos=mp3.getpos()-4;
      mp3.setpos(mp3.getpos()-4+apeSize);
      mp3.fRead(H,4);
      return true;
    }
  }
  //doesn't check for not-headered APE tags
  return false;
}
bool VbrFix::checkLyrics3(){
  if(H[0]=='L' && H[1]=='Y' && H[2]=='R' && H[3]=='I'){
    unsigned char E[7];
    mp3.readAndReturn(E,mp3.getpos(),7);
    if(E[0]=='C' && E[1]=='S' && E[2]=='B' && E[3]=='E' && E[4]=='G' && E[5]=='I' && E[6]=='N'){
      mp3.fRead(E,7);//move through file
      if(lyrics3Pos!=-1){
        logi("Warning - more than one lyrics3 tag found, using last found");
      }
      lyrics3Pos=mp3.getpos()-8-4;
      //find its length
      bool e=false;unsigned char c;uint correctv1=0,correctv2=0,p=0;char ev1[]="LYRICSEND";char ev2[]="LYRICS200";
      while(!e && mp3.getpos()< mp3length){
        mp3.fRead(&c,1);
        if(c==255){logi("Error byte value of 255 in the lyrics3 tag!, asuming tag has ended");e=true;}
        if(c==ev1[correctv1])correctv1++;else {correctv1=0;if(c==ev1[correctv1])correctv1++;}
        if(c==ev2[correctv2])correctv2++;else {correctv2=0;if(c==ev2[correctv2])correctv2++;}
        if(correctv1==sizeof(ev1)-1 && p<5100){logf("version 1");e=true;}
        if(correctv2==sizeof(ev2)-1){logf("version 1");e=true;}
        p++;
      }
      lyrics3Size=p+8+4;
      logi(wps("Found Lyrics3 tag with size") & lyrics3Size);        
      mp3.fRead(H,4);return true;
    }
  }
  return false;
}
bool VbrFix::checkId3v2(){
  unsigned long oldpos=0;
  if(flags & VBR_RemoveId3v2)return false;// don't let the program find the id3v2 if we don't want it
  if(H[0]=='I' && H[1]=='D' && H[2]=='3' && (unsigned long)(mp3.getpos()+6)<mp3length && H[3]<255){
    logf(wps("checking id3v2 tag at") & mp3.getpos()-4);
    oldpos=mp3.getpos();
    unsigned char bf[10];mp3.fRead(bf+4,6);bf[0]=H[0];bf[1]=H[1];bf[2]=H[2];bf[3]=H[3];
    if(bf[4]<255 && bf[6]<128 && bf[7]<128 && bf[8]<128 && bf[9]<128){
      unsigned long id3size=bf[6]*128*128*128+bf[7]*128*128+bf[8]*128+bf[9];
      logf(wps("id3size=") & id3size);
      if (id3size>10){

        if(id3v2!=-1){//already have a id3v2 tag leave the old one
          mp3.setpos(oldpos);//move back to where we were
          return false;
        }
        id3v2=oldpos-4;
        id3size=id3size+10;
        logf(wps("id3size=") & id3size);
        if(bf[5] & 16)id3size=id3size+10;
        logf(wps("id3size=") & id3size);
        id3v2Size=id3size;
        logi(wps("Found Id3v2 tag with size") & id3size);
        mp3.setpos(oldpos+id3size-4);
        mp3.fRead(H,4);
        return true;
      }
    }
    mp3.setpos(oldpos);//move back to where we were
  }
  return false;
}
void VbrFix::revWrite(long n){
  unsigned char *fred;
  fred=(unsigned char*)&n;
  unsigned char t[4];
  t[0]=fred[3];t[1]=fred[2];t[2]=fred[1];t[3]=fred[0];
  outmp3.fWrite(t,4);
  //fwrite(t,4,1,outmp3);
}
void VbrFix::writeVbrHeader(){
  logf("Writing VBR HEADER");
  //some tables
  int brtab[2][15]={{0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320},
    {0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}};
  int smtab[3][3] ={{44100,48000,32000},{22050,24000,16000},{11025,12000, 8000}};
  //cont
  if(!vbr && !(VBR_AlwaysWriteVBR & flags))return;//no need to write a vbr header
  long flen = outFileSize,top=0,end=0;
  if(id3v2!=-1)top+=id3v2Size;
  if(apePos!=-1)end+=apeSize;
  if(lyrics3Pos!=-1)end+=lyrics3Pos;
  if(id3v1!=-1)end+=128;
  unsigned char toc[100];
  for(int x=0;x<100;x++){
    toc[x]=(unsigned char)(((frameptrs[(x*frameNo) / 100].newptr +top)*256) / (flen+end));
  }
  unsigned char XhD[4];
  XhD[0] = 255;XhD[1]=128+64+32;//1st 11bits on
  XhD[3] = lastH[3];
  int br=9,fSize,headSize=0;bool pad=0;
  //                          [12][13][14][15][16]       [17] [18] [19] [20]
  if (lastver == 1) {XhD[1] += 16 + 8 + 0 + 2 + 1;XhD[2]=128 +  0 + 0 + 16;br=9;}
  if (lastver == 2) {XhD[1] += 16 + 0 + 0 + 2 + 1;XhD[2]=128 +  0 + 0 +  0;br=8;}
  if (lastver == 3) {XhD[1] +=  0 + 0 + 0 + 2 + 1;XhD[2]=  0 + 64 + 0 +  0;br=4;}//mpeg 2.5
  //                    [21][22]
  if(lastsm==0){XhD[2]+= 0 + 0;}
  if(lastsm==1){XhD[2]+= 0 + 4;}//was 8+ 0 was it correct to change it?
  if(lastsm==2){XhD[2]+= 8 + 0;}// was 8+4 was it correct to change it?
  outmp3.fWrite(XhD,4);headSize+=4;
  if(lastver == 1){fSize = (144000 * brtab[0][br]) / smtab[0][lastsm];if(pad)fSize++;}
  else {fSize = (72000 * brtab[1][br]) / smtab[lastver-1][lastsm];if(pad)fSize++;}

  //XhD[2]=28+4*lastsm;
  logi(wps("CHAN=") & lastchan);
  int xp = getXHpos(lastver,lastchan);//where we would find 'Xing'
  for(int t=0;t<xp;t++)outmp3.fWrite("\0",1);headSize+=xp;
  outmp3.fWrite("Xing",4);headSize+=4;
  revWrite(15);headSize+=4;
  revWrite(frameNo);headSize+=4;
  revWrite(flen+top+end);headSize+=4;
  outmp3.fWrite(toc,100);headSize+=100;//is adding 100 correct???
  revWrite(flen / (125 * aBrate));headSize+=4;//should be vbr scale 0-100
  logf(wps("HEADSIZE=") & headSize);
  if(lamevbr!=-1 && (flags & VBR_ReplaceLAMEkeepinfo)){
    char* lameinfo;lameinfo=(char*)malloc(fSize-headSize);
    mp3.readAndReturn(lameinfo,lamevbr,fSize-headSize);//copy the LAME tag in?
    outmp3.fWrite(lameinfo,fSize-headSize);//write the lame tag in the new vbrtag space
    free(lameinfo);
  }
  else{
    for(int tt=0;tt<fSize-headSize;tt++)outmp3.fWrite("\0",1); //fill rest of frame
  }
  logi("Written new vbr tag");
}
bool VbrFix::writeFrame(){
  char *tmp;tmp=NULL;unsigned long x=framesWritten;
  tmp=(char*)malloc(frameptrs[x].size);
  if(tmp==NULL)throw("out of memory error");
  mp3.setpos(frameptrs[x].oldptr);
  mp3.fRead(tmp,frameptrs[x].size);
  outmp3.fWrite(tmp,frameptrs[x].size);
  free(tmp);tmp=NULL;
  per=50+x*40/frameNo;
  framesWritten++;
  if(framesWritten==frameNo){
    writeAPE();
    writeLyrics3();
    writeId3v1();
    writing=false;
    return true;
  }
  return false;
}
void VbrFix::writeId3v1(){
  logf("Writing Id3v1 tag");
  if(id3v1<0)return;//there is no id3v1 tag or we don't want to write one
  char tmp[128];
  mp3.setpos(id3v1);
  mp3.fRead(tmp,128);
  outmp3.fWrite(tmp,128);
  logf("Done");
}
void VbrFix::writeId3v2(){
  logf("Writing Id3v2 tag");
  if(id3v2<0)return;//there is no id3v2 tag or we don't want to write one
  char *tmp;tmp=NULL;
  tmp=(char*)malloc(id3v2Size);
  if(tmp==NULL)throw("out of memory error when writing id3v2 tag");
  mp3.setpos(id3v2);
  mp3.fRead(tmp,id3v2Size);
  outmp3.fWrite(tmp,id3v2Size);
  free(tmp);
  logf("Done");
}
void VbrFix::writeLyrics3(){
  logf("Writing Lyrics3 tag");
  if(lyrics3Size<=0)return;//tag is < 0 in size
  char *tmp;tmp=NULL;
  tmp=(char*)malloc(lyrics3Size);
  if(tmp==NULL)throw("out of memory error when writing lyrics3 tag");
  mp3.setpos(lyrics3Pos);
  mp3.fRead(tmp,lyrics3Size);
  outmp3.fWrite(tmp,lyrics3Size);
  free(tmp);
  logf("Done");  
}
void VbrFix::writeAPE(){
  logf("Writing APE tag");
  if(apeSize<=0)return;//tag is < 0 in size
  char *tmp;tmp=NULL;
  tmp=(char*)malloc(apeSize);
  if(tmp==NULL)throw("out of memory error when writing APE tag");
  mp3.setpos(apePos);
  mp3.fRead(tmp,apeSize);
  outmp3.fWrite(tmp,apeSize);
  free(tmp);
  logf("Done");  
}
void VbrFix::slowFileCopy(const char* input,const char* output){//this function name is outdated
  if(output==NULL || input ==NULL){
    loge("Failed To copy files after fix");
  }
  else{
    if (wfile::copyFile(input,output,false)){
      logi(wps("Copied ") & input & "->" & output);
      per=100;
    }
    else{
      loge(wps("Failed to copy ") & input & "->" & output);
    }
  }
}
int getXHpos(int ver,int chan){
  //thanks to James Heinrich(author of getID3) for this part
  //MPEG1-mono: 0x15
  //MPEG1-stereo: 0x24
  //MPEG2[.5]-mono: 0x0D
  //MPEG2[.5]-stereo: 0x15
  if(ver==1){
    if(chan==1)return 0x15-4;else return 0x24-4;
  }
  else{
    if(chan==1)return 0x0D-4;else return 0x15-4;
  }
}

