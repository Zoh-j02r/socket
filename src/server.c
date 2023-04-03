#include "util.h"


struct sockaddr_in address;
int addrlen = sizeof(address);
int server_fd, client1_fd, client2_fd, valread;

	//TODO:
	// - remove the empty message when a client reconnect
	// - make only 1 thread function instead of declaring first and second
	// - add function for checking parameters in initialization
	// - make server decide which color each client should use
	// - put the socket_connect inside the main loop for flexibility
	// - make sever capable of handling N clients

void socket_setup(int port)
{
    // Create server socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	    perror("[E] Socket creation failed");
	    exit(EXIT_FAILURE);
	}
	
	// Setup server address and port
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	
	// Bind the socket to the given address and port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
	    perror("[E] Bind failed");
	    exit(EXIT_FAILURE);
	}
	
	// Listen for incoming connections
	if (listen(server_fd, 3) < 0) {
	    perror("[E] Listen failed");
	    exit(EXIT_FAILURE);
	}
	
	printf("[I] Server listening on port %d\n", port);

} 

void socket_connect(int *client, char *base)
{
	if ((*client = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
	{
		perror("[E] Could not accept client");
		exit(EXIT_FAILURE);
	} 
	else
	{
		memset(base, 0, MAX_CLIENT_NAME_SIZE * sizeof(char));
		read(*client, base, 12);
        	printf("[I] New client connected: [%s] %s:%d\n",base, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	} 
}

void *first_thread(void *arg) {
	char base[MAX_CLIENT_NAME_SIZE] = {0};
    	char buffer[256] = {0};
    	char aux_buffer[280] = {0};
	int valread;
	socket_connect(&client1_fd, base);
	while(1)
	{
        	valread = read(client1_fd, buffer, 256);
        	if (valread == 0) 
		{
        		printf("[I] Lost connection with %s\n",base);
			// If a client disconnects wait for a new client connection to take its place
			socket_connect(&client1_fd, base);
        	}
		// Add client name in brackets
		sprintf(aux_buffer,"[%s] %s",base,buffer);
		printf("[M] %s\n",aux_buffer);
        	send(client2_fd, aux_buffer, strlen(aux_buffer), 0);
		// Clear buffer and aux_buffer
		memset(buffer, 0, sizeof(buffer));
		memset(aux_buffer, 0, sizeof(aux_buffer));
	}
	return NULL;
}

void *second_thread(void *arg) {
	char base[MAX_CLIENT_NAME_SIZE] = {0};
    	char buffer[256] = {0};
    	char aux_buffer[280] = {0};
	int valread;
	socket_connect(&client2_fd, base);
	while(1){
        	valread = read(client2_fd, buffer, 256);
        	if (valread == 0) {
        		printf("[I] Lost connection with %s\n",base);
			socket_connect(&client2_fd, base);
        	}
		sprintf(aux_buffer,"[%s] %s",base,buffer);
		printf("[M] %s\n",aux_buffer);
        	send(client1_fd, aux_buffer, strlen(aux_buffer), 0);
		// Clear buffer and aux_buffer
		memset(buffer, 0, sizeof(buffer));
		memset(aux_buffer, 0, sizeof(aux_buffer));
    	}
	return NULL;
}

int main(int argc, char const *argv[]) {
    pthread_t client1_tid, client2_tid;
    socket_setup(atoi(argv[1]));

    if (pthread_create(&client1_tid, NULL, first_thread, NULL) != 0) {
        printf("[E] Thread creation error \n");
        return -1;
    }

    if (pthread_create(&client2_tid, NULL, second_thread, NULL) != 0) {
        printf("[E] Thread creation error \n");
        return -1;
    }

    while (1) {
    }

    close(client1_fd);
    close(client2_fd);
    close(server_fd);
    pthread_join(client1_tid, NULL);
    pthread_join(client2_tid, NULL);

    return 0;
}
