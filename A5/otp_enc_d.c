#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int convertChartoNum (char c){
	char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	int i;
	for(i = 0; alphabet[i] != '\0'; i++){
		if (c == alphabet[i]) {
			return i;
		}
	}
	return -1;
}

int main(int argc, char* argv[]){
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
		socklen_t sizeOfClientInfo;
    char buffer[80000];
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
		int yes = 1;

    if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

    setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[1]);    // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber);  // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;  // Any address is allowed for connection to this process

		// Set up the socket
		listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); //Create the socket
		if(listenSocketFD < 0){
				error("Error opening socket");
		}

		// Enable the socket to begin listening
    if(bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
        error("Error on binding");
    }

    listen(listenSocketFD, 5);  // Flip the socket on - it can now receive up to 5 connections

		// Accept a connection, blocking if one is not available until one connects
    while(1){
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *) &clientAddress, &sizeOfClientInfo);
        if(establishedConnectionFD < 0){
            error("ERROR on accept");
        }
				int status;
				pid_t spawnpid = -5;
        spawnpid = fork();
				switch(spawnpid) {   // fork off a child
					case -1: {
            perror("Forking error.");
            exit(1);
						break;
        }
					case 0: {
            int i, charsWritten, size;
            char *key;
            size = sizeof(buffer);
            int charsRead = 0;
            int holder = 0;

						memset(buffer, '\0', sizeof(buffer));

						read(establishedConnectionFD, buffer, sizeof(buffer) - 1); //get info from client to see if we can encrypt

						if(strcmp(buffer, "enc") != 0){
							 char output[] = "invalid";
							 write(establishedConnectionFD, output, sizeof(output));// let client know if invalid
							 exit(1);
					 }
					 else {
            char output[] = "enc";
						charsWritten = send(establishedConnectionFD, output, sizeof(output), 0); //send message that it is accepted
						if (charsWritten < 0) {
							error("ERROR writing to socket");
						}
				 		}

						// Get the message from the client
						memset(buffer, '\0', sizeof(buffer));
					  char* pointer = buffer;

						//send ALL contents of plaintext
						char textMessage[80000], buffer[10];
						memset(textMessage, '\0', sizeof(textMessage)); // Clear the buffer
						while (strstr(textMessage, "@") == NULL) // As long as we haven't found the terminal...
						{
								memset(buffer, '\0', sizeof(buffer)); // Clear the buffer
								charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); // Get the next chunk
								strcat(textMessage, buffer); // Add that chunk to what we have so far
						}
						int terminalLocation = strstr(textMessage, "@") - textMessage; // Where is the terminal
						textMessage[terminalLocation] = '\0'; // End the string early to wipe out the terminal


						//send contents of keyfile
						char keyMessage[80000], buffer2[80000];
						memset(keyMessage, '\0', sizeof(keyMessage)); // Clear the buffer
						while (strstr(keyMessage, "@") == NULL) // As long as we haven't found the terminal...
						{
								memset(buffer2, '\0', sizeof(buffer2)); // Clear the buffer
								charsWritten = recv(establishedConnectionFD, buffer2, sizeof(buffer2) - 1, 0); // Get the next chunk
								strcat(keyMessage, buffer2); // Add that chunk to what we have so far
							//	printf("PARENT: Message received from child: \"%s\", total: \"%s\"\n", buffer, textMessage);
						//		if (charsWritten == -1) { printf("r == -1\n"); break; } // Check for errors
						//		if (charsWritten == 0) { printf("r == 0\n"); break; }
						}
						int terminalLocation2 = strstr(keyMessage, "@") - keyMessage; // Where is the terminal
						keyMessage[terminalLocation2] = '\0'; // End the string early to wipe out the terminal

						char m, k; //used to get each char in plaintext and key
						//encrypt the message
						int sum = 0, valueText = 0, valueKey = 0, j;
						char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
						int l = (strlen(textMessage) - 1);
						for (i = 0; i < l; i++) {

							m = textMessage[i];
							k = keyMessage[i];

							//convert the chars of plain text and key into ints
							valueText = convertChartoNum(m);
							valueKey = convertChartoNum(k);


							sum = valueText + valueKey; //add message and key to sum
							sum = sum % 27; //to put in the cipher text file
							textMessage[i] =  alphabet[sum]; // convert to char

						}
						//textMessage[i] = '\0'; //add ending character at the end of message

						int messagelen = strlen(textMessage); //get length of complete message

						charsWritten = 0;
						do {
							charsWritten = charsWritten + send(establishedConnectionFD, textMessage + charsWritten, messagelen - charsWritten, 0); //send message to client
						} while (charsWritten < messagelen);


				charsWritten = send(establishedConnectionFD, "@", strlen("@"), 0); // send null symbol

				}

				default:
				 waitpid(-1, &status, WNOHANG);

        close(establishedConnectionFD);     // Close the existing socket which is connected to the client
    }
	}
    close(listenSocketFD);  // Close the listening socket
    return 0;
}
