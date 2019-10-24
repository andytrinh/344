#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

char* availableRoomName[10] = {"PARK", "DUNGEON", "SCHOOL", "HOUSE", "LAKE", "RIVER", "CABIN", "TRASHCAN", "GAMESTOP", "GYM"};
int outboundConnections[7][7]; /*0 for no connection, 1 for connection*/



struct Room { /*used to contain the necessary info (not connections)*/
    char* name;
    char* type;
    int numberConnections;
};


void swap(int *a, int *b) { /*used inside randomize*/
    int temp = *a;
    *a = *b;
    *b = temp;
}

void randomize(int arr[], int n) { /*used to switched up array where n is size of array*/
    srand(time(NULL));
    int i;
    for(i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        swap(&arr[i], &arr[j]);
    }
}

void createRoom(struct Room R[7]) {
  int i, ran;
  int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  randomize(arr, 10); /*randomize index*/
  int j;
  for (i = 0; i < 7; i++) {
    int index = arr[i];
    R[i].name = availableRoomName[index];/*assign name to random index of room names*/

  /*assign types*/
  if(i == 4) { /*chose random int 0-6*/
    R[i].type = "START_ROOM"; /*1 start room*/
  }
  else if(i == 2) { /*random int again*/
    R[i].type = "END_ROOM"; /*1 end room*/
  }
  else { /* rest of the rooms*/
    R[i].type = "MID_ROOM"; /*5 mid rooms*/
  }
}


/*assign number of connections*/
int index1;
for (index1 = 0; index1 < 7; index1++) {
  int random_num_connections = rand() % (6+1-3) + 3; /*random number from 3-6*/
  R[index1].numberConnections = random_num_connections;

}
  connectRooms(R); /* make the connection in the outboundConnections array*/
}



void connectRooms(struct Room R[7]) {
  int i;
  int j;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < R[i].numberConnections; j++) {
      int random = rand() % 7;
      if (random == i) { /*so that a connection to itself can't be made*/
        random++;
      }
      /*1 = connection, 0 = no connection*/
      outboundConnections[i][random] = 1; /*set random connection*/
      outboundConnections[random][i] = 1; /*must connect both ways*/

      random++; /* increment*/

      if (random > 7) { /*reset once random is greater than 7*/
        random = 0;
      }
    }
  }

}

/*used for testing(prints the struct)*/
void printStruct(struct Room R[7]) {
  int i;
  int j;
  for (i = 0; i < 7; i++) {
    printf("ROOM NAME: %s\n", R[i].name);
    printf("ROOM TYPE: %s\n", R[i].type);
    printf("NUMBER OF CONNECTIONS: %d\n", R[i].numberConnections);
  }
}

void makeFiles(struct Room R[7]){

  /*make directory to hold files*/
  char directoryName[100];
  int pid_ID = getpid();
  sprintf(directoryName, "trinha.rooms.%d", pid_ID);
  mkdir(directoryName, 0755);
  chdir(directoryName); /* change working directory*/

  /*make files*/
  int i;
  for(i = 0; i < 7; i++) {
    char name[100];
    sprintf(name,"%s_room", R[i].name);
    FILE* roomFile = fopen(name, "w+"); /*create room*/
    fprintf(roomFile, "ROOM NAME: %s\n", R[i].name); /*write room name*/
    int j;
    int n = 0;
    for(j = 0; j < 7; j++){
        if (outboundConnections[i][j] == 1){ /*set connection if 1*/
          n++;
          fprintf(roomFile, "CONNECTION %d: %s\n", n, R[j].name); /*write connection if there is one*/
      }
    }
      fprintf(roomFile, "ROOM TYPE: %s\n", R[i].type); /*write room type*/
      fclose(roomFile);
   }
}



int main() {
  struct Room R[7];
  createRoom(R);
  makeFiles(R);

/*
  int i,j;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < 7; j++) {
    printf("Room Name: %s | outboundConnections: [%d][%d] = %d\n", R[i].name ,i, j, outboundConnections[i][j]);
    }
  }
*/
  exit(0);
  return 0;
}
