#include "util.h"
#include <stdint.h>
#include <ncurses.h>

#define GREEN_PAIR 1
#define RED_PAIR 2
#define BLACK_PAIR 3

int16_t row, col, lok, msg = 0;
int32_t sock_fd;
char client[MAX_CLIENT_NAME_SIZE];

char in_buffer[MAX_MESSAGE_SIZE];

WINDOW* chat_win; 
WINDOW* input_win;
WINDOW* win;

void check_args(int args)
{
	if (args < 3)
	{
	    printf("[E] Some arguments are missing \n");
		exit(-1);
	}
}

void socket_start(const char* address,const char* port,const char* name)
{
	struct sockaddr_in serv_addr;
	// Create client socket
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    printf("[E] Socket creation error \n");
	    exit(-1);
	}
	// Setup server address and port
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port));
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
	    printf("[E] Invalid address or address not supported \n");
	    exit(-1);
	}
	
	// Connect to server
	if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
	    
		move(0, 0);
		endwin();
		//printf("[E] Connection Failed \n");
		printf("[E] Impossivel achar novo Link, tente novamente \n");
		printf("\n");
	    exit(-1);
	}
	
	if (send(sock_fd, name, strlen(name), 0) == -1) {
		move(0, 0);
		endwin();
		printf("[E] Failed sending the client name to the server");
		printf("\n");
	    exit(-1);
	}
}

void wait(int time_delay,WINDOW* input_win,int color)
{
    wclear(input_win);
	float tick = ((float)time_delay/(col - 4)) * 1000.0f;
    win = newwin(3 , col - 2 , (row - 4) + 1 , 1);
	wattron(win,COLOR_PAIR(color));
    wmove(win, 1, 0);
	for (int16_t i = 0 ; i < ( col - 4 ) ; i++ )
	{

    	wmove(win, 1, 1 + i);
		waddch(win, ACS_VLINE);
    	box(win, 0, 0);
		wrefresh(win);
		usleep(tick * 1000.0f);
	}
	wattroff(win,COLOR_PAIR(color));
    wclear(win);
    werase(win);
    wborder(input_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);

	wattron(input_win,COLOR_PAIR(BLACK_PAIR));
    mvwprintw(input_win, 0, 2, "[Zen-Prompt]");	
	wattron(input_win,COLOR_PAIR(GREEN_PAIR));
	wmove(input_win, 1, 2);
	wprintw(input_win, "> "); 
	wrefresh(input_win);
}

