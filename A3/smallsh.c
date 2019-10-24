#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

void doCommands(char**, int*, struct sigaction, int*, char*, char*); //for non-built in commands
void catchSIGINT(int);
void catchSIGSTP(int);

int main() {
  char* arg[513];
  char* token;
  char* home = "HOME"; //set home variable
  pid_t spawnpid = -5;

  char input[2000];
  int checker = 1;
  int numArg = 0; //number of arguments
  int childExitMethod = 0; //initialize exit value
  int backgroundProcess;

  char* in = NULL;  //input file
  char* out = NULL; //output file
  int openOutFile = -1;
  int openInFile = -1;
  int pid = getpid();


  //signal handling
  struct sigaction SIGINT_action = {0}, SIGUSR2_action = {0}, ignore_action = {0}; //referenced from lecture notes

  SIGINT_action.sa_handler = catchSIGINT; //handles ^C
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;
  sigaction(SIGINT, &SIGINT_action, NULL);

  SIGUSR2_action.sa_handler = catchSIGSTP; //handles ^Z
  sigfillset(&SIGUSR2_action.sa_mask);
  SIGUSR2_action.sa_flags = 0;
  sigaction(SIGTSTP, &SIGUSR2_action, NULL);


  while(checker) {
    backgroundProcess = 0;
    printf(": ");
    fflush(stdout); //always flush after print

    if(fgets(input, 513, stdin) == NULL){  //get the user input
      return 0;
    }


    token = strtok(input, " \n"); //put the user input into token
    numArg = 0; //reset for each user input

    while(token != NULL) {
      if(strcmp(token, "<") == 0) { //handle input
        token = strtok(NULL, " \n");
        in = strdup(token);
        token = strtok(NULL, " \n");
      }
      else if(strcmp(token, ">") == 0) { //handle output
        token = strtok(NULL, " \n");
        out = strdup(token);
        token = strtok(NULL, " \n");
      }
      else if(strcmp(token, "&") == 0) {
        backgroundProcess = 1; //set background processes if '&' is present
        break;
      }
      else { //parse each argument into string array referenced: http://www.cplusplus.com/reference/cstring/strtok/
          arg[numArg] = strdup(token);
          int index;
          for (index=0; arg[numArg][index]; index++) {//replace "$$" with pid
            if((arg[numArg][index] == '$') && (arg[numArg][index+1] == '$')){
              arg[numArg][index] = '\0'; //remove '$$' in the command
              sprintf(arg[numArg], "%s%d", arg[numArg], pid); //add the pid at the end
            }
          }
          token = strtok(NULL, " \n");
          numArg++; //increment to read next argument
      }
    }
    arg[numArg] = NULL; //reset array


  if(*(arg[0]) == '#') { //compares the first character to "#"
    continue;
  }
  else if(*(arg[0]) == '\0') { //compares the first character to "#"
    continue;
  }
  else if(strcmp(arg[0], "exit") == 0) {//exit program
    if(arg[1] == NULL) {
      exit(0);
    }
    else {
      printf("Unexpected Arguments\n");
      fflush(stdout);
      exit(0);
    }
  }

  else if(strcmp(arg[0], "cd") == 0) {//changing directories
    if(arg[1] == NULL) {
      chdir(getenv(home)); //go to home
    }
    else {
      chdir(arg[1]); //go to directory of the next argument
    }
  }
  else if (strcmp(arg[0], "status") == 0) {//print exit value
    if(WIFEXITED(childExitMethod) != 0) {
      printf("exit value %d\n", WEXITSTATUS(childExitMethod));
      fflush(stdout);
    }
    else {
      printf("Terminated by signal %d\n", childExitMethod);
      fflush(stdout);
    }
  }
  else { //handling exec
    doCommands(arg, &childExitMethod, SIGINT_action, &backgroundProcess, in, out);

    //delete contents of the argay
    int i;
    for(i = 0; arg[i] != NULL; i++){
           free(arg[i]);
       }
    in = NULL;
    out = NULL;
  }

  }
  return 0;
}

int background = 1;
void catchSIGINT(int signo) {
//  char* message = signo;
  printf("Terminated by signal %d\n", signo);
  fflush(stdout);
}

void catchSIGSTP(int signo) {
  if (background == 1) {
    char* message = "Entering foreground-only mode (& is now ignored)\n";
    write(1, message, 50);
    fflush(stdout);
    background = 0;
  }
  else {
    char* message = "Exiting foreground-only mode\n";
		write (1, message, 30);
		fflush(stdout);
		background = 1;
  }
}


void doCommands(char** arg, int* childExitMethod, struct sigaction SIGINT_action, int* backgroundProcess, char* in, char* out) {
  pid_t spawnpid = -5;
  int openInFile;
  int openOutFile;
  //printf("\nbackgroundProcess: %d\n", *backgroundProcess);
  //exit(1);
  spawnpid = fork(); // fork off a child
  switch(spawnpid) {
    case -1:
      perror("Hull Breach!");
      *childExitMethod = 1; //set status
      break;
    case 0:
      if(backgroundProcess == 0){
        SIGINT_action.sa_handler = SIG_DFL; //set ^c as default if there aren't any background processes
        sigaction(SIGINT, &SIGINT_action, NULL);
      }
      if(in != NULL){ //handles opening input files
          openInFile = open(in, O_RDONLY);
          if(openInFile == -1) {
            fprintf(stderr, "cannot open %s for input\n", in);
            fflush(stdout);
            exit(1);
          }
          if(dup2(openInFile, 0) == -1){
            perror("Error with input file");
            exit(1);
          }
          close(openInFile); //close because it was opened
      }
      else if(*backgroundProcess == 1){
          openInFile = open("/dev/null", O_RDONLY); //standard input redirected from /dev/null
          if(openInFile == -1){
              perror("open");
              _exit(1);
          }
          if(dup2(openInFile, 0) == -1){
              perror("Error with input file");
              _exit(1);
          }
      }

      if(out != NULL){ //handle opening output files
          openOutFile = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if(openOutFile == -1) {
              perror("open()");
              fprintf(stderr, "cannot open %s for output\n", out);
              fflush(stdout);
              _exit(1);
          }
          if(dup2(openOutFile, 1) == -1){
              perror("dup2");
              _exit(1);
          }
          close(openOutFile); //close because it was opened
      }


      if(execvp(arg[0], arg)){ // handles none built in commands
         printf("%s: no such file or directory\n", arg[0]);
         fflush(stdout);
         exit(1);
      }
      break;

    default:
      if ((*backgroundProcess != 0) && (background != 0)){ //do if there are background processes
      	pid_t pidNow = waitpid(spawnpid, childExitMethod, WNOHANG);
				printf("background pid is %d\n", spawnpid);
				fflush(stdout);
			}
			else { //no background processe
				pid_t pidNow = waitpid(spawnpid, childExitMethod, 0);
			}

      spawnpid = waitpid(-1, childExitMethod, WNOHANG); //referenced lecture: "Check if any process has completed, return immediately with 0 if none have"
      while(spawnpid > 0){
          printf("background pid %d is done: ", spawnpid);
          if(WIFEXITED(*childExitMethod) != 0) {
            printf("exit value %d\n", WEXITSTATUS(*childExitMethod));
            fflush(stdout);
          }
          else {
            printf("Terminated by signal %d\n", (*childExitMethod));
            fflush(stdout);
          }
          spawnpid = waitpid(-1, childExitMethod, WNOHANG);
      }
  }
}
