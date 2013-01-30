#include "fujimapCommon.hpp"
#include "bitVec.hpp"
#include <iostream>

using namespace std;
using namespace fujimap_tool;

int main(){
  for (uint64_t i = 1; i < (1LLU << 32); ++i){
    uint64_t len = gammaLen(i);
    uint64_t code = 0;
    for (uint64_t j = 0; j < len; ++j){
      code |= (gammaEncodeBit(j, i) << j);
    }
    
    uint64_t ret = gammaDecode(code);
    if (i != ret){
      cerr << "error x:" << i << " code:" << gammaDecode(code) << endl;
      return -1;
    }
    if ((i % 10000) == 0) cerr << i << endl;
  }
  return 0;
}


