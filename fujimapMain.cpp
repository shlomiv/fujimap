#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "cmdline.h"
#include "fujimap.hpp"

using namespace std;

void printCurrentStatus(fujimap_tool::Fujimap& fm){
  size_t wsize = fm.getWorkingSize();
  cerr << "keyNum:       " << fm.getKeyNum() << endl
       << "fpLen:        " << fm.getFpLen() << endl
       << "encoding:     " << fm.getEncodeTypeStr() << endl
       << "seed:         " << fm.getSeed() << endl
       << "wsize(bytes): " << wsize / 8 << endl
       << "bits/key:     " << (float)wsize / fm.getKeyNum() << endl;
}


inline uint64_t decodeInteger(const unsigned char* pChar) {
  return (uint64_t)(pChar[0] & 0x7f) + (
                !(pChar[0] & 0x80) ? 0 : (((uint64_t)(pChar[1] & 0x7f)) << 7) + (
                !(pChar[1] & 0x80) ? 0 : (((uint64_t)(pChar[2] & 0x7f)) << 14) + (
                !(pChar[2] & 0x80) ? 0 : (((uint64_t)(pChar[3] & 0x7f)) << 21) + (
                !(pChar[3] & 0x80) ? 0 : (((uint64_t)pChar[4] << 28))))));
}

inline int intLength(uint64_t i){
  return i & 0xff0000000 ? 5 : i & 0xfe00000 ? 4 : i & 0x1fc000 ? 3 : i & 0x3f80 ? 2 : 1;
}

inline void encodeInteger(uint64_t i, unsigned char* pChar, int byteCount)
{
  switch (byteCount)  {
  case 1: *((uint64_t*)pChar) = i & 0x7f; break;
  case 2: *((uint64_t*)pChar) = (i & 0x7f) + ((i & 0x3f80) << 1) + 0x80; break;
  case 3: *((uint64_t*)pChar) = (i & 0x7f) + ((i & 0x3f80) << 1) + ((i & 0x1fc000) << 2) + 0x8080; break;
  case 4: *((uint64_t*)pChar) = (i & 0x7f) + ((i & 0x3f80) << 1) + ((i & 0x1fc000) << 2) + ((i & 0xfe00000) << 3) + 0x808080; break;
  case 5: *((uint64_t*)pChar) = (i & 0x7f) + ((i & 0x3f80) << 1) + ((i & 0x1fc000) << 2) + ((i & 0xfe00000) << 3) + ((i & 0xff0000000) << 4) + 0x80808080; break;
  }
}


