#include <iostream>
#include <cassert>
#include "keyFile.hpp"

using namespace std;
using namespace fujimap_tool;

const char* testfn   = "test.kf";
const uint64_t MAXID = 128;
const uint64_t N     = 1000000;

int main(int argc, char* argv[]){
  KeyFile kf;
  if (kf.initWorkingFile(testfn) == -1){
    return -1;
  }

  kf.initMaxID(MAXID);

  cerr << "estimated key num:" << N / MAXID << endl;
  for (uint64_t i = 0; i < N; ++i){
    char buf[100];
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    string bufs(buf);
    kf.write(i % MAXID, bufs.c_str(), bufs.size(), i); 

    if ((i+1) % 10000 == 0){
      cerr << ".";
    }
  }
  cerr << endl;

  for (uint64_t i = 0; i < MAXID; ++i){
    vector<pair<string, uint64_t> > kvs;
    if (kf.read(i, kvs) == -1){
      cerr << "read " << i << " error" << endl;
      return -1;
    }
    for (size_t j = 0; j < kvs.size(); ++j){
      if ((kvs[j].second % MAXID) != i){
	cerr << "error:" << kvs[j].second << endl;
	assert(false);
      }
    }
  }

  return 0;
}
