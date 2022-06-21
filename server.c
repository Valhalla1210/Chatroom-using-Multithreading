#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 100 
#define BUFFER_SZ 2048
#define BACKLOG 10

static unsigned int cli_count = 0;
static int uid = 10;

/* Client structure 
	Struct to store data(sockid, name, address) accessed by other clients when a messaged is broadcasted
	and when a thread terminates
*/
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int error_handler(char* s,int val)
{
	if(val<0)
	printf("%s", s);
	return EXIT_FAILURE;
}
void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}
/*
	Method to NULL terminate a string that is new line terminated
*/
void null_string_term (char* arr, int length) {
  for (int i = 0; i < length; i++) { 
    if (arr[i] == '\n') {
      arr[i] = '\0';
      return;
    }
  }
}
/* Add clients to queue using mutex locks*/
void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients from queue */
void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients except sender */
void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Handles all the communication with the client */
void *handle_client(void *arg){
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;

	cli_count++;
	client_t *cli = (client_t *)arg;

	// Name
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else{
		strcpy(cli->name, name);//name copied to client's name in struct
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		send_message(buff_out, cli->uid);//joining message broadcasted to all other clients
	}

	bzero(buff_out, BUFFER_SZ);

	while(1){
		if (leave_flag) {
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);//while the client has not exited, keeps receiving messages
		if (receive > 0){										//from the client to broadcast them using send_message method
			if(strlen(buff_out) > 0){
				send_message(buff_out, cli->uid);

				null_string_term(buff_out, strlen(buff_out));
				printf("%s -> %s\n", buff_out, cli->name);
			}
		} else if (receive == 0 || strcmp(buff_out, "exit") == 0){
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SZ);
	}

  /* Delete client from queue and yield thread 
     Now a new thread can be created by a pending connection
  */
	close(cli->sockfd);
  queue_remove(cli->uid);
  free(cli);
  cli_count--;
  pthread_detach(pthread_self());//thread is destro

	return NULL;
}

int main(int argc, char **argv){//port no is passed in terminal
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;

  /* Socket settings */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);// socket creation on server side
  serv_addr.sin_family = AF_INET;			// socket is binded wiht IPv4 address and port address
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);
	/* Bind */
	error_handler("ERROR: Socket binding failed",bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) );
  /* Listen */
  error_handler("ERROR: Socket listening failed",listen(listenfd, BACKLOG));
  //server is set into passive mode to listen to client requests
    

	printf("=== WELCOME TO THE CHATROOM ===\n");

	while(1){
		unsigned int clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		error_handler("ERROR: Socket acceptance failed",connfd);

		/* Check if max clients is reached, if yes, then rejects the thread creation*/
		if((cli_count) == MAX_CLIENTS){
			printf("Max clients reached. Rejected: ");
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		/* Add client to the queue and fork thread */
		queue_add(cli);
		pthread_create(&tid, NULL, handle_client, (void*)cli);

		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}
