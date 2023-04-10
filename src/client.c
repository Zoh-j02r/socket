#include "util.h"
#include <time.h>
#include <unistd.h>

char input_buffer[256] = {0};
int input_processed = 1;
char client[MAX_CLIENT_NAME_SIZE];
int sock;

int locked = 0;
	//TODO:
	// -- add function for checking user parameters
	// -- add support for special characters
	// -- use stdint header file

void handle_new_message(int sock)
{
	char buffer[256] = {0};
	int valread = read(sock, buffer, 256);
	
	if (valread > 0) 
	{
    	printf("\033[31mMensagem recebida:\033[0m\n");
	    printf("%s\n",buffer);
	}
}

void handle_wait_window(int time_delay)
{
	while(time_delay > 0)
	{
        sleep(1);

        time_delay--;
	}
	locked = 0;
    printf("\033[31mMensagem enviada com sucesso!\033[0m\n");
}
int clear_line(char *c,int i)
{
	if (*c == '\n')
	{
		*c = ' ';
    	printf("\r");  
        for (int j = 0; j < i+1; j++)
		{
            printf(" ");
        }
    	printf("\r"); 
		return 0;
	}
	return 1;
}

void *input_thread(void *arg)
{
 char buffer[256];
    while (1)
	{
        int i = 0;
        char c = getchar();
        while (clear_line(&c,i) && i < 256-1) 
		{
			if (!locked)
			{
			if (c == 127)
			{  
                if (i > 0) 
				{
                    i--;
                    printf("\b \b"); 
                }
            } else {
                buffer[i] = c;
                i++;
                putchar(c);
				fflush(stdout);
            }
            c = getchar();
        	}
		}
		locked = 1;
 		fflush(stdout);
        input_buffer[i] = '\0';
		strncpy(input_buffer, buffer, sizeof(input_buffer) - 1);
		memset(buffer, 0, sizeof(buffer));
    }
    return NULL;
}

void *send_thread(void *arg)
{
	char buffer[256] = {0};
	while (1)
	{
		if (locked)
		{
			handle_wait_window(10);
			usleep(100000);
		}
		while (strlen(input_buffer) == 0) 
		{
			usleep(1000); 
		}
		strncpy(buffer, input_buffer, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
		memset(input_buffer, 0, sizeof(input_buffer)); 
		if (send(sock, buffer, strlen(buffer), 0) == -1)
		{
		    printf("Send failed \n");
		    break;
		}
		memset(buffer, 0, sizeof(buffer));
	}
	return NULL;
}

void *messaging(void *arg)
{
	while (1) 
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sock, &set);
		struct timeval timeout = {0, 0};
		int ret = select(sock + 1, &set, NULL, NULL, &timeout);
		if (ret > 0) 
		{
	    	handle_new_message(sock);
		}
	}
	return NULL;
}

int main(int argc, const char* argv[]) 
{
	const char* address = argv[1];
	const char* port = argv[2];
	sprintf(client,"[%s]",argv[3]);
	
	struct sockaddr_in serv_addr;
	pthread_t input_tid, send_tid, message_tid;
	
	// Create client socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    	printf("[E] Socket creation error \n");
	    	return -1;
	}
	// Setup server address and port
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port));
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
	    	printf("[E] Invalid address or address not supported \n");
	    	return -1;
	}
	// Connect to server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
	    	printf("[E] Connection Failed \n");
	    	return -1;
	}
	if (pthread_create(&message_tid, NULL, messaging, NULL) != 0) {
		printf("[E] Thread creation error \n");
		return -1;
	}
	if (send(sock, argv[3], strlen(argv[3]), 0) == -1) {
		printf("[E] Failed sending the client name to the server");
	}
	// Start getting user input
	if (pthread_create(&input_tid, NULL, input_thread, NULL) != 0) {
	    	printf("[E] Thread creation error \n");
	    	return -1;
	}
	// Start send thread
	if (pthread_create(&send_tid, NULL, send_thread, NULL) != 0) {
		printf("[E] Thread creation error \n");
		return -1;
	}
	// Send messages to server
	while (1) 
	{
	}
	// Close socket
	close(sock);
	pthread_join(input_tid, NULL);
	pthread_join(send_tid, NULL);
	pthread_join(message_tid, NULL);
	return 0;
	
}
