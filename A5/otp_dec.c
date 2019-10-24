#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char* argv[]){
    int socketFD, establishedConnectionFD, portNumber, charsWritten, charsRead, yes = 1;
    socklen_t sizeOfClientInfo;
    char buffer[80000];
    struct sockaddr_in serverAddress, clientAddress;
    struct hostent* serverHostInfo;
    char check[] = "dec";

    if (argc < 4) {
      fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
      exit(0); //check usage & args
    }

    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET;     //Create network-capable socket
    serverAddress.sin_port = htons(portNumber);     //Store port number
    serverHostInfo = gethostbyname("localhost");    //Convert machine name into address into a special form of address
    if(serverHostInfo == NULL){
        fprintf(stderr, "CLIENT: ERROR, no such host\n");
        exit(0);
    }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address


    //Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the socket
    if(socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }
  //  setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // Connect to server
    if(connect(socketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
        error("CLIENT: ERROR connecting");
    }

    // Send message to server
    charsWritten = send(socketFD, check, strlen(check), 0); // Write to the server telling it whether or not we can accept
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end



    if(strcmp(buffer, "dec") == 0){
        ;
    }
    else{
      fprintf(stderr, "ERROR, cannot use that port.\n");
      exit(2);
    }

    int c2; //holder to count each character
    int plainTextCount = 0, plainTextLength = 0;
    int plainTextFile = atoi(argv[1]); //get the plaintext filename
    FILE *fp;
    fp = fopen(argv[1], "r");
    while((c2 = fgetc(fp)) != EOF) {
      plainTextCount++;
      }
      plainTextLength = plainTextCount; //get the number of characters in plaintext file

    int c; //holder to count each character
    int keyCount = 0, keyLength = 0;
    int keyFile = atoi(argv[2]); //get the key filename
    FILE *fptr;
    fptr = fopen(argv[2], "r");
    while((c = fgetc(fptr)) != EOF) {
	     keyCount++;
	  }
    keyLength = keyCount; //get size of key

    //check for key that is too short
    if(keyLength < plainTextLength){    //error handling for keys that are too short
        fprintf(stderr, "Key length is too short.\n");
        exit(1);
    }

    //check for invalid characters
    int i, x = 0;
    char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char charCounter = open(argv[1], 'r'); //open plain text file for reading
    while(read(charCounter, buffer, 1) != 0){
        for (i = 0; i < 27; i++) {
          if (buffer[0] == alphabet[i] || buffer[0] == ' ') { //check if file contents is valid
            x = 1;
          }
        }

        if(!x) {
          fprintf(stderr, "%s has invalid characters!\n", argv[1]); //if contents of file aren't letters and spaces print error
          exit(1);
        }

    }

    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

    //send contents of plaintext
    FILE* fileText = fopen(argv[1], "r"); //open plaintext
    int charsWritten1 = 0;
    char *plainTextMessage = malloc(sizeof(char) * 80000); //create space for message
    memset(plainTextMessage, '\0', 80000); //set to null
    size_t bufsize = 0; //used to for getline

    plainTextLength = getline(&plainTextMessage, &bufsize, fileText); //get contents of plaintext
    plainTextMessage[strcspn(plainTextMessage, "\n")] = '@'; //replace new line with '@' so server knows when to stop reading

    //send ALL contents of plaintext
    do {
      charsWritten1 = charsWritten1 + send(socketFD, plainTextMessage, plainTextLength, 0);
    } while (charsWritten1 > plainTextLength);

    //send contents of keyfile
    FILE* fileKey = fopen(argv[2], "r");
    int charsWritten2 = 0;
    int checkSend = -5;  // Bytes remaining in send buffer

    do
    {
      ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
      //printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are:
    } while (checkSend > 0);

    char *keyMessage = malloc(sizeof(char) * 80000);
    memset(keyMessage, '\0', 80000);

    bufsize = 2;

    keyLength = getline(&keyMessage, &bufsize, fileKey); //get contents of plaintext
    keyMessage[strcspn(keyMessage, "\n")] = '@'; //replace new line with '@' so server knows when to stop reading

    //send ALL contents of plaintext
    do {
      charsWritten2 = charsWritten2 + send(socketFD, keyMessage, keyLength, 0);
    } while (charsWritten2 > keyLength);


    //decrypted message that will be sent back to clients
    char completeMessage[80000];
    memset(completeMessage, '\0', sizeof(completeMessage));


    while (strstr(completeMessage, "@") == NULL) // As long as we haven't found the terminal...
    {
        memset(buffer, '\0', sizeof(buffer)); // Clear the buffer
        charsWritten = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Get the next chunk
        strcat(completeMessage, buffer); // Add that chunk to what we have so far

    }

    int terminalLocation3 = strstr(completeMessage, "@") - completeMessage; // Where is the terminal
    completeMessage[terminalLocation3] = '\n';

    if(charsRead < 0){
        error("CLIENT: ERROR from reading socket");
    }
    printf("%s", completeMessage);                     //Print and cleanup
    close(socketFD);

    return 0;
}
