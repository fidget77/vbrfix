/***************************************************************************
                          wputil.h  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Eric Benson
    email                : eric_a_benson@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// Eric Benson altered to wputilqt.h & wputilmfc.h
// to produce this version using only C++ calls not needing MFC or QT
// Based on code by william Pye
using namespace std;

#include <stdio.h>
#include <string>
#include <fstream>

inline string trim_right(const string& source, const string& t = " \t")
{
  string str = source;
  return str.erase(str.find_last_not_of(t) + 1);
}

inline string trim_left(const string& source, const string& t = " \t")
{
  string str = source;
  return str.erase(0 ,source.find_first_not_of(t));
}

inline string string_trim(const string& source, const string& t = " \t")
{
  string str = source;
  return trim_left(trim_right(str ,t), t);
}

inline string left(const string& source, unsigned int len)
{
  return source.substr(0, len);
}

inline string right(const string& source, unsigned int len)
{
  return source.substr(source.length() - len, len);
}

class wps{
  private:
  string str;
  public:
    wps(string s){
      str=s;
    }
    wps(const char *c){
      if(c!=NULL)str=c;
    }
    wps(bool b){
      if(b) str="true";else str="false";
    }
    wps operator&(wps add){
      return wps(str+add.str);
    }
    wps operator&(int i){
      char tmp[100];
      sprintf(tmp,"%d",i);
      return wps(str+tmp);
    }
    wps operator=(wps &equal){
      wps fred(equal.str);
      return fred;
    }
    wps operator=(const char *c){
      wps fred(c);
      return fred;
    }
    const char* getStr(){
      return str.c_str();
    }
    int length(){
      return str.length();
    }
    wps trim(){
      return wps(string_trim(str));
    }
    bool isTrue(){
      if(str=="true" || str=="TRUE" || str=="True" || str=="T" || str=="Y" || str=="y" || str=="t"){
        return true;
      }
      else return false;
    }
};
class wfile{
private:
  fstream fil;
  bool rmode; //rmode=true -> read rmode=false -> write
  bool opened;
public:
  wfile(){
    opened=false;
    rmode=true;
  }
  ~wfile(){
    closeFile();
  }
  void openFile(const char* name,bool read){
    if(opened)closeFile();
    rmode=read;ios_base::openmode flags;
    if(rmode) {
      flags=ios::in|ios::binary;
    } else {
      flags=ios::out|ios::binary;
    }
    fil.open(name, flags);
    if (!fil.fail()){
	opened=true;
    }
  }
  void closeFile(){
    if (opened) {
      fil.close();
    }
    opened=false;
  }
  unsigned long getLength(){
    if (rmode) {
      streampos here = fil.tellg();
      fil.seekg(0, ios::beg);
      streampos start = fil.tellg();
      fil.seekg(0, ios::end);
      streampos finish = fil.tellg();
      fil.seekg(here, ios::beg);
      return (finish - start);
    } else {
      streampos here = fil.tellp();
      fil.seekp(0, ios::beg);
      streampos start = fil.tellp();
      fil.seekp(0, ios::end);
      streampos finish = fil.tellp();
      fil.seekp(here, ios::beg);
      return (finish - start);
    }
  }
  void fWrite(const void* ptr, unsigned int size){
    if(!rmode)fil.write((char*)ptr,size);
    if (fil.fail()) abort();
  }
  void fRead(void* ptr,unsigned int size){
    if(rmode)fil.read((char*)ptr,size);
    if (fil.fail()) fil.clear();
  }
  void readAndReturn(void* ptr,unsigned long pos, unsigned int size){
    unsigned long cpos=getpos();
    setpos(pos);
    fRead((char*)ptr,size);
    setpos(cpos);
  }
  bool isopen(){
    return opened;
  }
  unsigned long getpos(){
    unsigned long pos;
    if (!opened)return 0;
    if (rmode) {
      pos = fil.tellg();
    } else {
      pos = fil.tellp();
    }
    if (fil.fail()) fil.clear();
    return pos;
  }
  void setpos(unsigned long pos){
    if (!opened)return;
    if (rmode) {
      fil.seekg(pos, ios::beg);
    } else {
      fil.seekp(pos, ios::beg);
    }
    if (fil.fail())fil.clear();
  }
  static bool copyFile(const char *input,const char *output,bool dontreplace){
    ifstream in(input, ios::in|ios::binary);
    if(in.fail())return false;
    if(dontreplace){
      // Portable test for existing file
      ifstream test(output, ios::in|ios::binary);
      if(!test.fail()){
	in.close();
	test.close();
	return false;
      }
    }
    ofstream out(output, ios::out|ios::binary|ios::trunc);
    if(out.fail())return false;
    // fast portable file copy
    return out << in.rdbuf();
  }
  static wps loadSetting(const char *file,wps setting,wps defaultValue){
    ifstream in(file, ios::in);
    if(in.fail())return defaultValue;
    while(!in.eof()){
      string line;
      getline(in,line);
      string l=string_trim(line);
      int i=l.find("=");
      if(i<1 || left(l,2)=="//" || left(l,1)=="#" || left(l,1)==";"){//commented line
      }
      else
      {
        if(string_trim(left(l,i))==setting.getStr()){
          in.close();return wps(string_trim(right(l,l.length()-i-1)));
        }
      }
    }
    in.close();
    return defaultValue;
  }
  static void saveSetting(const char* file,wps setting,wps value){
    ifstream in(file);ofstream out("tmp.tmp");
    if(in.fail()){
      ofstream tmpout(file, ios::out);
      if(!tmpout.fail()){
        tmpout <<"#[" << file << "] CONFIG FILE\n";
        tmpout.close();
	in.open(file);
      }
    }
    if(in.fail())return;
    if(out.fail()){in.close();return;}
    bool found=false;
    while(!in.eof()){
      string line;
      getline(in, line);
      string l=string_trim(line);
      int i=l.find("=");
      if(i<1 || left(l,2)=="//" || left(l,1)=="#" || left(l,1)==";"){//commented line
        out << line << "\n";
      }else{
        if(string_trim(left(l,i))==setting.getStr()){
          if(found==false){//otherwise delete the line
            found=true;out << setting.getStr() << "=" << value.getStr() << "\n";
          }
        }
        else{
          out << line << "\n";
        }
      }
    }
    if(found==false){
      out << setting.getStr() << " = " << value.getStr() << "\n";
    }
    in.close();
    out.close();
    //copy tmp.tmp to config file
    copyFile("tmp.tmp",file,false);
    remove("tmp.tmp");
  }
  static bool remove(const char* fileName){
    int status = ::remove(fileName);
    return status == 0;
  }
};
