/**
 * verb.c
 */
#include "verb.h"
#include "utils/csapp.h"

Verb *new_Verb(const char *name, char **args, int argc)
{
	// First, check to see that the name is valid.
	if ((strcmp(name, ALOHA))  		||
		(strcmp(name, IAM))			||	// <name>
		(strcmp(name, MSG))			||	// <message>
		(strcmp(name, CREATER))		||	// <name>
		(strcmp(name, LISTR))		||
		(strcmp(name, JOIN))		||	// <id> 
		(strcmp(name, LEAVE))		||
		(strcmp(name, KICK))		||	// <username>
		(strcmp(name, TELL))		||	// <name> <message>
		(strcmp(name, LISTU))		||
		(strcmp(name, CREATEP))		||	// <name> <password>
		(strcmp(name, JOINP))		||	// <id>
		(strcmp(name, IAMNEW))		||	// <name>
		(strcmp(name, NEWPASS))		||	// <password>
		(strcmp(name, PASS))		||	// <password>
		(strcmp(name, AHOLA))		||
		(strcmp(name, ECHO))		||	// <username> <message>
		(strcmp(name, ERR))			||	// <num> <message>
		(strcmp(name, RETAERC))		||	// <name>
		(strcmp(name, RTSIL))		||	// <room1> <id1> <type1> \r\n <room2> <id2> ...\r\n\r\n
		(strcmp(name, NIOJ))		||	// <id>
		(strcmp(name, KCIK))		||	// <username>
		(strcmp(name, KBYE))		||
		(strcmp(name, LLET))		||	// <name> <message>
		(strcmp(name, ECHOP))		||	// <sendername> <message>
		(strcmp(name, UTSIL))		||	// <name1> \r\n<name2> \r\n<name3>...
		(strcmp(name, PETAERC))		||	// <name>
		(strcmp(name, PNIOJ))		||	// <id>
		(strcmp(name, HINEW))		||	// <name>
		(strcmp(name, HI))			||	// <name>
		(strcmp(name, AUTH))		||	// <name>
		(strcmp(name, BYE)))
	{
		Verb *v = (Verb *)Malloc(sizeof(Verb));
	
		v->name = (char *)Malloc(sizeof(char) * strlen(name) + 1);
		strcpy(v->name, name);

		v->argc = argc;

		// copy the string for every string in args
		int i;
		for (i = 0; i < argc; i++)
		{
			v->args[i] = (char *)Malloc(sizeof(char) * strlen(args[i]) + 1);
			strcpy(v->args[i], args[i]);
		}	

		return v;
	}

	// else, return -1
	return 0;
}

void destroy_Verb(Verb *v)
{
	if (!v)
		return;

	Free((void*)v->name);

	// free every string in args
	int i;
	for (i = 0; i < v->argc; i++)
		Free(v->args[i]);

	Free((void*)v);
}

char *encode_Verb(Verb *v)
{
	// RTSIL and UTSIL work differently
	if (strcmp(v->name, RTSIL) == 0 || strcmp(v->name, UTSIL) == 0)
	{
		// RTSIL <room1> <id1> \r\n <room2> <id2> \r\n <room3> <id3> \r\n\r\n
		int size = strlen(v->name);
		size += 2; // "\r\n"
		char *message = (char *)Malloc((sizeof(char) * size) + 1);

		strcpy(message, v->name);

		int i;
		for (i = 0; i < v->argc; i++)
		{
			size += strlen(v->args[i]);
			size += 4; // 2 spaces, \r\n
			message = (char *)Realloc(message, (sizeof(char) * size) + 1);
			strcat(message, " ");
			strcat(message, v->args[i]);
			strcat(message, MSG_END);
		}

		strcat(message, "\r\n"); // different from MSG_END because there is no space in the front.

		return message;
	}

	// returned string will be in the form : "v->name <args[0]> <args[1]> ... <args[argc-1]> \r\n
	// ex: "ECHO <username> <message> \r\n"
	//     "LLET <name> <message> \r\n"
	//     "ALOHA! \r\n"
	
	int size = strlen(v->name);
	size += 3;// " \r\n"
	char *message = (char *)Malloc((sizeof(char) * size) + 1);

	strcpy(message, v->name);

	int i;
	for (i = 0; i < v->argc; i++)
	{
		size += strlen(v->args[i]);
		size += 1; // space
		message = (char *)Realloc(message, (sizeof(char) * size) + 1);
		strcat(message, " ");
		strcat(message, v->args[i]);

	}

	strcat(message, MSG_END);

	return message;
}