int buildFromFile(cmdline::parser& p){
  ofstream compfs;
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
  if (p.exist("seed")) {
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

  bool hasCompanion = p.exist("companion");
  if (hasCompanion) {
    compfs.open(p.get<string>("companion").c_str());
    if (!compfs){
      cerr << "Unable to open for write " << p.get<string>("companion") << endl;
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
  long offset = 1;
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
      if (hasCompanion){

        unsigned char freq[6];
        unsigned  int intLen = intLength(val);
        encodeInteger(val, freq, intLen);

        //   cout << "comp " << line.c_str() << " " << freq << " "  << offset << endl;
        compfs.write(line.c_str(), p);
        compfs.write("\0", 1);
        compfs.write((char*)freq, intLen);

        fm.setInteger(line.c_str(), p, offset, false);
        offset += (p + intLen +1);
      }
      else {
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
    cerr << "could not save " << fm.what() << endl;
    return -1;
  }

  cerr << "save done." << endl;
  if (hasCompanion) {
    compfs.flush();
    compfs.close();
  }
  return 0;
}

int testFile(cmdline::parser& p, fujimap_tool::Fujimap* fm1, fujimap_tool::Fujimap* fm2){
  istream *pis=&cin;
  ifstream ifs;

  if (p.get<string>("test") != "-"){
    ifs.open(p.get<string>("test").c_str());
    if (!ifs){
      cerr << "Unable to open " << p.get<string>("test") << endl;
      return -1;
    }

    pis = static_cast<istream*>(&ifs);
  }

  istream &is=*pis;
  fujimap_tool::Fujimap fm;

  size_t readNum = 0;
  size_t collisions = 0;
  string line;
  for (size_t lineN = 1; getline(is, line); ++lineN){
    size_t p = line.length();
    if (p == 0) continue; // no key or no value

    uint64_t code1 = fm1->getInteger(line.c_str(), p);
    uint64_t code2 = fm2->getInteger(line.c_str(), p);

    if (code1 == fujimap_tool::NOTFOUND || code2 == fujimap_tool::NOTFOUND || code1 != code2 ){
    } else {
      cout << "FOUND collision '" << line.c_str() <<"'" <<  endl;
      collisions++;
    }

    readNum++;
  }
  cerr << "read " << readNum << " keys, " << collisions << " collisions" << endl;

  cerr << "test done. FP=" << (double)((double)collisions/(double)readNum) << endl;
  return 0;
}

int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("index",       'i', "Index",                                                        true);
  p.add<string>("index2",      'o', "Other Index",                                                  false);
  p.add<string>("test",        'T', "test file",                                                    false);
  p.add<string>("companion",   'c', "companion file. with -d the file is created, otherwise used.", false);
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
    fujimap_tool::Fujimap fm2;
    if (p.exist("seed")) {
        fm.initSeed(p.get<int>("seed"));
    }
    if (fm.load(p.get<string>("index").c_str()) == -1){
      cerr << fm.what() << endl;
      return -1;
    }
    cout << "load done. " << endl;
    printCurrentStatus(fm);

    int twoIndices = p.exist("index2");
    if (twoIndices) {
      cout << "loading second index.." << endl;
      if (fm2.load(p.get<string>("index2").c_str()) == -1){
        cerr << fm2.what() << endl;
        return -1;
      }
      cout << "load second index done. " << endl;
      printCurrentStatus(fm2);

    }

    if (p.exist("test")) {
      testFile(p, &fm, &fm2);
      return 0;
    }

    bool hasCompanion = p.exist("companion");
    long companionSize=0;
    ifstream compis;
    unsigned char *compData = NULL;
    if (hasCompanion) {
      compis.open(p.get<string>("companion").c_str(), ios::binary| ios::ate);
      if (!compis){
        cerr << "Unable to open for read " << p.get<string>("companoin") << endl;
        return -1;
      }
      companionSize = compis.tellg();
      cout << "loading companion of " <<  companionSize << " bytes" << endl;
      compData = new unsigned char [companionSize];
      compis.seekg (0, ios::beg);
      compis.read ((char*)compData, companionSize);
      compis.close();
    }

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

        timeval stop, start;
        gettimeofday(&start, NULL);

        uint64_t code1 = fm .getInteger(key.c_str(), key.size());
        uint64_t code2 = code1;

        if (twoIndices) {
          code2 = fm2.getInteger(key.c_str(), key.size());
        }

        if (code1 == fujimap_tool::NOTFOUND || code2 == fujimap_tool::NOTFOUND || code1 != code2 ){
          gettimeofday(&stop, NULL);
          cout << "NOTFOUND ("<<(stop.tv_usec - start.tv_usec)<<" micros): " << endl;
        } else {
          if (hasCompanion) {
            code1--;
            //  cout << "got offset " << code1 << "  " << (char*) (compData + code1) << endl;
            if (code1 < companionSize && strcmp((char*)(compData + code1), key.c_str())==0) {
              long v = decodeInteger(compData + code1 + key.size()+1);
              gettimeofday(&stop, NULL);
              cout << "FOUND ("<<(stop.tv_usec - start.tv_usec)<<" micros): " << v << endl;
            }
            else {
              gettimeofday(&stop, NULL);
              cout << "NOTFOUND ("<<(stop.tv_usec - start.tv_usec)<<" micros): " << endl;
            }
          } else {
            gettimeofday(&stop, NULL);
            cout << "FOUND ("<<(stop.tv_usec - start.tv_usec)<<" micros): " << code1 << endl;
          }
        }
        // sleep(1);

      }
    }
  }
  return 0;
}





  //do stuff
