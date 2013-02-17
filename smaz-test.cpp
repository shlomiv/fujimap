
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smaz.h"

using namespace std;

int main(int argc, char* argv[]){
  char smaz[4000];
  char text[] = "assssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";//"how have you been";//"adjudicate <NUM>";
  char back[4000];
  int len = smaz_compress(text, sizeof(text)-1, smaz, sizeof(smaz));
  smaz_decompress(smaz, len, back, sizeof(back));
  cout << text << " = '" << back << "' " << smaz << " "<< len << " " << strlen(smaz)<< " " << strcmp(back, text) <<
endl;
}
