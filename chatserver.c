/* ------- Multiple clients connects to server and can commuinicate simultaneously.
 We have use Threads for making it Non-Blocking such that which ever client is ready with its message can send -----
 It takes port no as command line input
 eg: ./a.out 9090 */


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
#define MAX 100 // We take maximum clients N=100

#define BUFFER 1024

const int backLog=10;
static _Atomic unsigned int count=0; // multiple client can use it without raising synchronisation problems

static int id=1; //marks each client uniquely


typedef struct{
	struct sockaddr_in address;
	int id;
	int sockfd;    //a structure to store each client information
	}mclient;
	
	mclient *clients[MAX];
	
	pthread_mutex_t cmutex=PTHREAD_MUTEX_INITIALIZER; // statically initializes mutex object. Here cmutex is shared object
	
	void add(mclient *c){
	pthread_mutex_lock(&cmutex); // lock on shared object cmutex [to handle multiple client access to add function]. if locked already suspends calling thread. add is atomic operation
	
	for(int i=0;i<MAX;i++){
	if(!clients[i]){
	clients[i]=c; //add client into array
	break;
	}
	}
	
	pthread_mutex_unlock(&cmutex);// unlock on shared object cmutex
	}
	
	void delete(int id){
	pthread_mutex_lock(&cmutex);// lock on shared object cmutex [to handle multiple client access to delete function]. delete is atomic operation
	for(int i=0;i<MAX;i++){
	if(clients[i]){
	if(clients[i]->id==id){ // checks if same client is getting removed
	clients[i]=NULL;
	break;
	}
	}
	}
	pthread_mutex_unlock(&cmutex); //performing unlock on cmutex (released)
}


	void send_msg(char* s,int id){//sends message to all clients except sender itself. Takes input as message to send and the id of current client
	int k;
	pthread_mutex_lock(&cmutex); // send_msg is a atomic operation. lock performed on mutex object cmutex
	for(int i=0;i<MAX;i++){
	if(clients[i]){
	if(clients[i]->id!=id){// dont send back the message to itself
	k=write(clients[i]->sockfd,s,strlen(s));
	if(k<0){
	printf("error");
	break;
	}
	}
	}
	}
	pthread_mutex_unlock(&cmutex);// release of mutex object cmutex
	}
	
	
	void *actual(void *arg){ //specifying the behavior of thread created
	char buff[BUFFER];
	int flag=0;//if any error flag=1
	count++;
	
	mclient *cl=(mclient*)arg;
	
	sprintf(buff,"Client:%d entered \n",cl->id); //notes down the client which entered

	send_msg(buff,cl->id);//send message to all clients joined before it
	bzero(buff,BUFFER);
	while(1){
	if(flag){
	break;
	}
	int rec=recv(cl->sockfd,buff,BUFFER,0); //recieve message from client
	if(rec>0){
	if(strlen(buff)>0){//checks if buffer has something
	send_msg(buff,cl->id);//send message to all other clients
	
	
	for(int i=0;i<(strlen(buff));i++){//trimming of characters(when struck with newline make it end of character
	if(buff[i]=='\n'){
	buff[i]='\0';
	break;
	}
	}
	
	
	printf("Client:%d ::: %s\n",cl->id,buff); //print on server which client has send 
	}
	}
	else if(rec== 0 || strcmp(buff,"exit")==0){ // if client sends exit, client exits the server
	sprintf(buff,"Client:%d exit \n ",cl->id); //message showing client has left
	printf("%s",buff);
	send_msg(buff,cl->id); // send message to all other clients that current client left the chatroom
	flag=1;
	}else{
	printf("error");
	flag=1;
	}
	bzero(buff,BUFFER);
	}
	close(cl->sockfd); //disconnect client
	delete(cl->id); //remove the client
	free(cl); // free memory
	count--; // decrease no of clients 
	pthread_detach(pthread_self()); //marks thread as dead
	return NULL;
	}
 

int main(int argc, char **argv){
	if(argc < 2){
		printf("Please specify the port no !!!!!\n ./a.out <port no>"); // handling command line input for port no
		exit(0);
	}

	
	int option = 1,t;
	int serversockfd=0,ret=0;
	struct sockaddr_in server,client;
	socklen_t len=sizeof(client);
	pthread_t tid;
	
	serversockfd=socket(AF_INET,SOCK_STREAM,0);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr("127.0.0.1"); // as mainly a group chat program hence we take a default IP address
	server.sin_port=htons(atoi(argv[1]));//  port no given as command line argument given by user 
	
	signal(SIGPIPE,SIG_IGN); //pipe signals (software generated interrupts)
	
	t=setsockopt(serversockfd,SOL_SOCKET,(SO_REUSEPORT|SO_REUSEADDR),(char*)&option,sizeof(option));// to remove binding issues while connecting to multiple clients
	if(t<0){ 
	printf("Error");
	exit(0);
	}
	t=bind(serversockfd,(struct sockaddr*)&server,sizeof(server));
	if(t<0){
	printf("Error");
	exit(0);
	}
	t=listen(serversockfd,backLog);
	if(t<0){
	printf("error");
	exit(0);
	}
	printf("Server Listening Here!\n");
	
	
	
	while(1){ //inf loop to accept connection from multiple clients till it reaches N=100
	
	ret=accept(serversockfd,(struct sockaddr*)&client,&len);
	
	if((count+1)==MAX){
	printf("Maximum clients connected");// check for max client
	close(ret);
	continue;
	}
	
	
	mclient *cl=(mclient *)malloc(sizeof(mclient)); //creating client 
	cl->address=client;
	cl->sockfd=ret; //connection socket descriptor accepted from client
	cl->id=id++; //increment count of client
	
	add(cl);// add client to queue 
	pthread_create(&tid,NULL,&actual,(void*)cl); // thread created for new client
	
	sleep(1); //to give a pause
	}
	
	
	

	return 0;
}	
