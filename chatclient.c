/* ------- Multiple clients connects to server and can commuinicate simultaneously.
 We have use Threads for making it Non-Blocking such that which ever client is ready with its message can send -----
 It takes port no as command line input
 eg: ./a.out 9090 */


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

#define LEN 1024 
char name[50];
volatile sig_atomic_t flag = 0; // for atomic access in asynchronous environment
int clientfd = 0;



	void send_msg() { //imparts sending message characteristics to thread
	char msg[LEN] = {};
	char buff[LEN + 60] = {};

	while(1) {
	fgets(msg, LEN, stdin);
	int i;
	for (i = 0; i < LEN; i++) { ////trimming of characters(when struck with newline make it end of character
	if (msg[i] == '\n') {
	msg[i] = '\0';
	break;
	}
	}
	if (strcmp(msg, "exit") == 0) { //if client sends exit then set flag=1 and later close connection
	break;
	} else {
	sprintf(buff, "%s: %s\n", name, msg);
	send(clientfd, buff, strlen(buff), 0); //send message to other clients along with clients name
	}

	bzero(msg, LEN);
	bzero(buff, LEN + 60);
	}
	flag=1;
	}

	void recv_msg() { //imparts recieving message characteristics to the thread
	char msg[LEN] = {};
  	while (1) {
	int receive = recv(clientfd, msg, LEN, 0); //recieving from other clients
    	if (receive > 0) {
      	printf("%s", msg);
    	} else if (receive == 0) {
	break;
    	} 
	memset(msg, 0, sizeof(msg)); //fill msg with 0
  	}
	}
	void terminate(int sig) {
    	flag = 1;
	}


	int main(int argc, char **argv){
	if(argc< 2){
	printf("Please  provide port no!!! -- ./a.out <port>");
	exit(0);
	}
	signal(SIGINT, terminate);
	printf("Please enter your name(string): "); // client's name
	fgets(name, 50, stdin); 
	int len=strlen(name);
	int i;
	for (i = 0; i < len; i++) { //trimming of characters(when struck with newline make it end of character
	if (name[i] == '\n') {
	name[i] = '\0';
	break;
    	}
  	}
  
  
  
	struct sockaddr_in server; 
	clientfd = socket(AF_INET, SOCK_STREAM, 0);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr("127.0.0.1"); // as mainly a group chat program hence we take a default IP address
  server.sin_port = htons(atoi(argv[1]));// port number given in command line 


  int ret = connect(clientfd, (struct sockaddr *)&server, sizeof(server)); // connect client to server 
  if (ret<0) {
		printf("error");
		exit(0);;
	}

	send(clientfd, name, 50, 0); //send name of client 

	printf("You are Connected\n");

	pthread_t send_thread; // declaring thread to send message
	ret=pthread_create(&send_thread, NULL, (void *) send_msg, NULL); // creating thread + giving the function of send_msg to send messages to other clients
  	if( ret!= 0){
	printf("error");
    	exit(0);
	}

	pthread_t recv_thread; //declaring thread to recieve message
	ret=pthread_create(&recv_thread, NULL, (void *) recv_msg, NULL); //creating thread + recieving messages from other clients through recv_msg
  	if( ret!= 0){
	printf("error");
	exit(0);
	}

	while (1){
	if(flag){
	printf("\nBye.All connections are closed\n"); //client continues to send and recieve messages until we give exit or flag =1
	break;
	}
	}

	close(clientfd); //close client socket descriptor

	return 0;
}
