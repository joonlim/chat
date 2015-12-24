/**
 * verb.h
 * Verb struct for PIRC
 */
#ifndef __VERB
#define __VERB

#include <string.h>
#include "user_list.h"
#include "chat_room_list.h"

/* Verbs used in Pineapple IRC */

const char *MSG_END = " \r\n";

/* Names */

/* Client -> Server */
const char *ALOHA = 			"ALOHA!";
const char *IAM = 				"IAM";
const char *MSG = 				"MSG";
const char *CREATER =			"CREATER";
const char *LISTR =				"LISTR";
const char *JOIN =				"JOIN";
const char *LEAVE =				"LEAVE";
const char *KICK = 				"KICK";
const char *TELL =				"TELL";
const char *LISTU =				"LISTU";
const char *CREATEP =			"CREATEP";
const char *JOINP =				"JOINP";
const char *IAMNEW =			"IAMNEW";
const char *NEWPASS =			"NEWPASS";
const char *PASS =				"PASS";


/* Server -> Client */
const char *AHOLA =				"!AHOLA";
const char *ECHO =				"ECHO";
const char *ERR =				"ERR";
const char *RETAERC =			"RETAERC";
const char *RTSIL =				"RTSIL";
const char *EVAEL =				"EVAEL";
const char *NIOJ =				"NIOJ";
const char *KCIK =				"KCIK";
const char *KBYE =				"KBYE";
const char *LLET =				"LLET";
const char *ECHOP =				"ECHOP";
const char *UTSIL =				"UTSIL";
const char *PETAERC =			"PETAERC";
const char *PNIOJ =				"PNIOJ";
const char *HINEW =				"HINEW";
const char *HI =				"HI";
const char *AUTH =				"AUTH";
const char *SORRY =				"SORRY";
const char *SERVER =  			"SERVER";				//used in message of the day display - need some way to denote sender of echo
const char *RTSIL_NO_ROOMS =	"no_rooms -1"; 			//the default no rooms available for rtsil


/* Both */
const char *BYE =				"BYE";
const char *SPACE =		" ";

/*
Error Code Table

The following lists the error codes and associated messages.

				ERR  Code Message
				---  ------------
				00   SORRY <name>
				01   USER <name> EXISTS
				02   <name> DOES NOT EXIST
				10 	 ROOM EXISTS
				11   MAXIMUM ROOMS REACHED
				20   ROOM DOES NOT EXIST
				30   USERNOT PRESENT
				40   NOT OWNER
				41   INVALID USER
				60   INVALID OPERATION
				61   INVALID PASSWORD

The default error message for any other conditions which were not characterized should be:

				ERR 100 INTERNAL SERVER ERROR
*/
#define SORRY_ERROR_CODE					00
#define USER_NAME_EXISTS_ERROR_CODE			01
#define NAME_DOES_NOT_EXIST_ERROR_CODE		02
#define ROOM_EXISTS_ERROR_CODE				10
#define MAXIMUM_ROOMS_REACHED_ERROR_CODE	11
#define ROOM_DOES_NOT_EXIST_ERROR_CODE		20
#define USER_NOT_PRESENT_ERROR_CODE			30
#define NOT_OWNER_ERROR_CODE				40
#define INVALID_USER_ERROR_CODE				41
#define INVALID_OPERATION_ERROR_CODE		60
#define INVALID_PASSWORD_ERROR_CODE			61
#define INTERNAL_SERVER_ERROR_CODE			100


#define MAX_ARGS	500

typedef struct
{
    char *name;
    char *args[MAX_ARGS];
    int argc;

} Verb;

/**
 * Constructor for Verb
 * 
 * @return [description]
 */

/**
 * [new_Verb description]
 * 
 * @param  name        [description]
 * @param  args        [description]
 * @param  argc        [input] The size of args
 * @return             New Verb object. Return 0 if name is invalid.
 */
Verb *new_Verb(const char *name, char **args, int argc);

/**
 * Destructor for Verb
 * @param v [description]
 */
void destroy_Verb(Verb *v);

/**
 * Encode a verb struct into a string to be sent over a connection.
 * Returns a new char* that must be deallocated.
 * 
 * @param  v   [description]
 * @param  msg [description]
 * @return     [description]
 */
char *encode_Verb(Verb *v);

/**
 * Decode a message received from a connection into a Verb object.
 * Returns a new Verb object that must be destroyed.
 * 
 * @param  msg [description]
 * @return     [description]
 */
Verb *decode_Verb(char *msg);

/********************** Make specific verbs **********************/

/* Factory for verbs. */
/* When you pass a string to one of these factory functions, it
   gets copied so you do not have to allocate space for it prior
   to calling the function. */

/* These functions are used by both the client and the server */

Verb *make_ALOHA_verb();
Verb *make_AHOLA_verb();

Verb *make_LEAVE_verb();
Verb *make_EVAEL_verb();
Verb *make_BYE_verb();

Verb *make_CREATER_verb(char *room_name);
Verb *make_RETAERC_verb(char *room_name);

Verb *make_LISTR_verb();
Verb *make_RTSIL_verb(struct ChatRoomList *chatroom_list);

Verb *make_LISTU_verb();
Verb *make_UTSIL_verb(struct UserList *user_list);

Verb *make_IAM_verb(char *username);
Verb *make_HI_verb(char *username);

Verb *make_MSG_Verb(char *msg);
Verb *make_ECHO_verb(char *username, char *msg);

Verb *make_ERR_verb(int errorno, char *msg); // optional msg, can be NULL

Verb *make_KICK_verb(char *username);
Verb *make_KCIK_verb(char *username);
Verb *make_KBYE_verb();

Verb *make_JOIN_verb(char *id);
Verb *make_NIOJ_verb(char *id);

Verb *make_TELL_verb(char *name, char *message);
Verb *make_LLET_verb(char *user_to_msg, char *msg);
Verb *make_ECHOP_verb(char *from_user, char *msg);

Verb *make_IAMNEW_verb(char *name); // C > S
Verb *make_HINEW_verb(char *name); // C < S

Verb *make_NEWPASS_verb(char *password); // C > S

Verb *make_AUTH_verb(char *name); // C < S
Verb *make_PASS_verb(char *password); // C > S

Verb *make_CREATEP_verb(char *room_name, char *password); // C > S
Verb *make_PETAERC_verb(char *room_name); // C < S

Verb *make_JOINP_verb(char *id, char *password); // C > S
Verb *make_PNIOJ_verb(char *id); // C < S

#endif