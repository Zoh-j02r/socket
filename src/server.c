#include "util.h"

struct sockaddr_in address;
int addrlen = sizeof(address);
int16_t server_fd;
int max_number_clients;


typedef struct
{
	pthread_t messaging;
	char name[MAX_CLIENT_NAME_SIZE];
	char buffer[MAX_MESSAGE_SIZE];
	int sock;
} 
Client;

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

void mission_begin()
{

}

void *message_handler(void *arg)
{
	Client* client = (Client*) arg;
	char aux_buffer[MAX_MESSAGE_SIZE + MAX_CLIENT_NAME_SIZE + 3] = {0};
	while(1)
	{
		for (int i = 0; i < max_number_clients - 1; i++)
		{
			if (client[i].sock != 0)
			{
				if (strlen(client[i].buffer) != 0) 
				{
					for (int j = 0; j < max_number_clients - 1; j++)
					{
						sprintf(aux_buffer,"%s: %s",client[i].name,client[i].buffer);
						send(client[j].sock, aux_buffer, strlen(aux_buffer), 0);
					}
					printf("[M] %s\n",aux_buffer);
					memset(client[i].buffer, 0, sizeof(client[i].buffer));
					memset(aux_buffer, 0, sizeof(aux_buffer));
				}
			}
		}
	
	}
	return NULL;
}

void *client_thread(void *arg) {
	Client* client = (Client*) arg;
	int16_t valread;
	while(1)
	{
        valread = read(client->sock, client->buffer, MAX_MESSAGE_SIZE);
        if (valread == 0) 
		{
        	printf("[I] Lost connection with %s\n", client->name);
			break;
        }
	}

    close(client->sock);
	client->sock = 0;
	return NULL;
}

int main(int argc, char const *argv[])
{
	pthread_t messaging;
    socket_setup(atoi(argv[1]));
	max_number_clients = atoi(argv[2]);
	Client client[max_number_clients - 1];
	for (int i = 0;  i < max_number_clients - 1; i++)
	{ 
		client[i].sock = 0;
		memset(client[i].buffer, 0, sizeof(client[i].buffer));
	}
	if (pthread_create(&messaging, NULL, message_handler,&client) != 0) {
	    printf("[E] Thread creation error \n");
	    return -1;
	}
	
	int avail = 0;
    while (1)
	{
			for (int i = 0;  i < max_number_clients - 1; i++)
			{
				if(client[i].sock == 0)
				{
					avail = i;
				}
   			}
			
			if ((client[avail].sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
			{
				perror("[E] Could not accept client");
				exit(EXIT_FAILURE);
			}
			else
			{
				char client_name[12];
				read(client[avail].sock, client_name, 12);
    		    printf("[I] New client connected: %s - %s:%d\n", client_name, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
				strncpy(client[avail].name, client_name, sizeof(client[avail].name) - 1);
				client[avail].name[sizeof(client[avail].name) - 1] = '\0'; 
    			
				if (pthread_create(&client[avail].messaging, NULL, client_thread,(void*) &client[avail]) != 0) {
    			    printf("[E] Thread creation error \n");
    			    return -1;
    			}

			} 
	}

    close(server_fd);
	for (uint16_t i = 0;  i < max_number_clients - 1; i++) {
    	pthread_join(client[i].messaging, NULL);
 	}
    return 0;
}
