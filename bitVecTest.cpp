#include <iostream>
#include <vector>
#include <cstdlib>
#include "bitVec.hpp"
#include "fujimapCommon.hpp"

using namespace std;

int main(int argc, char* argv[]){
  
  const uint64_t n = 1000000;
  vector<uint32_t> v;
  vector<uint32_t> sv;

  BitVec bv(n);
  for (uint64_t i = 0; i < n; ++i){
    int b = rand() % 2;
    v.push_back(b);
    if (b){
      sv.push_back(i);
      bv.setBit(i);
    }
  }

  // check
  cerr << "getBit test" << endl;
  for (uint64_t i = 0; i < n; ++i){
    if (v[i] != bv.getBit(i)){
      cerr << "error i:" << i << " v[i]:" << v[i] << " bv.getBit(i):" << bv.getBit(i) << endl;
      return -1;
    }
  }
  cerr << "done" << endl;

  cerr << "getBits test" << endl;
  for (uint64_t i = 0; i < n; ++i){
    uint32_t code = 0;
    for (uint32_t j = 0; j < 10; ++j){
      code |= (bv.getBit(i + j)) << j;
    }
    uint32_t code2 = bv.getBits(i, 10);
    if (code != code2) {
      cerr << "error i:" << i << " code:" << code <<" getBits:" << code2 << endl;
      for (int j = 0; j < 10; ++j){
	cerr << ((code >> j) & 1U) <<  " " << ((code2 >> j) & 1U) << endl;
      }
      return -1;
    }
  }

  BitVec bv2(n);
  for (uint64_t i = 0; i < n; ++i){
    bv2.setBit(i);
  }

  size_t offset = 0;
  for (int i = 0; i < 63; ++i){
    if ((i % 2) == 0) {
      bv2.setBits(offset, i+1, 0);
    }
    offset += (i+1);
  }

  offset = 0;
  for (int i = 0; i < 63; ++i){
    uint64_t bits = bv2.getBits(offset, i+1);
    fujimap_tool::printBit(bits, i+1);
    offset += (i+1);
  }
  cerr << "done" << endl;
  

  return 0;
}
