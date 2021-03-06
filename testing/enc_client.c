// enc_client.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead, charsChecked, charsReadTotal, badNewLine;
  struct sockaddr_in serverAddress;
  char buffer_txt[256];
  char checker[256];
  char buffer_key[256];

  // set handshake messages
  char enc_handshake_client[36] = "I am requesting encoding service.\n\0";
  char enc_handshake_server[25] = "Proceed with encoding.\n\0";

  // Check usage & args
  if (argc < 4) {
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]);
    exit(0);
  }

  // open file descriptors for plaintext and key
  int plaintext_fd = open(argv[1], O_RDONLY);
  int key_fd = open(argv[2], O_RDONLY);

  // check file lengths
  struct stat stat1, stat2;
  fstat(plaintext_fd, &stat1);
  fstat(key_fd, &stat2);
  // printf("%lld\n", stat1.st_size);
  // printf("%lld\n", stat2.st_size);
  if (stat1.st_size > stat2.st_size) {
      fprintf(stderr, "CLIENT: key \'%s\' is too short\n", argv[2]);
      exit(1);
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, enc_handshake_client, strlen(enc_handshake_client), 0);
  if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
  if (charsWritten < strlen(enc_handshake_client)){
    fprintf(stderr, "CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Get return message from server
      // Clear out the buffer_txt again for reuse
      memset(buffer_txt, '\0', sizeof(buffer_txt));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer_txt, sizeof(buffer_txt) - 1, 0);
  if (charsRead < 0) error("CLIENT: ERROR reading from socket");

  if (strcmp(enc_handshake_server, buffer_txt) == 0) {
      // do encoding

      // read key
      memset(buffer_key, '\0', sizeof(buffer_key));
      charsRead = read(key_fd, buffer_key, sizeof(buffer_key) - 2);
      if (charsRead < 0) error("CLIENT: ERROR reading from key");

      // read plaintext
      memset(buffer_txt, '\0', sizeof(buffer_txt));
      charsRead = read(plaintext_fd, buffer_txt, sizeof(buffer_txt) - 2);
      charsReadTotal = charsRead;

      // loop check, send, recieve, print, read until read returns 0
      while (charsRead > 0) {
          // check for input validity
          memset(checker, '\0', sizeof(checker));
          charsChecked = pread(plaintext_fd, checker, sizeof(checker) - 1, charsReadTotal);
          if (checker[0] == '\0') badNewLine = 0;
          else badNewLine = 1;
          int next_char;
          for (size_t k = 0; k < strlen(buffer_txt); k++) {
              next_char = buffer_txt[k];
              if (next_char == 32) continue;
              if (next_char > 64 && next_char < 91) continue;
              if (next_char == 10) {
                  if (!badNewLine && k == strlen(buffer_txt) - 1) continue;
              }
              fprintf(stderr, "enc_client error: input contains bad characters" );
          }

          // send MSG
          // if (strlen(buffer_txt) == 256)
          charsWritten = send(socketFD, buffer_txt, strlen(buffer_txt), 0);
          if (charsWritten < 0) error("CLIENT: ERROR writing to socket");

          // check reciept of MSG, thereby waiting and not overloading socket
          memset(buffer_txt, '\0', sizeof(buffer_txt));
          charsRead = recv(socketFD, buffer_txt, sizeof(buffer_txt) - 1, 0);
          if (charsRead < 0) error("CLIENT: ERROR reading from socket");
          if (strcmp(enc_handshake_server, buffer_txt) != 0) error("CLIENT: ERROR not given proceed directive");

          // send KEY
          charsWritten = send(socketFD, buffer_key, strlen(buffer_key), 0);
          if (charsWritten < 0) error("CLIENT: ERROR writing to socket");

          // get encoded msg
          charsRead = recv(socketFD, buffer_txt, sizeof(buffer_txt) - 1, 0);
          if (charsRead < 0) error("CLIENT: ERROR reading from socket");

          // print encoded msg
          fprintf(stdout, "%s", buffer_txt);

          // read next key data
          memset(buffer_key, '\0', sizeof(buffer_key));
          charsRead = read(key_fd, buffer_key, sizeof(buffer_key) - 1);
          if (charsRead < 0) error("CLIENT: ERROR reading from key");
          // read next plaintext data
          memset(buffer_txt, '\0', sizeof(buffer_txt));
          charsRead = read(plaintext_fd, buffer_txt, sizeof(buffer_txt) - 1);
          charsReadTotal = charsReadTotal + charsRead;
      }
      if (charsRead < 0) error("CLIENT: ERROR reading from plaintext");
  }
  else {
      error("could not contact enc_server on port %s", argv[3]);
  }

  // Close the socket
  close(socketFD);
  return 0;
}