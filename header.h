#ifndef __HEADER
#define __HEADER

/**
 * header.h
 * Include this file inside client.h and server.h
 */
#include <stdio.h>
#include <stdlib.h>
#include "utils/csapp.h"
#include "stdbool.h"
#include "verb.c"

#define MAX_LINE_SIZE 1001
#define PRINTED_USAGE_FLAG 100

const char *S_C = "C < S | ";
const char *C_S = "C > S | ";

/******************************** Colors *******************************/

#define COLOR_INFO		"\x1B[1;34m"	// BBlue
#define COLOR_ERROR		"\x1B[1;31m"	// BRed
#define COLOR_PM		"\x1B[1;35m"	// BPurple
#define COLOR_DEFAULT	"\x1B[0m"

#define COLOR_C_S		"\x1B[1;33m"	// BYellow
#define COLOR_S_C		"\x1B[1;36m"	// BCyan

void print_info(const char *string)
{
	fprintf(stdout, COLOR_INFO"%s"COLOR_DEFAULT, string);
}

void print_private_room(const char *string)
{
	fprintf(stdout, COLOR_PM"%s"COLOR_DEFAULT, string);
}

void print_error(const char *string)
{
	fprintf(stderr, COLOR_ERROR"%s"COLOR_DEFAULT, string);
}

void print_private_msg(const char *string)
{
	fprintf(stdout, COLOR_PM"%s"COLOR_DEFAULT, string);
}

void print_server_to_client(const char *string)
{
	fprintf(stdout, COLOR_S_C"%s"COLOR_DEFAULT, string);
}

void print_client_to_server(const char *string)
{
	fprintf(stdout, COLOR_C_S"%s"COLOR_DEFAULT, string);
}

/**************************** Usage **************************************/

/**
 * Print usage message for client.
 */
void print_usage_client()
{
	print_info("Usage:\n"
		   "./client [-h] [-c] NAME SERVER_IP SERVER_PORT\n"
		   "-h            Displays help menu & returns EXIT_SUCCESS.\n"
		   "-c            Requests to server to create a new user\n"
		   "\n"
		   " NAME         This is the Username to display when chatting.\n"
		   " SERVER_IP    IP address of server to connect to.\n"
		   " SERVER_PORT  Port to connect too.\n");
}

/**
 * Print usage message for server.
 */
void print_usage_server()
{
	print_info("Usage:\n"
		   "./server [-he] [-N num] PORT_NUMBER MOTD\n"
		   "-e            Echo messages received on server's stdout.\n"
		   "-h            Displays help menu & returns EXIT_SUCCESS.\n"
		   "-N num        Specifies maximum number of chat rooms allowed\n"
		   "              on server. Default = 5\n"
		   "\n"
		   " PORT_NUMBER  Port number to listen on.\n"
		   " MOTD         Message to display to the client when they connect.\n");
}

/****************************** ConnectionInfo *******************************/
typedef struct
{
	//passed in params needed to establish connection
	char *name;
	char *ip;
	char *port;

	//used to see if we are amking a new user. 0 - false, 1 - true
	int is_making_new_user;

} ConnectionInfo;

/**
 * Constructor for ConnectionInfo.
 * 
 * @param  name [description]
 * @param  ip   [description]
 * @param  port [description]
 * @return      [description]
 */
ConnectionInfo *new_ConnectionInfo(const char *name, const char *ip, const char *port, const int should_create_new_user)
{
	ConnectionInfo *ci = (ConnectionInfo *)Malloc(sizeof(ConnectionInfo));

	ci->name = (char *)Malloc(sizeof(char) * strlen(name) + 1);
	strcpy(ci->name, name);

	ci->ip = (char *)Malloc(sizeof(char) * strlen(ip) + 1);
	strcpy(ci->ip, ip);

	ci->port = (char *)Malloc(sizeof(char) * strlen(port) + 1);
	strcpy(ci->port, port);

	ci->is_making_new_user = should_create_new_user;

	return ci;
}

/**
 * Destructor for ConnectionInfo.
 * @param ci [description]
 */
void destroy_ConnectionInfo(ConnectionInfo* ci)
{
	if (!ci)
		return;

	Free((void*)ci->name);
	Free((void*)ci->ip);
	Free((void*)ci->port);
	Free((void*)ci);
}

/******************************* ServerInfo **********************************/
typedef struct
{
	//passed in params needed to establish connection
	int port;
	char *port_s; // string version of port for Open_listenfd()
	char *motd;

} ServerInfo;

/**
 * Constructor for ServerInfo.
 * 
 * @param  port [description]
 * @param  motd        [description]
 * @return             [description]
 */
ServerInfo *new_ServerInfo(const char *port, const char *motd)
{
	ServerInfo *si = (ServerInfo *)Malloc(sizeof(ServerInfo));

	si->port = atoi(port);

	si->port_s = (char *)Malloc(sizeof(char) * strlen(port) + 1);
	strcpy(si->port_s, port);

	si->motd = (char *)Malloc(sizeof(char) * strlen(motd) + 1);
	strcpy(si->motd, motd);

	return si;
}

/**
 * Destructor for ServerInfo.
 * @param si [description]
 */
void destroy_ServerInfo(ServerInfo* si)
{
	if (!si)
		return;

	Free((void*)si->port_s);
	Free((void*)si->motd);
	Free((void*)si);
}

#endif