#include "util.h"
#include <ncurses.h>

char input_buffer[256] = {0};
char client[MAX_CLIENT_NAME_SIZE];
int sock;
WINDOW *input_win; 
WINDOW *chat_win;
	//TODO:
	// -- add function for checking user parameters
	// -- add support for special characters
	// -- use stdint header file
	// -- add message and input text corrections in case of overflowing
	// the availave space
	// -- add btop like windows

void handle_new_message(int sock) {
	char buffer[256] = {0};
	int valread = read(sock, buffer, 256);
	
	if (valread > 0) 
	{
	    	wprintw(chat_win,"%s\n",buffer);
	   	wrefresh(chat_win); 
	}
}

void *input_thread(void *arg) {
	int index = 0; 
    	char buffer[256];
	while(1)
	{
 		int ch = wgetch(input_win); 
        	if (ch == '\n') 
        	{
        	    	wprintw(chat_win,"%s %s\n",client,buffer);
        	    	wrefresh(chat_win);
		    	strncpy(input_buffer, buffer, sizeof(input_buffer) - 1);
		    	input_buffer[sizeof(input_buffer) - 1] = '\0'; 
		    	memset(buffer, 0, sizeof(buffer)); 
        	    	index = 0;
        	    	wclear(input_win); 
        	    	wprintw(input_win, "%s",client); 
        	    	wrefresh(input_win); 
        	}
        	else if (ch == KEY_BACKSPACE || ch == 127) 
        	{
        		if (index > 0) 
        		{
        			index--; 
        			wmove(input_win, 0, strlen(client)+index); 
        			wprintw(input_win, " "); 
        			wmove(input_win, 0, strlen(client)+index); 
        			wrefresh(input_win); 
        		}
        	}
        	else if (ch >= 32 && ch <= 126) 
        	{
        		if (index < (int)sizeof(buffer)-1) 
        		{
        			buffer[index++] = ch; 
        			wprintw(input_win, "%c", ch); 
        			wrefresh(input_win); 
        		}
        	}
	}
    return NULL;
}

void *send_thread(void *arg) {
	char buffer[256] = {0};
	while (1) {
		while (strlen(input_buffer) == 0) 
		{
			usleep(1000); 
		}
		// Copy input buffer to send buffer
		strncpy(buffer, input_buffer, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
		memset(input_buffer, 0, sizeof(input_buffer)); 
		// Send data to server
		if (send(sock, buffer, strlen(buffer), 0) == -1) {
		    printf("\nSend failed \n");
		    break;
		}
		memset(buffer, 0, sizeof(buffer));
	}
	return NULL;
}

int main(int argc, const char* argv[]) 
{
	const char* address = argv[1];
	const char* port = argv[2];
	sprintf(client,"[%s]",argv[3]);
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE); 
	
	start_color(); // enable color support
	init_pair(1, COLOR_RED, COLOR_BLACK); 
	init_pair(2, COLOR_GREEN, COLOR_BLACK); 
	init_pair(3, COLOR_BLUE, COLOR_BLACK); 
	init_pair(4, COLOR_MAGENTA, COLOR_BLACK); 
	init_pair(5, COLOR_YELLOW, COLOR_BLACK); 
	
	int row, col;
	getmaxyx(stdscr, row, col); 
	input_win = newwin(1, col, row-1, 0);
	chat_win = newwin(row-1, col, 0, 0);
	scrollok(input_win, TRUE); 
	wmove(input_win, 0, 0); 
	wmove(chat_win, 0, 0); 
	wprintw(input_win, "%s",client); 
	wrefresh(input_win); 
	
	struct sockaddr_in serv_addr;
	pthread_t input_tid, send_tid;
	
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
	
	if (send(sock, argv[3], strlen(argv[3]), 0) == -1) {
		printf("[E] Failed sending the client name to the server");
	}
	
	// Send messages to server
	while (1) 
	{
	    	fd_set set;
	    	FD_ZERO(&set);
	    	FD_SET(sock, &set);
	    	struct timeval timeout = {0, 0};
	    	// Wait for new message from server
	    	int ret = select(sock + 1, &set, NULL, NULL, &timeout);
	    	if (ret > 0) {
	        	handle_new_message(sock);
	    	}
	}
	// Close socket
	close(sock);
	pthread_join(input_tid, NULL);
	pthread_join(send_tid, NULL);
	return 0;
	
}
