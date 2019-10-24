#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>

int isValidName(char* name);

char* availableRoomName[10] = {"PARK", "DUNGEON", "SCHOOL", "HOUSE", "LAKE", "RIVER", "CABIN", "TRASHCAN", "GAMESTOP", "GYM"};

struct Room {
    char* name;
    char* type;
    int connections[7];
    int numberConnections;
};

/*get the current time*/
void* currentTime(){
  FILE* fp;
  fp = fopen("currentTime.txt", "w+"); /* create txt file if it does not exist else write to it*/
  char buffer[100];
  struct tm *sTm;

  time_t current = time (0);
  sTm = gmtime (&current);

  time ( &current );
  sTm = localtime ( &current ); /*get the right time*/
  strftime(buffer, 100, "%I:%M%p, %A, %B %d, %Y", sTm);
  fputs(buffer, fp);
  fclose(fp);

}

char* getDirectoryName() {
  /* referenced from notes: https:/*oregonstate.instructure.com/courses/1725991/pages/2-dot-4-manipulating-directories*/
  int newestDirTime = -1; /* Modified timestamp of newest subdir examined*/
  char targetDirPrefix[32] = "trinha.rooms."; /* Prefix we're looking for*/
  //char newestDirName[256]; /* Holds the name of the newest dir that contains prefix*/
  char* newestDirName = malloc(sizeof(char) * 20);
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck; /* Holds the directory we're starting in*/
  struct dirent *fileInDir; /* Holds the current subdir of the starting dir*/
  struct stat dirAttributes; /* Holds information we've gained about subdir*/

  dirToCheck = opendir("."); /* Open up the directory this program was run in*/

  if (dirToCheck > 0) /* Make sure the current directory could be opened*/
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) /* Check each entry in dir*/
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) /* If entry has prefix*/
      {
    /*    printf("%s\n", fileInDir->d_name);*/
        stat(fileInDir->d_name, &dirAttributes); /* Get attributes of the entry*/

        if ((int)dirAttributes.st_mtime > newestDirTime) /* If this time is bigger*/
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
        //  return fileInDir->d_name; /* return newest directory name*/
        }
      }
    }
  }
  closedir(dirToCheck); /* Close the directory we opened*/
  return newestDirName;

}

/*purpose is to get the name of the room with the type: start*/
char* getStartRoomName() {
 char* directory = getDirectoryName();

 char* first = malloc(sizeof(char) * 20); /*allocate space for name of file*/

 DIR* folder;
 struct dirent* find;

  folder = opendir(directory); /* open directory*/

  char wholeLine[100];
  char beginningWholeLine[100];

   while((find = readdir(folder)) != NULL){ /*read through all files in directory*/


         if(!strcmp(find -> d_name, ".")) { /*skip*/
          continue;
         }
         else if (!strcmp(find -> d_name, "..")) { /*skip*/
           continue;
         }

         /*open all files in directory*/
         char filepath[100];
         char* directoryName = getDirectoryName();
         char* filename = find->d_name; /*name of file*/
         sprintf(filepath, "%s/%s", directoryName , filename);
         char* file = filename;
         FILE* stream;
         stream = fopen(filepath, "r");

         while(fgets(wholeLine, 100, stream) != NULL){ /* reads until the end of file*/

         if (strstr(wholeLine, "ROOM TYPE: START_ROOM") != 0) { /*if line is the same*/
           /*reopen file to get the first line*/
           sprintf(filepath, "%s/%s", directoryName , filename);
           char* file = filename;
           FILE* stream;
           stream = fopen(filepath, "r");
           fgets(beginningWholeLine, 100, stream); /*get the first line*/

           char garbage[100];
           
           char* name = malloc(sizeof(char) * 20);
           sscanf(beginningWholeLine, "%s %s %s", garbage, garbage, name); /* parse the line to get the name*/
           return name;
        }
}
       }
       closedir(folder);
}


void readStartRoom(struct Room R[7]) {
  char* startFileName = getStartRoomName(); /*firstroom*/

  sprintf(startFileName,"%s_room", startFileName); /*add _room suffix*/

  char filepath[100];
  char* directoryName = getDirectoryName();

  sprintf(filepath, "%s/%s", directoryName, startFileName); /*create file path*/

  char* file = startFileName;
  FILE* stream;
  stream = fopen(filepath, "r"); /* open the file with the room type start*/

  if (stream == 0) {
    fprintf(stderr, "Could not open %s\n", file);
    exit(0);
  }

  char wholeLine[100];
  fgets(wholeLine, 100, stream);

  char* name = malloc(sizeof(char) * 20); /* allocate space for the name*/
  char garbage[100];
  sscanf(wholeLine, "%s %s %s", garbage, garbage, name); /*name gets the name*/
  R[0].name = name; /*set name of first room*/

  fclose(stream);
}