Verb *decode_Verb(char *msg)
{
	// First, copy msg so we can call strtok on it.
	char msgcpy[strlen(msg) + 1];
	sprintf(msgcpy, "%s", msg);

	char *argument = strtok(msgcpy, " \t\r\n");

	char *name = argument;

	if(name[strlen(name) - 1] == ' ')
		name[strlen(name) - 1] = '\0';

	// RTSIL and UTSIL work differently
	if (strcmp(name, RTSIL) == 0 || strcmp(name, UTSIL) == 0)
	{
		// RTSIL <room1> <id1> \r\n 
		// <room2> <id2> \r\n 
		// <room3> <id3> \r\n
		// \r\n

		argument = strtok(NULL, "\r\n");
		int argc = 1;
		char *args[1];

		if (argument[strlen(argument) - 1] == ' ')	// get rid of space at end
			argument[strlen(argument) - 1] = '\0';
		if (argument[0] == ' ')						// get rid of space at front
			argument = argument + 1;

		args[0] = argument;

		Verb *v = new_Verb(name, args, argc);

		return v;
	}


	argument = strtok(NULL, " \t\r\n");

	int argc = 0;

	char *args[MAX_ARGS];

	// tokenize the arguments
	while (argument)
	{
		args[argc] = argument;

		argument = strtok(NULL, " \t\r\n");
		argc++;
	}

	Verb *v = new_Verb(name, args, argc);
	return v;
}

