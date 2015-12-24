#ifndef __SERVER
#define __SERVER

#include "header.h"
#include "user_list.h"
#include "chat_room_list.h"
#include "user_info/file_manipulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>								 
#include <ctype.h>
//used to determine if the help menu was printed
#define PRINTED_USAGE_FLAG 100
#define MAX_CLIENTS 10
#define MAX_INPUT_SIZE 1024
#define SHOULD_KILL 1
#define DEFAULT_MAX_NUM_ROOMS 5

/* If user executes with -v flag, we can see the verb transactions */
/* If user executes with -vv flag, this is set to true so that debugging output is provided. */
int verbose;

/* Maximum number of chat rooms allowed. */
int max_num_rooms;
int total_num_rooms;
/* Mutex to make sure that the total number of rooms is correct
and to make sure that two rooms with the same name do not accidentally get created. */
sem_t mutex;  

//used for Rio_Readline
rio_t rio;

/* List of all chat rooms */
struct ChatRoomList *chatroom_list;
int next_chat_room_id; // incremented number to use for next chat room

/* This is set to true if the server program is started with -e flag. */
/* If the server is started with the -e flag then all chat that is sent 
to the client(s) and all messages should also be displayed on the server. */
int verbose_echo;

/* Used to store server port and the "message of the day" */
ServerInfo *serverInfo; // Global instance

/* Listening file descriptor */
int listenfd;

/* ECHO VARIABLES */
//the echo flag used to see if echo is set up
//0 indicates that the echo thread is not up, 1 means it is up
int echo_flag = 0;
//the id of the echo thread - keep it global, it must be shared by all
pthread_t echo_thread_id;

/* WAITING ROOM VARIABLES*/
int waiting_room_flag = 0;		//0 - not up, 1 - up
pthread_t waiting_room_thread_id;

/*Storage structs for server data*/

struct UserList *all_users_list;			// list of ALL users currently on server
struct UserList *waiting_room_users_list; 	// list of all users in waiting room

/*
 *stuff for connecting clients
 */
 //to keep track of num fd for ECHO thread
fd_set echo_thread_active_set, echo_thread_ready_set;							
int echo_fd_count;
//to keep track of num fd for WATING_ROOM thread
fd_set waiting_room_thread_active_set, waiting_room_thread_ready_set;	
int waiting_room_fd_count;

struct timeval timeout;					//to keep track of time

//buffer for input from clients
char input_buffer[MAX_INPUT_SIZE];

/******************************* Functions *****************************/
/**
 * Prints what is being echoed to the clients onto the server host's
 * stdout.
 */
void print_verbose_echo(char *username, char *msg);

/**
 * Parse argv and create a ConnectionInfo object.
 * 
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
ServerInfo *parse_parameters(int argc, char **argv);

/**
 * Initialize global variables and other important things.
 */
void init_globals();

/**
 * Handler to catch Ctrl+C
 * Sends BYE to all Clients
 */
void interrupt_handler(int signal);

/**
 * Receive a verb from a fd and respond accordingly.
 * @param  connfd [description]
 * @param  msg    [description]
 * @return        [description]
 */
int receive_verb(int connfd, char *msg);

/**
 * Check if this verb is allowed in waiting room.
 * Allowed verbs:
 * 		CREATER
 * 		CREATEP
 * 		LISTR
 * 		JOIN
 * 		JOINP
 * 		BYE
 */
bool is_allowed_in_waiting_room(char *verb_name);

/**
 * Echo the message of the day to connfd.
 * @param connfd [description]
 */
void send_message_of_the_day(int connfd);

/**
 * Given a user's fd, remove them from any chat room if they are in one.
 * This is important for the verbs
 * 		BYE
 * 		CREATER
 * 		CREATEP
 * 		JOIN
 * 		JOINP
 * 		KICK
 * 		LEAVE
 * kicked is true if user has been kicked. There will be a different 
 * message sent to all users remaining in chat room.
 * @param connfd [description]
 */
void remove_user_from_chatrooms(int connfd, bool kicked);

/**
 * Creates a new room and spawns a new thread for it.
 * @param room_name [description]
 * @param ownerfd   [description]
 */
void create_new_room(char *room_name, int ownerfd);

/**
 * Creates a new room and spawns a new thread for it.
 * @param room_name [description]
 * @param ownerfd   [description]
 */
void create_new_private_room(char *room_name, char *room_password, int ownerfd);

/**
 * Join an existing room given an id.
 * @param id     [description]
 * @param connfd [description]
 */
void join_room(int id, int fd);

/**
 * Given a room's id and potential password, join it
 * Returns -1 on error, else success
 */
int join_private_room(int room_id, int fd, char *pw);


/**
 * Given a user's fd, remove them from the waiting room and take their fd
 * out of the waiting room fd_set.
 * @param connfd [description]
 */
void remove_from_waiting_room(int connfd);

/**
 * Echo the MOTD and add the user to waiting room
 * @param connfd - the connection decription of the user
 * @param usrename - the user name of the user
 */
void complete_login(int connfd, char *username);

/**
 * Given a password, make sure it is,
 *     5 characters long
 *     Has at least 1 uppercase letter
 *     Has at least 1 symbol character
 *     Has at least 1 number character
 * @return - returms 1 if pw is valid, 0 if otherwise
 */
int validatePassword(char *password);

/**
 * Return true if room is private or room does not exist. Else, return false.
 * 
 * @param  room_id [description]
 * @return         [description]
 */
bool is_room_private(int room_id);


//used in hashing
char *make_salt(char *pw);
char *hash(char *pw, char *salt);
int check_if_match(char *pw, char *salt, char *storedHashedPW);

/******************************* Threads *******************************/

void *login_thread(void *vargp);

void *waiting_room_thread(void *vargp);

void *chatroom_thread(void *vargp);

void *echo_thread(void *vargp);

/****************************** Verbs *********************************/


void send_AHOLA_verb(int fd);
void send_HI_verb(int fd, char *user);
void send_BYE_verb(int fd);
void send_ECHO_verb(int fd, char *username, char *msg);
void send_RETAERC_verb(int fd, char *room_name);
void send_RTSIL_verb(int fd, struct ChatRoomList *chatroom_list);
void send_UTSIL_verb(int fd, struct UserList *user_list);
void send_NIOJ_verb(int fd, char *id);
void send_EVAEL_verb(int fd);
void send_KBYE_verb(int fd);
void send_LLET_verb(int fd, char *user_to_msg, char *msg);
void send_ECHOP_verb(int fd, char *from_user, char *msg);
void send_HINEW_verb(int fd, char *name);
void send_AUTH_verb(int fd, char *name);
void send_PETAERC_verb(int fd, char *room_name);
void send_PNIOJ_verb(int fd, char *id);
void send_KCIK_verb(int fd, char *username);

										// optional msg
void send_ERR_verb(int fd, int errorno, char *msg);
#endif