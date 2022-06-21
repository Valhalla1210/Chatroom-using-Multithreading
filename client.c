#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_LENGTH 2048

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout() {//method to overwrite
  printf("%s", "> ");
  fflush(stdout);
}
/*
	Method used to NULL terminate a string that is new line terminated
*/
void null_string_term(char* a,int n)
{
	for(int i=0;i<n;i++)
	{
		if(a[i]=='\n')
		{
			a[i]='\0';
			return;
		}
	}
}


void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
/* Method invoked by thread that receives, null terminates the strings
   message to be sent is copied from terminal then sent to server which further
   broadcasts it to all other clients.
   Terminates only when client exits the chatroom.
*/
void send_msg_handler() {
  char message[MAX_LENGTH] = {};
	char buffer[MAX_LENGTH + 32] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(message, MAX_LENGTH, stdin);
    null_string_term(message, MAX_LENGTH);

    if (strcmp(message, "exit") == 0) {
			break;
    } else {
      sprintf(buffer, "%s: %s\n", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

		bzero(message, MAX_LENGTH);
    bzero(buffer, MAX_LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}
/* 
	Method invoked by another thread that is responsible for receiving the message broadcasted
	by the server
	Terminates when no message can be recieved by the client
*/
void recv_msg_handler() {
	char message[MAX_LENGTH] = {};
  while (1) {
		int receive = recv(sockfd, message, MAX_LENGTH, 0);
    if (receive > 0) {
      printf("%s", message);
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
}

int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Please enter your name: ");
  fgets(name, 32, stdin);
  null_string_term(name, strlen(name));


	if (strlen(name) > 32 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);


  // Connect to Server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, 32, 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");

	/*
		Thread created to keep sending the messages parallel to receiving them
		Terminates only when the client exits the chat.
	*/
	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}
	/*
		Thread created to keep receiving the messages parralle to sending them
		Terminates when no message can be received by the client.
	*/
	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