void printConnection(char* roomName){ /*print all possible connections of the room*/
  char filepath[100];
  char* directoryName = getDirectoryName();

  sprintf(filepath, "%s/%s", directoryName, roomName); /*create file path*/

  char* file = roomName;
  FILE* stream;
  stream = fopen(filepath, "r"); /*open file*/

  char wholeLine[100];
  char garbage[100];
  char **connectionName = malloc(6 * sizeof(char *)); /*dynamic array of strings to hold connections*/
  int i;
  for (i = 0; i < 6; ++i) {
      connectionName[i] = (char *)malloc(20+1);
  }

  int tracker = 0; /*number of connections*/
  while(fgets(wholeLine, 100, stream) != NULL){ /* reads til the end of file*/
    if (strstr(wholeLine, "CONNECTION ") != 0) {
      char* name = malloc(sizeof(char) * 20);
      sscanf(wholeLine, "%s %s %s", garbage, garbage, name); /*name gets the name*/
      strcpy(connectionName[tracker], name); /*copy data over to array*/
      tracker++;
    }
  }


  /*loop to print all connections*/
  int j;
  printf("POSSIBLE CONNECTIONS: ");
  for(j=0;j<tracker;j++) {
    printf("%s", connectionName[j]);

    if(j<(tracker-1)){
      printf(", ");
    }
    else
    printf(".\n");
    }


  fclose(stream);



}

/*returns true if name exists*/
int isValidName(char* name) {
  sprintf(name, "%s_room", name);
  int checker = 1;
  char* directory = getDirectoryName();
  char* first = malloc(sizeof(char) * 20); /*allocate space to hold name*/

  DIR* folder;
  struct dirent* find;

   folder = opendir(directory);

   char wholeLine[100];
   char beginningWholeLine[100];

    while((find = readdir(folder)) != NULL){
          if(!strcmp(find -> d_name, ".")) { /*skip*/
           continue;
          }
          else if (!strcmp(find -> d_name, "..")) { /*skip*/
            continue;
          }
      char filepath[100];
      char* directoryName = getDirectoryName();
      char* filename = find->d_name; /*name of file*/
      sprintf(filepath, "%s/%s", directoryName , filename);
      char* file = filename;
      FILE* stream;
      stream = fopen(filepath, "r");


      if(strcmp(filename, name) == 0) { /* if the strings match*/
        return 1;
      }
      else if (strcmp(filename, name) != 0) { /* if strings don't match*/
        checker = -1;
      }

    }
    if (checker != 1) {
      return -1;
    }

}

int checkWinner(char* name, int step) { /*if the room file has room type end_room print victory message*/
  char filepath[100];
  char* directoryName = getDirectoryName();

  sprintf(filepath, "%s/%s", directoryName, name); /*create file path*/

  char* file = name;
  FILE* stream;
  stream = fopen(filepath, "r");
  char wholeLine[100];
  while(fgets(wholeLine, 100, stream) != NULL){ /* reads til the end of file*/
    if (strstr(wholeLine, "ROOM TYPE: END_ROOM") != 0) {
      printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS! YOU TOOK %d STEPS. ", step);
      break;
    }

}

}

void Game(struct Room R[7]) {
  readStartRoom(R);

  printf("CURRENT LOCATION: %s\n", R[0].name);
  char* startRoom = malloc(sizeof(char) * 20); /*just name*/

  strcpy(startRoom, R[0].name);
  sprintf(startRoom,"%s_room", startRoom);
  printConnection(startRoom);

  char **pathTaken = malloc(6 * sizeof(char *)); /*string of arrays to hold path taken*/
  int i;
  for (i = 0; i < 6; ++i) {
      pathTaken[i] = (char *)malloc(20+1);
  }

  int foo = 0;
  int checker = 1;
  int steps = 0;
  int a = 0;


  while (checker) {
    char* name = malloc(sizeof(char) * 20);
    char* name2 = malloc(sizeof(char) * 20); /*to hold name without the _room suffix*/
    printf("WHERE TO? >");
    scanf("%s", name); /*USER INPUT*/

    strcpy(name2,name);


      if(isValidName(name) != -1) {
        printf("\nCURRENT LOCATION: %s\n\n", name2);

        pathTaken[a] = name2; /*hold each path taken*/
        a++; /*increment to next index*/
        steps++; /*increment steps taken*/
        int win; /*used to see if*/
        win = checkWinner(name, steps); /*returns 1 if reached end room*/

          if(win){
            printf("YOUR PATH TO VICTORY WAS:\n");
            int index;
            for (index = 0; index < a; index++) {
              printf("%s\n", pathTaken[index]);
            }
            exit(0);
          }

          else {
            printConnection(name);
          }
      }
      else if(strcmp(name, "time_room") == 0) { /*input "name" is converted with _room*/
        pthread_mutex_t mutex;

        pthread_t threaded;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);

        int thread = pthread_create(&thread, NULL, currentTime, NULL);
        pthread_mutex_unlock(&mutex); /* unlock the mutex */
        pthread_mutex_destroy(&mutex);
        usleep(50); /*for thread to gain lock*/

        FILE* file = fopen("currentTime.txt", "r"); /*create file if doesn't exist*/
        char buff[100];
        fgets(buff, 100, file); /*read line in the file into char array buff*/
        printf("%s\n", buff); /*print the line in the file*/
        fclose(file);
      }
      else {
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n"); /*if invalid name*/
        checker = 1; /* loop back up*/
      }
  }
}




 int main() {
   struct Room R[7];
   Game(R);
   return 0;
 }
