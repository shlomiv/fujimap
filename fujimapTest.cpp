#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include "cmdline.h"
#include "fujimap.hpp"

using namespace std;

int buildFromFile(const cmdline::parser& p){
  const char* fn = p.get<string>("dic").c_str();
    
  fujimap_tool::Fujimap fm;
  fm.initFP(p.get<int>("fplen"));
  fm.initTmpN(p.get<int>("tmpN"));

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


  ifstream ifs(fn);
  if (!ifs){
    cerr << "Unable to open " << fn << endl;
    return -1;
  }

  map<string, uint32_t> keyValues; // for test
  string line;
  while (getline(ifs, line)){
    size_t p = line.find('\t');
    if (p == string::npos){
      cerr << "Warning: not tab found : " << line << endl;
      continue;
    }
    if (p == 0) continue;
    if (p+1 == line.size()) continue;
    uint64_t val = strtoll(line.substr(p+1).c_str(), NULL, 10);
    fm.setInteger(line.c_str(), p, val, true); // key is searchable immediately
    string key = line.substr(0, p);
    keyValues[key] = val;
  }

  cerr << "keyNum:" << fm.getKeyNum() << endl;

  int ret = fm.build(); 
  if (ret == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  if (fm.save(p.get<string>("index").c_str()) == -1){
    cerr << fm.what() << endl;
    return -1;
  }

  cerr << fm.getKeyNum() << endl;

  // test
  fujimap_tool::Fujimap fm2;
  if (fm2.load(p.get<string>("index").c_str()) == -1){
    cerr << fm2.what() << endl;
    return -1;
  }

  cerr << "load done." << endl;
  cerr << fm2.getKeyNum() << endl;

  int fnErrorN = 0;
  for (map<string, uint32_t >::const_iterator it = keyValues.begin();
       it != keyValues.end(); ++it){
    uint64_t ret = fm2.getInteger(it->first.c_str(), it->first.size());
    if (it->second != ret){
      fnErrorN++;
      cerr << "Error: [" << it->first << "]" << endl
      	   << "correct:" << it->second << " " << " incorrect:" << ret << endl;
      
    }
  }
  cerr << "fnErrorN:" << fnErrorN << endl;
  
  int fpErrorN = 0;
  for (int i = 0; i < 10000; ++i){
    ostringstream os;
    os << i << " " << i+1 << " " << i+2;
    uint64_t ret = fm2.getInteger(os.str().c_str(), os.str().size());
    if (ret != fujimap_tool::NOTFOUND){
      fpErrorN++;
    }
  }
  cerr << fpErrorN << "/" << 10000 << endl;

  return 0;
}


int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("dic", 'd', "dictionary", true, "");
  p.add<int>("fplen", 'f', "false positive rate 2^{-f} (0 <= f < 31) ", false, fujimap_tool::FPLEN);
  p.add<int>("tmpN", 't', "temporarySize", false, fujimap_tool::TMPN);
  p.add<string>("index", 'i', "index", true, "");
  p.add<string>("encode", 'e', "Code encoding  (=binary, gamma)", "binary");
  p.add("help", 'h', "print this message");
  p.set_program_name("fujimap_test");
  
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

  if (buildFromFile(p) == -1){
    return -1;
  }
  return 0;
}
