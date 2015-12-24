/**
 * Definitions of client verb functions
 */
#include "client.h"

/* AHOLA! */
void send_ALOHA_verb(int fd)
{
	Verb *v = make_ALOHA_verb();
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending ALOHA! to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_IAM_verb(int fd, char *name)
{
	Verb *v = make_IAM_verb(name);
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending IAM ");
		print_error(name);
		print_error(" to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_MSG_verb(int fd, char *input)
{
	Verb *v = make_MSG_Verb(input);
	char *output = encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending MSG to server. Terminating.\n");
		// print_usage_client();

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

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending BYE to server. Terminating.\n"); // ironic
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}
	
	destroy_Verb(v);
	Free(output);
}

void send_LISTR_verb(int fd)
{
	Verb *v = make_LISTR_verb();
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending LISTR to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_LISTU_verb(int fd)
{
	Verb *v = make_LISTU_verb();
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending LISTU to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_CREATER_verb(int fd, char *roomname)
{
	Verb *v = make_CREATER_verb(roomname);
	char *output = encode_Verb(v);

	if (send(clientfd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending CREATER to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_JOIN_verb(int fd, char *id)
{
	Verb *v = make_JOIN_verb(id);
	char *output = encode_Verb(v);

	if (send(clientfd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending JOIN to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_LEAVE_verb(int fd)
{
	Verb *v = make_LEAVE_verb();
	char *output = encode_Verb(v);

	if (send(clientfd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending LEAVE to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_TELL_verb(int fd, char *name, char *message)
{
	Verb *v = make_TELL_verb(name, message);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending TELL to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_KICK_verb(int fd, char *username)
{
	Verb *v = make_KICK_verb(username);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending KICK to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_IAMNEW_verb(int fd, char *name)
{
	Verb *v = make_IAMNEW_verb(name);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending IAMNEW to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_NEWPASS_verb(int fd, char *password)
{
	Verb *v = make_NEWPASS_verb(password);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending NEWPASS to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);	
}

void send_PASS_verb(int fd, char *password)
{
	Verb *v = make_PASS_verb(password);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending PASS to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_CREATEP_verb(int fd, char *room_name, char *password)
{
	Verb *v = make_CREATEP_verb(room_name, password);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending CREATEP to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}

void send_JOINP_verb(int fd, char *id, char *password)
{
	Verb *v = make_JOINP_verb(id, password);
	char *output=  encode_Verb(v); // convert to " \r\n" terminated string

	if (send(fd, output, strlen(output), 0) < 0)
	{
		print_error("Error sending JOINP to server. Terminating.\n");
		// print_usage_client();

		destroy_Verb(v);
		Free(output);

		exit(EXIT_SUCCESS);
	}

	destroy_Verb(v);
	Free(output);
}