Verb *make_ALOHA_verb()
{
	const char *name = ALOHA;
	
	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_AHOLA_verb()
{
	const char *name = AHOLA;
	
	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_BYE_verb()
{
	const char *name = BYE;

	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_CREATER_verb(char *room_name)
{
	const char *name = CREATER;

	char *args[1];
	args[0] = room_name;

	Verb *v = new_Verb(name, args, 1);
	return v;
}

Verb *make_RETAERC_verb(char *room_name)
{
	const char *name = RETAERC;

	char *args[1];
	args[0] = room_name;

	Verb *v = new_Verb(name, args, 1);
	return v;
}


Verb *make_LISTR_verb()
{
	const char *name = LISTR;

	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_IAM_verb(char *username)
{
	const char *name = IAM;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;
}

Verb *make_HI_verb(char *username)
{
	const char *name = HI;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;
}

Verb *make_MSG_Verb(char *msg)
{
	const char *name = MSG;

	char *args[1];
	args[0] = msg;

	Verb *v = new_Verb(name, args, 1);
	return v;
}

Verb *make_LEAVE_verb()
{
	const char *name = LEAVE;

	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_LISTU_verb()
{
	const char *name = LISTU;

	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_KICK_verb(char *username)
{
	const char *name = KICK;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;
}

Verb *make_KCIK_verb(char *username)
{
	const char *name = KCIK;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;
}


Verb *make_KBYE_verb()
{
	const char *name = KBYE;

	Verb *v = new_Verb(name, NULL, 0);
	return v;
}

Verb *make_ECHO_verb(char *username, char *msg)
{
	const char *name = ECHO;

	char *args[2];
	args[0] = username;
	args[1] = msg;

	Verb *v = new_Verb(name, args, 2);
	return v;
}

Verb *make_EVAEL_verb()
{
	const char *name = EVAEL;

	Verb *v = new_Verb(name, NULL, 0);
	return v;	
}

Verb *make_JOIN_verb(char *id)
{
	const char *name = JOIN;

	char *args[1];
	args[0] = id;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}

Verb *make_NIOJ_verb(char *id)
{
	const char *name = NIOJ;

	char *args[1];
	args[0] = id;

	Verb *v = new_Verb(name, args, 1);
	return v;		
}

Verb *make_TELL_verb(char *username, char *message)
{
	const char *name = TELL;

	char *args[2];
	args[0] = username;
	args[1] = message;

	Verb *v = new_Verb(name, args, 2);
	return v;
}

Verb *make_LLET_verb(char *user_to_msg, char *msg)
{
	const char *name = LLET;

	char *args[2];
	args[0] = user_to_msg;
	args[1] = msg;

	Verb *v = new_Verb(name, args, 2);
	return v;
}

Verb *make_ECHOP_verb(char *from_user, char *msg)
{
	const char *name = ECHOP;

	char *args[2];
	args[0] = from_user;
	args[1] = msg;

	Verb *v = new_Verb(name, args, 2);
	return v;
}

Verb *make_IAMNEW_verb(char *username)
{
	const char *name = IAMNEW;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}
Verb *make_HINEW_verb(char *username)
{
	const char *name = HINEW;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}

Verb *make_NEWPASS_verb(char *password)
{
	const char *name = NEWPASS;

	char *args[1];
	args[0] = password;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}

Verb *make_AUTH_verb(char *username)
{
	const char *name = AUTH;

	char *args[1];
	args[0] = username;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}

Verb *make_PASS_verb(char *password)
{
	const char *name = PASS;

	char *args[1];
	args[0] = password;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}

Verb *make_CREATEP_verb(char *room_name, char *password)
{
	const char *name = CREATEP;

	char *args[2];
	args[0] = room_name;
	args[1] = password;

	Verb *v = new_Verb(name, args, 2);
	return v;
}

Verb *make_PETAERC_verb(char *room_name)
{
	const char *name = PETAERC;

	char *args[1];
	args[0] = room_name;

	Verb *v = new_Verb(name, args, 1);
	return v;	
}

Verb *make_JOINP_verb(char *id, char *password)
{
	const char *name = JOINP;

	char *args[2];
	args[0] = id;
	args[1] = password;

	Verb *v = new_Verb(name, args, 2);
	return v;
}

Verb *make_PNIOJ_verb(char *id)
{
	const char *name = PNIOJ;

	char *args[1];
	args[0] = id;

	Verb *v = new_Verb(name, args, 1);
	return v;		
}

Verb *make_RTSIL_verb(struct ChatRoomList *chatroom_list)
{
	// RTSIL <room1> <id1> <type1> \r\n <room2> <id2> <type2> \r\n <room3> <id3> <type3> \r\n\r\n
	// RTSIL no_rooms -1 \r\n\r\n	

	// Each char* in args[] has "<room> <id>"
	
	const char *name = RTSIL;

	if (chatroom_list->size == 0)
	{
		char *args[1];
		char *arg = "no_rooms -1";
		args[0] = arg;
		Verb *v = new_Verb(name, args, 1);
		return v;
	}

	char **args = NULL;
	struct ChatRoom *room = chatroom_list->head;
	int numrooms = 0;
	while (room)
	{
		args = (char**)Realloc(args, sizeof(char*) * numrooms);

		// Get number of digits in id
		int numdigits = 0;
		int n = room->room_id;
		while(n != 0)
		{
		    n /= 10;            
		    ++numdigits;
		}
		if (numdigits == 0)
			numdigits = 1;

		//set 1 for public, 2 for private
		int publicPrivate;
		if(room->is_public)
			publicPrivate = 1;
		else
			publicPrivate = 2;

		args[numrooms] = (char *)Malloc(strlen(room->room_name) + 1 + numdigits + 1 + 1 + 1); //last 3 1's - space private? space
		sprintf(args[numrooms], "%s %d %d", room->room_name, room->room_id, publicPrivate);

		numrooms++;
		room = room->next_link;
	}	

	Verb *v = new_Verb(name, args, numrooms);

	// Free args
	int i;
	for (i = 0; i < numrooms; i++)
	{
		Free(args[i]);
	}
	Free(args);

	return v;
}

Verb *make_UTSIL_verb(struct UserList *user_list)
{
	// UTSIL <name1> \r\n<name2> \r\n<name3> \r\n\r\n
	// There is always at least one name (the caller)	

	// Each char* in args[] has "<name>"
	
	const char *name = UTSIL;

	char *args[user_list->size];
	struct User *user = user_list->head;
	int argc = 0;
	while (user)
	{
		args[argc] = user->user_name;
		argc++;
		user = user->next_link;
	}

	Verb *v = new_Verb(name, args, argc);
	return v;
}

Verb *make_ERR_verb(int errorno, char *msg) // optional msg, can be NULL
{
	const char *name = ERR;
	
	char *args[4];	// max 4 different args
	int argc = 0;
	Verb *v = NULL;

	switch(errorno)
	{
		case SORRY_ERROR_CODE:

						// 00   SORRY <name>
						args[argc++] = "00";
						args[argc++] = "SORRY";
						args[argc++] = msg;		// <name>

						break;

		case USER_NAME_EXISTS_ERROR_CODE: 

						// 01   USER <name> EXISTS
						args[argc++] = "01";
						args[argc++] = "USER";
						args[argc++] = msg;		// username
						args[argc++] = "EXISTS";

						break;
		case NAME_DOES_NOT_EXIST_ERROR_CODE:

						// 02   <name> DOES NOT EXIST
						args[argc++] = "02";
						args[argc++] = msg; 		// <name>
						args[argc++] = "DOES NOT EXIST";

						break;
		case ROOM_EXISTS_ERROR_CODE:

						// 10 	 ROOM EXISTS
						args[argc++] = "10";
						args[argc++] = "ROOM EXISTS";

						break;
		case MAXIMUM_ROOMS_REACHED_ERROR_CODE:

						// 11   MAXIMUM ROOMS REACHED
						args[argc++] = "11";
						args[argc++] = "MAXIMUM ROOMS REACHED";

						break;
		case ROOM_DOES_NOT_EXIST_ERROR_CODE:

						// 20   ROOM DOES NOT EXIST
						args[argc++] = "20";
						args[argc++] = "ROOM DOES NOT EXIST";

						break;
		case USER_NOT_PRESENT_ERROR_CODE:

						// 30   USER NOT PRESENT
						args[argc++] = "30";
						args[argc++] = "USER NOT PRESENT";

						break;
		case NOT_OWNER_ERROR_CODE:

						// 40   NOT OWNER
						args[argc++] = "40";
						args[argc++] = "NOT OWNER";

						break;
		case INVALID_USER_ERROR_CODE:

						// 41   INVALID USER
						args[argc++] = "41";
						args[argc++] = "INVALID USER";

						break;
		case INVALID_OPERATION_ERROR_CODE:

						// 60   INVALID OPERATION
						args[argc++] = "60";
						args[argc++] = "INVALID OPERATION";
						break;
		case INVALID_PASSWORD_ERROR_CODE:

						// 61   INVALID PASSWORD
						args[argc++] = "61";
						args[argc++] = "INVALID PASSWORD";
						break;
		default: 

						// 100 INTERNAL SERVER ERROR
						args[argc++] = "100";
						args[argc++] = "INTERNAL SERVER ERROR";
	}

	v = new_Verb(name, args, argc);
	return v;
}