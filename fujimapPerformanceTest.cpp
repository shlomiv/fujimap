#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <map>
#include <cstring>
#include <cstdlib>
#include "fujimap.hpp"

using namespace std;

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

double gettimeofday_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void run_fujimap(const uint64_t N){
  fujimap_tool::Fujimap fm;
  fm.initFP(0);
  fm.initTmpN(0);

  char buf[100];
  double t1 = gettimeofday_sec();
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    fm.setInteger(buf, strlen(buf), i, false);
  }
  fm.build();
  double t2 = gettimeofday_sec();
  fprintf(stderr, "fm  set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

  int dummy = 0;
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    dummy += fm.getInteger(buf, strlen(buf));
  }
  double t3 = gettimeofday_sec();
  fprintf(stderr, "fm  lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);
  cerr <<         "fm    size: " << fm.getWorkingSize() << endl;

  if (dummy == 77777){
    fprintf(stderr, "you're lucky!");
  }

}

void run_map(const uint64_t N){
  map<string, int> m;

  char buf[100];
  double t1 = gettimeofday_sec();
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    m[string(buf)] = i;
  }
  double t2 = gettimeofday_sec();
  fprintf(stderr, "map set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

  int dummy = 0;
  for (uint64_t i = 0; i < N; ++i){
    snprintf(buf, 100, "%llu", (unsigned long long int)i);
    dummy += m[string(buf)];
  }
  double t3 = gettimeofday_sec();
  fprintf(stderr, "map lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);

  if (dummy == 77777){
    fprintf(stderr, "you're lucky!");
  }

  
}


int main(int argc, char* argv[]){
  static uint64_t N = 100000000;
  if (argc == 2){
    N = strtoll(argv[1], NULL, 10);
  }


  run_fujimap(N);
  run_map(N);
  return 0;
}
