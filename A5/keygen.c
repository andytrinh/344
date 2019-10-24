#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  int length;
  int random;
  char key[80000];
  char list[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  if (argc < 2) {
    fprintf(stderr, "Not enough arguments.");
    return 1;
  }


  srand(time(NULL));
  length = atoi(argv[1]);
  int i;
  for(i = 0; i < length; i++){                 //Pick random capital letters. Put in space character where needed
       random = rand() % 27;
       key[i] = list[random];
       fprintf(stdout, "%c", key[i]);          //Write to stdout
   }

   fprintf(stdout, "\n");

   return 0;
}
