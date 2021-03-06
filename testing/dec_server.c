// dec_server.c

// https://stackoverflow.com/questions/1568786/fork-before-or-after-accepting-connections

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

char decode_char(char msg, char key) {
    if (msg == '\n') return '\n';
    if (msg == '\0') return '\0';
    int msg_int;
    int key_int;
    if (msg == ' ') msg_int = 0;
    else msg_int = msg - 64;
    // printf("%c = %d\n", msg, msg_int);
    if (key == ' ') key_int = 0;
    else key_int = key - 64;
    // printf("%c = %d\n", key, key_int);
    msg_int = (msg_int - key_int) % 27;
    if (msg_int < 0) msg_int = msg_int + 27;
    if (msg_int == 0) return ' ';
    else return msg_int + 64;
}


int main(int argc, char *argv[]){
  int connectionSocket, charsRead, newLine;
  char buffer_txt[256];
  char buffer_key[256];
  char next_char;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // set handshake messages
  char dec_handshake_client[36] = "I am requesting decoding service.\n\0";
  char dec_handshake_server[25] = "Proceed with decoding.\n\0";

  // Check usage & args
  if (argc < 2) {
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connections. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  int number_of_children = 0;

  for (size_t j = 0; j < 4; j++) {
      pid_t spawnPid = fork();
      switch (spawnPid) {
          case -1:
              perror("fork()\n");
              exit(1);
              break;
          case 0:
              // child process instructions
              // Accept a connection, blocking if one is not available until one connects
              while(1){

// COPY HERE

                  // Accept the connection request which creates a connection socket
                  connectionSocket = accept(listenSocket,
                      (struct sockaddr *)&clientAddress,
                      &sizeOfClientInfo);

                  if (connectionSocket < 0){
                      error("ERROR on accept");
                  }

                  // Get the message from the client and check it
                  memset(buffer_txt, '\0', sizeof(buffer_txt));
                  charsRead = recv(connectionSocket, buffer_txt, sizeof(buffer_txt) - 1, 0);
                  if (charsRead < 0) error("ERROR reading from socket 4");

                  if (strcmp(dec_handshake_client, buffer_txt) == 0) {
                      // offer decoding service
                      charsRead = send(connectionSocket,
                          dec_handshake_server, sizeof(dec_handshake_server), 0);
                      if (charsRead < 0){
                          error("ERROR writing to socket");
                      }

// LOOP HERE: GET MSG THEN KEY, DECODE MSG, RETURN DECODED MSG

// loop to get, decode, send until data stream complete
newLine = 0;
while (!newLine) {
    // Get the next message datagram from the client
    memset(buffer_txt, '\0', sizeof(buffer_txt));
    charsRead = recv(connectionSocket, buffer_txt, sizeof(buffer_txt) - 1, 0);
    if (charsRead < 0) error("ERROR reading from socket 1");

    // ACK msg
    charsRead = send(connectionSocket,
        dec_handshake_server, strlen(dec_handshake_server), 0);
    if (charsRead < 0){
        error("ERROR writing to socket");
    }

    // Get the next key datagram from the client
    memset(buffer_key, '\0', sizeof(buffer_key));
    charsRead = recv(connectionSocket, buffer_key, sizeof(buffer_key) - 1, 0);
    if (charsRead < 0) error("ERROR reading from socket 2");

    // DECODE alg on chars (strlen(buffer_txt))
    for (size_t l = 0; l < strlen(buffer_txt); l++) {
        next_char = decode_char(buffer_txt[l], buffer_key[l]);
        if (next_char == '\n') newLine = 1;
        buffer_txt[l] = next_char;
    }

    // send compiled, decoded msg
    charsRead = send(connectionSocket,
        buffer_txt, sizeof(buffer_txt) - 1, 0);
    if (charsRead < 0) error("ERROR writing to socket");
}
                  }
                  else {
                      // gtfoh
                      // printf("bruh.\n");
                      charsRead = send(connectionSocket,
                          "git the foo outta he'e", 22, 0);
                      if (charsRead < 0){
                          error("ERROR writing to socket");
                      }
                  }

// TO HERE

                  // Close the connection socket for this client
                  close(connectionSocket);
              }

              exit(1);
              break;
          default:
              // increment numOfChildren
              number_of_children = number_of_children + 1;
              if (number_of_children == 4) {
                  // Accept a connection, blocking if one is not available until one connects
                  while(1){

// COPY HERE

                // Accept the connection request which creates a connection socket
                connectionSocket = accept(listenSocket,
                    (struct sockaddr *)&clientAddress,
                    &sizeOfClientInfo);
                if (connectionSocket < 0) error("ERROR on accept");

                // Get the message from the client and check it
                memset(buffer_txt, '\0', sizeof(buffer_txt));
                charsRead = recv(connectionSocket, buffer_txt, sizeof(buffer_txt) - 1, 0);
                if (charsRead < 0) error("ERROR reading from socket 4");

                if (strcmp(dec_handshake_client, buffer_txt) == 0) {
                    // offer decoding service
                    charsRead = send(connectionSocket,
                        dec_handshake_server, sizeof(dec_handshake_server), 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }

// LOOP HERE: MSG THEN KEY, decODE, RETURN

// loop to get, decode, send until data stream complete
newLine = 0;
while (!newLine) {
  // Get the next message datagram from the client
  memset(buffer_txt, '\0', sizeof(buffer_txt));
  charsRead = recv(connectionSocket, buffer_txt, sizeof(buffer_txt) - 1, 0);
  if (charsRead < 0) error("ERROR reading from socket 1");

  // ACK msg
  charsRead = send(connectionSocket,
      dec_handshake_server, strlen(dec_handshake_server) + 1, 0);
  if (charsRead < 0){
      error("ERROR writing to socket");
  }

  // Get the next key datagram from the client
  memset(buffer_key, '\0', sizeof(buffer_key));
  charsRead = recv(connectionSocket, buffer_key, sizeof(buffer_key) - 1, 0);
  if (charsRead < 0) error("ERROR reading from socket 2");
  
  // alg on chars (strlen(buffer_txt))
  for (size_t l = 0; l < strlen(buffer_txt); l++) {
      next_char = decode_char(buffer_txt[l], buffer_key[l]);
      if (next_char == '\n') newLine = 1;
      buffer_txt[l] = next_char;
  }

  // send compiled, decoded msg
  charsRead = send(connectionSocket,
      buffer_txt, sizeof(buffer_txt) - 1, 0);
  if (charsRead < 0) error("ERROR writing to socket");
}
                }
                else {
                    // gtfoh
                    // printf("bruh.\n");
                    charsRead = send(connectionSocket,
                        "git the foo outta he'e", 22, 0);
                    if (charsRead < 0){
                        error("ERROR writing to socket");
                    }
                }

// TO HERE

                      // Close the connection socket for this client
                      close(connectionSocket);
                  }
              }
              break;
      }
  }

  // Close the listening socket
  close(listenSocket);
  return 0;
}