void* input_win_thread(void* arg)
{
    initscr();
	cbreak();
	noecho();

    curs_set(0);


	start_color();

	init_pair(BLACK_PAIR, COLOR_WHITE, COLOR_BLACK);
	init_pair(GREEN_PAIR, COLOR_GREEN, COLOR_BLACK);
	init_pair(RED_PAIR, COLOR_RED, COLOR_BLACK);


    getmaxyx(stdscr, row, col);
	if (col - 10 > MAX_MESSAGE_SIZE)
	{
		endwin();
    	printf("[E] Your terminal size is to big \n");
    	exit(-1);
	}
	chat_win  = newwin(row - 3, col - 2, 0, 1);
    input_win = newwin(3 , col - 2 , (row - 4) + 1 , 1);
	wattron(chat_win,COLOR_PAIR(GREEN_PAIR));
	wattron(input_win,COLOR_PAIR(GREEN_PAIR));
	wmove(chat_win, 0, 0); 
    box(chat_win, 0, 0);
	wrefresh(chat_win); 
    keypad(stdscr, TRUE);
    scrollok(input_win, TRUE);

	lok = msg = 0;

	char buffer[col - 10];
	int16_t index = 0;

    wborder(input_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
	wattron(input_win,COLOR_PAIR(BLACK_PAIR));
    mvwprintw(input_win, 0, 2, "[Zen Prompt]");
	wattron(input_win,COLOR_PAIR(GREEN_PAIR));
	wmove(input_win, 1, 2);
	wprintw(input_win, "> ");
	wrefresh(input_win);
	wprintw(chat_win,"\n");
	while(1)
	{
    	box(chat_win, 0, 0);
    	int ch = wgetch(input_win);
		if (ch == '\n') 
    	{
			lok = 1;
			wait(2,input_win,GREEN_PAIR);
			msg++;
			if (msg > row - 6 )
			{
    			wclear(chat_win);
				box(chat_win,0,0);
			    wprintw(chat_win,"\n");
		   		wrefresh(chat_win); 
				msg = 0;
			}
			strncpy(in_buffer, buffer, sizeof(in_buffer) - 1);
			in_buffer[sizeof(in_buffer) - 1] = '\0'; 
			memset(buffer, 0, sizeof(buffer));
    		index = 0;
    	}
    	else if ( ch == KEY_BACKSPACE || ch == 127 ) 
    	{
    		if (index > 0) 
    		{
    			buffer[index--] = ' '; 
    			buffer[index] = ' '; 
    			wmove(input_win, 1, 4 + index); 
    			wprintw(input_win, " "); 
    			wmove(input_win, 1, 4 + index); 
    			wrefresh(input_win); 
    		}
    	}
    	else if ( ch >= 32 && ch <= 126 ) 
    	{
    		if (index < (int16_t)sizeof(buffer)-1) 
    		{
    			buffer[index++] = ch; 
    			wprintw(input_win, "%c", ch); 
    			wrefresh(input_win); 
    		}
    	}
	}

    endwin();
	return NULL;
}

void *send_message_thread(void *arg) 
{
	char buffer[MAX_MESSAGE_SIZE] = {0};
	while (1) {
			while (strlen(in_buffer) == 0) 
			{
				usleep(1000); 
			}
			strncpy(buffer, in_buffer, sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			memset(in_buffer, 0, sizeof(in_buffer)); 
			if (send(sock_fd, buffer, strlen(buffer), 0) == -1) {
			    printf("[E] Send failed \n");
			    break;
			}
			memset(buffer, 0, sizeof(buffer));
	}
	return NULL;
}

int main(int argc,const char* argv[])
{	
	check_args(argc);
	socket_start(argv[1],argv[2],argv[3]);

	pthread_t in_tid, out_tid;
	if (pthread_create(&in_tid, NULL, input_win_thread,(void *) argv[3]) != 0) {
	    	printf("[E] Thread creation error \n");
	    	exit(-1);
	}
	if (pthread_create(&out_tid, NULL, send_message_thread, NULL) != 0) {
	    	printf("[E] Thread creation error \n");
	    	exit(-1);
	}
	while(1)
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(sock_fd, &set);
		struct timeval timeout = {0, 0};
		int ret = select(sock_fd + 1, &set, NULL, NULL, &timeout);
		if (ret > 0)
		{
			char buffer[MAX_MESSAGE_SIZE] = {0};
			int valread = read(sock_fd, buffer, MAX_MESSAGE_SIZE);
			if (valread > 0) 
			{
				if (msg > row - 6 )
				{
    				wclear(chat_win);
					box(chat_win,0,0);
			    	wprintw(chat_win,"\n");
			   		wrefresh(chat_win); 
					msg = 0;
				}
				msg++;
			    wprintw(chat_win," %s\n",buffer);
				box(chat_win,0,0);
			   	wrefresh(chat_win); 
			}
			else 
			{
				close(sock_fd);
    			wborder(input_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
				wattron(input_win,COLOR_PAIR(BLACK_PAIR));
    			mvwprintw(input_win, 0, 2, "[Zen-Prompt]");
				wattron(input_win,COLOR_PAIR(GREEN_PAIR));
				wmove(input_win, 1, 2);
			    wprintw(chat_win," [E] Link principal instavel, tentando criar nova rota...");
				box(chat_win,0,0);
			    wrefresh(chat_win);
				wrefresh(input_win);
				sleep(3);
				wait(5,input_win,GREEN_PAIR);
				socket_start(argv[1],argv[2],argv[3]);
			}
		}
	}
	wattroff(chat_win,COLOR_PAIR(GREEN_PAIR));
	wattroff(input_win,COLOR_PAIR(GREEN_PAIR));
	endwin();
	close(sock_fd);
    pthread_join(in_tid, NULL);
    pthread_join(out_tid, NULL);
    return 0;
}
