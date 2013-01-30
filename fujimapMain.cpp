#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <stdlib.h>
#include "cmdline.h"
#include "fujimap.hpp"

using namespace std;

void printCurrentStatus(fujimap_tool::Fujimap& fm){
  size_t wsize = fm.getWorkingSize();
  cerr << "keyNum:       " << fm.getKeyNum() << endl
       << "fpLen:        " << fm.getFpLen() << endl
       << "encoding:     " << fm.getEncodeTypeStr() << endl
       << "wsize(bytes): " << wsize / 8 << endl
       << "bits/key:     " << (float)wsize / fm.getKeyNum() << endl;
}

int buildFromFile(cmdline::parser& p){
  istream *pis=&cin;
  ifstream ifs;
  if (p.get<string>("dic") != "-"){
    ifs.open(p.get<string>("dic").c_str());
    if (!ifs){
      cerr << "Unable to open " << p.get<string>("dic") << endl;
      return -1;
    }

    pis = static_cast<istream*>(&ifs);
  }

  istream &is=*pis;
  fujimap_tool::Fujimap fm;
  if (p.exist("seed") {
      fm.initSeed(p.get<int>("seed"));
  }

  uint64_t fpLen = 0;
  if (p.exist("fpwidth")){
    fm.initFP(p.get<int>("fpwidth"));
    fpLen = p.get<int>("fpwidth");
  }

  if (p.exist("tmpN")){
    fm.initTmpN(p.get<int>("tmpN"));
  }

  if (p.exist("workingfile")){
    if (fm.initWorkingFile(p.get<string>("workingFile").c_str()) == -1){
      cerr << fm.what() << endl;
      return -1;
    }
  }

  bool logValue = p.exist("logvalue");
  bool stringValue = p.exist("stringvalue");

  if (p.exist("encode")){
    string encodeTypeS = p.get<string>("encode");
    fujimap_tool::EncodeType et = fujimap_tool::BINARY;
    if (encodeTypeS == "binary"){
      et =  fujimap_tool::BINARY;
    } else if (encodeTypeS == "gamma"){
      et = fujimap_tool::GAMMA;
    } else {
      p.usage();
      return -1;
    }
    fm.initEncodeType(et);
  }

  size_t readNum = 0;
  string line;
  for (size_t lineN = 1; getline(is, line); ++lineN){
    size_t p = line.find_last_of('\t');
    if (p == string::npos){
      cerr << "Warning: not tab found : " << line << endl;
      continue;
    }
    if (p == 0 || p+1 == line.size()) continue; // no key or no value


    if (stringValue){
      fm.setString(line.c_str(), p,
                   line.substr(p+1).c_str(), line.substr(p+1).size(),
                   false);
    } else {
        uint64_t val = strtoll(line.substr(p+1).c_str(), NULL, 10);
      if (logValue){
        if (val == 0){
          cerr << "When -l is specified, all values should be larger than 0" << endl;
          cerr << "line " << lineN << ":" << line << endl;
          return -1;
        }
        val = fujimap_tool::log2(val);
      }
      fm.setInteger(line.c_str(), p, val, false);
    }

    readNum++;
  }
  cerr << "read " << readNum << " keys" << endl;

  int ret = fm.build();
  if (ret == -1){
    return -1;
  }
  cerr << "build done." << endl;
  printCurrentStatus(fm);

  if (fm.save(p.get<string>("index").c_str()) == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  cerr << "save done." << endl;
  return 0;
}


int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("index",       'i', "Index",                                                        true);
  p.add<string>("dic",         'd', "Key/Value file. when \"-\" is specified it reads from stdin",  false);
  p.add<int>   ("fpwidth",     'f', "False positive rate 2^{-f} (0 <= f < 31) ",                    false);
  p.add<int>   ("tmpN",        't', "TemporarySize",                                                false);
  p.add<int>   ("seed",        'S', "seed for random",                                              false);
  p.add<string>("encode",      'e', "Code encoding  (=binary, gamma)",                              false);
  p.add<string>("workingfile", 'w', "Working file for temporary data",                              false);
  p.add        ("logvalue",    'l', "When specified, store a log of input value");
  p.add        ("stringvalue", 's', "When specified, sotre a string of input value");
  p.add("help", 'h', "print this message");
  p.set_program_name("fujimap");

  bool parseOK = p.parse(argc, argv);
  if (argc == 1 || p.exist("help")){
    cerr << p.usage();
    return -1;
  }

  if (!parseOK){
    cerr << p.error() << endl
         << p.usage();
    return -1;
  }

  if (p.exist("dic")){
    if (buildFromFile(p) == -1){
      return -1;
    }
  } else {
    fujimap_tool::Fujimap fm;
    if (p.exist("seed") {
        fm.initSeed(p.get<int>("seed"));
    }
    if (fm.load(p.get<string>("index").c_str()) == -1){
      cerr << fm.what() << endl;
      return -1;
    }

    cout << "load done. " << endl;
    printCurrentStatus(fm);

    bool stringValue = p.exist("stringvalue");

    string key;
    for (;;){
      cout << ">" << flush;
      if (!getline(cin, key)){
        break;
      }
      if (stringValue){
        size_t len = 0;
        const char* sval = fm.getString(key.c_str(), key.size(), len);
        if (sval == NULL){
          cout << "NOTFOUND:" << endl;
        } else {
          cout << "FOUND:" <<  sval << endl;
        }
      } else {
        uint64_t code = fm.getInteger(key.c_str(), key.size());
        if (code == fujimap_tool::NOTFOUND){
          cout << "NOTFOUND:" << endl;
        } else {
          cout << "FOUND:" << code << endl;
        }
      }
    }
  }

  return 0;
}
