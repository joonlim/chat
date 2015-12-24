/**
 * Definitions of server verb functions
 */
#include "server.h"

/* !AHOLA */
void send_AHOLA_verb(int fd)
{
	Verb *v = make_AHOLA_verb();
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending !AHOLA to client. Terminating.\n");
		// print_usage_server();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_HI_verb(int fd, char *user)
{
	Verb *v = make_HI_verb(user);
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending HI to client. Terminating.\n");
		// print_usage_server();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_BYE_verb(int fd)
{
	Verb *v = make_BYE_verb();
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending BYE to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}
	
	destroy_Verb(v);
	Free(output);
}

void send_ECHO_verb(int fd, char *username, char *msg) 
{
	Verb *v = make_ECHO_verb(username, msg);
	char *output = encode_Verb(v);

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending ECHO to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);		
	}

	destroy_Verb(v);
	Free(output);
}

void send_RTSIL_verb(int fd, struct ChatRoomList *chatroom_list)
{
	Verb *v = make_RTSIL_verb(chatroom_list);
	char *output = encode_Verb(v);
	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending ECHO to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);		
	}

	destroy_Verb(v);
	Free(output);
}

void send_UTSIL_verb(int fd, struct UserList *user_list)
{
	Verb *v = make_UTSIL_verb(user_list);
	char *output = encode_Verb(v);

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending ECHO to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);		
	}

	destroy_Verb(v);
	Free(output);	
}

void send_RETAERC_verb(int fd, char *room_name)
{
	Verb *v = make_RETAERC_verb(room_name);
	char *output = encode_Verb(v);

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending RETAERC to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);		
	}

	destroy_Verb(v);
	Free(output);
}

void send_NIOJ_verb(int fd, char *id)
{
	Verb *v = make_NIOJ_verb(id);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending NIOJ to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_EVAEL_verb(int fd)
{
	Verb *v = make_EVAEL_verb();
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending EVAEL to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}
	
	destroy_Verb(v);
	Free(output);	
}

void send_KCIK_verb(int fd, char *username)
{
	Verb *v = make_KCIK_verb(username);
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending KCIK to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}
	
	destroy_Verb(v);
	Free(output);		
}

void send_KBYE_verb(int fd)
{
	Verb *v = make_KBYE_verb();
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending KBYE to client. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}
	
	destroy_Verb(v);
	Free(output);	
}


void send_LLET_verb(int fd, char *user_to_msg, char *msg)
{
	Verb *v = make_LLET_verb(user_to_msg, msg);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending LLET to client. Terminating.\n");

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}
void send_ECHOP_verb(int fd, char *from_user, char *msg)
{
	Verb *v = make_ECHOP_verb(from_user, msg);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending ECHOP to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_HINEW_verb(int fd, char *name)
{
	Verb *v = make_HINEW_verb(name);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending HINEW to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_AUTH_verb(int fd, char *name)
{
	Verb *v = make_AUTH_verb(name);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending AUTH to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_PETAERC_verb(int fd, char *room_name)
{
	Verb *v = make_PETAERC_verb(room_name);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending PETAERC to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_PNIOJ_verb(int fd, char *id)
{
	Verb *v = make_PNIOJ_verb(id);
	char *output=  encode_Verb(v); 

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending PNIOJ to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

										// optional msg
void send_ERR_verb(int fd, int errorno, char *msg)
{
	Verb *v = make_ERR_verb(errorno, msg);
	char *output = encode_Verb(v);

	/* Print to stdout */
	if (verbose) 
	{
		print_server_to_client(S_C);
		print_server_to_client(output);
	}
	
	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending ERR to client. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);		
	}

	destroy_Verb(v);
	Free(output);
}