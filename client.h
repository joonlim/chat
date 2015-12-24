#ifndef __CLIENT
#define __CLIENT

#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_MESSAGE_SIZE 1024

/* If user executes with -v flag, this is set to true so that debugging output is provided. */
bool verbose;

/* Used to store username, server ip, and server port */
ConnectionInfo *connectionInfo; // Global instance

/* Client file descriptor */
int clientfd;
rio_t rio;

/* Used to store user input */
char input_buffer[MAX_LINE_SIZE];
size_t input_len = MAX_LINE_SIZE;
ssize_t read_amt;

/******************************* Functions *****************************/

/**
 * Do  the "aloha" connection protocol
 * Send "ALOHA!"
 * Receive  "!AHOLA"
 * Send IAM <NAME>
 * Receive BYE if user is already in chat
 * Receive HI if name is OK
 */
int do_connection_protocol(rio_t *rfp);

/**
 * Parse argv and create a ConnectionInfo object.
 * 
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
ConnectionInfo * parse_parameters(int argc, char **argv);

/**
 * Handler to catch Ctrl+C.
 * Sends BYE to Server.
 */
void interrupt_handler(int signal);

/**
 * Execute a user command given by a slash then a command.
 * Ex: /help
 * 
 * @param input [description]
 */
void execute_user_command(char *input);

/**************************** User Commands ***************************/

// /createp <name> <password>	Send CREATEP to the server to create a private 
// 								chatroom with a password.
// /creater <name>				Send CREATER followed by the desired name of the
// 								room. 
// /help 						Print to the screen all user /commands which can be 
// 								entered on the client.
// /join <id>					Send JOIN to the server to join a room.
// /joinp <id> <password>		Send JOINP to the client to join a private chatroom.
// /kick <name>					Send KICK to the server to kick a user.
// /leave						Send LEAVE to the server to leave room.
// /listrooms 					Send LISTR to the server to get list of room names 
// 								and	ids. The list will be terminated by a double 
// 								r\n\r\n.
// /listusers 					Send LISTU to the server to get list of users in
// 								the	room.
// /quit						Quit the chat client.
// /tell <name> <message>		Send TELL to the server to send a private message.

/**
 * Given the user command /createp <name> <password>, send CREATEP to the
 * server to create a private chatroom with a password.
 * 
 * @param input [description]
 */
void send_create_private_chatroom(char *input);

/**
 * Given the user command /creater <name>, send CREATER followed by the
 * desired name of the room to create it.
 * 
 * @param input [description]
 */
void send_create_public_chatroom(char *input);

/**
 * Given the user command /help, print to the screen all the user /commands
 * which can be entered on the client.
 */
void display_user_commands();

/**
 * Given the user command /join <id>, send JOIN to the server to join a room.
 * 
 * @param input [description]
 */
void send_join_public_chatroom(char *input);

/**
 * Given the user command /joinp <id> <password>, send JOINP to the client to
 * join a private chatroom.
 * 
 * @param input [description]
 */
void send_join_private_chatroom(char *input);

/**
 * Given the user command /kick <name>, send KICK to the server to kick a user.
 * 
 * @param input [description]
 */
void send_kick_user(char *input);

/**
 * Given the user command /leave, send LEAVE to the server to leave a chatroom.
 */
void send_leave_chatroom();

/**
 * Given the user command /listrooms, send LISTR to the server to get a list of 
 * room names and ids. The list will be terminated by a double \r\n\r\n.
 */
void send_list_chatrooms();

/**
 * Given the user command /listusers, send LISTU to the server to get list of 
 * users in the room.
 */
void send_list_users_in_chatroom();

/**
 * Given the user command /quit, Exit the chat client.
 * 
 * @return [description]
 */
void do_logout_procedure();

/**
 * Given the user command /tell <name> <message>, send TELL to the server to
 * send a private message.
 */
void send_private_msg(char *input);

/****************************** Verbs***********************************/

/* Verbs sent to server */

/**
 * ALOHA verb sent to server to intiate a log in.
 */
void send_ALOHA_verb(int fd);

/**
 * IAM <name> verb sent to server to request authorization and authentification.
 */
void send_IAM_verb(int fd, char *name);

/**
 * MSG verb to echo a msg whenever a special command is not called.
 */
void send_MSG_verb(int fd, char *msg);

/**
 * BYE verb sent to server to initiate log out.
 */
void send_BYE_verb(int fd);

/**
 * LISTR verb sent to server to receive list of rooms.
 */
void send_LISTR_verb(int fd);

/**
 * LISTU verb sent to server to receive list of users in the current room.
 * TODO: waiting room counts as room?
 */
void send_LISTU_verb(int fd);

/**
 * CREATER verb sent to server to create a public chatroom.
 */
void send_CREATER_verb(int fd, char *roomname);

/**
 * JOIN verb sent to server to join a public chatroom.
 */
void send_JOIN_verb(int fd, char *id);

/**
 * LEAVE verb sent to server to leave a chatroom.
 */
void send_LEAVE_verb(int fd);

/**
 * TELL verb sent to server to send a user in the same chatroom a private message.
 */
void send_TELL_verb(int fd, char *name, char *message);

/**
 * KICK verb sent to server to kick a user from a room. The sender must be the owner.
 */
void send_KICK_verb(int fd, char *username);


void send_IAMNEW_verb(int fd, char *name);
void send_NEWPASS_verb(int fd, char *password);
void send_PASS_verb(int fd, char *password);
void send_CREATEP_verb(int fd, char *room_name, char *password);
void send_JOINP_verb(int fd, char *id, char *password);


/************************* Respond to Verbs ****************************/

/**
 * Receive a verb message, decode it, and react accordingly.
 */
void receive_verb(char *msg);

#endif
