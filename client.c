//C client for server
#include "client_verb.c"

// Hackey way of checking if client should terminate upon entering a wrong password.
int logging_in;

bool creating_p;

bool motd;

int main(int argc, char ** argv) 
{
	// Holds username, ip, and port
	// If the arguments are correct, initialize connectionInfo, other exit the program.
	connectionInfo = parse_parameters(argc, argv);
	// printf("connectionInfo: %s %s %s\n", connectionInfo->name, connectionInfo->server_ip, connectionInfo->server_port);

	// Connect to server!
	if (verbose) printf("**Requesting connection to server.\n");
	clientfd = Open_clientfd(connectionInfo->ip, connectionInfo->port);

	//add signal handler for Ctrl + C
	signal(SIGINT, interrupt_handler);

	/* I/O Multiplexing for stdin and clientfd */
	fd_set active_set, ready_set;
	FD_ZERO(&active_set);					/* Clear read set */
	FD_SET(STDIN_FILENO, &active_set);	/* Add stdin to read set */
	FD_SET(clientfd, &active_set);		/* Add clientfd to read set */
	
	/* Initialize global rio_t rio; */
	Rio_readinitb(&rio, clientfd);

	//before continuing, make sure we can connect safely
	if (verbose) printf("**About to enter connection protocol\n");
	do_connection_protocol(&rio);
	if (verbose) printf("**Finished connection protocol\n");

	// Don't need this anymore
	destroy_ConnectionInfo(connectionInfo);

	//start infinite client loop
	while(1)
	{
		fflush(stdout);
		ready_set = active_set;
		Select(clientfd+1, &ready_set, NULL, NULL, NULL);

		/* Event triggered by for standard input */
		if (FD_ISSET(STDIN_FILENO, &ready_set))
		{
			char *buf = input_buffer;
			if ((read_amt = getline(&buf, &input_len, stdin)) != 0)
			{
				/* If the user just pressed <RETURN>, do nothing. */
				if (strcmp(input_buffer, "\n") == 0)
					continue;

				//remove newline
				char *ptr = input_buffer;
				while(*ptr != '\n')
					ptr++;
				*ptr = '\0';

				/* Here we can check to see if the user typed any special command */
				if (input_buffer[0] == '/')
					execute_user_command(input_buffer);

				//otherwise send the data to the server
				else
				{
					if (verbose) printf("**Sending MSG %s.\n", input_buffer);
					send_MSG_verb(clientfd, input_buffer);
					if(verbose) printf("**Send MSG\n");
				}
			}
		}

		/* Event triggered by for clientfd */
		if (FD_ISSET(clientfd, &ready_set))
		{
			if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
			{
				if(verbose) printf("receieved%sinput\n", input_buffer);
				receive_verb(input_buffer);

				// printf("%s", input_buffer);
				// Rio_writen(clientfd, buf, n);
			}
		}

	} // end loop

    return EXIT_SUCCESS;
}

void interrupt_handler(int sig)
{
	signal(sig, SIG_IGN);

	if (verbose) printf("**CTRL+C hit detected. Cleaning up. \n");

	send_BYE_verb(clientfd);

	//TODO: Implement cleaning of ports
	//for now it only makes sense to close the connection port as it is the only one we have implemented
	Close(clientfd);

	//exit as per ctrl+c
	exit(EXIT_SUCCESS);
}

/**
 * Do the PIRC connection protocol
 * 1. Send ALOHA!
 * 2. Receive !AHOLA
 * 3. Send IAM <name>
 * 4. Receive BYE if username already exists
 * 	  Receive HI if name is OK
 */
int do_connection_protocol(rio_t *rfp)
{
	//printf("about to do connection protocol\n");

	/* Send ALOHA! */
	send_ALOHA_verb(clientfd);
	if (verbose) printf("**Sent ALOHA!\n");

	/* Receive !AHOLA */
	if ((read_amt = Rio_readlineb(rfp, input_buffer, MAX_LINE_SIZE)) != 0) 
	{
		/* After receiving !AHOLA, other verbs are handled */
		if (verbose) printf("**Received !AHOHA\n");
		receive_verb(input_buffer); // !AHOLA
	}
	else
	{
		print_error("Error receiving !AHOLA. Terminating.\n");
		exit(EXIT_FAILURE);
	}

	if (verbose) printf("**Finished Log in protocol.\n");

	return 1;
}

/**
 * ./client [-h] [-c] NAME SERVER_IP SERVER_PORT
 */
ConnectionInfo * parse_parameters(int argc, char **argv)
{
	/* NAME, SERVER_IP, and SERVER_PORT are mandatory. */
	if(argc < 4 || argc > 6)
	{
        print_error("Invalid arguments.\n");
        print_usage_client();
        exit(EXIT_FAILURE);
	}

	int opt; // used to check flags and arguments

    /* Parse short options */
    int should_create_new_user = 0;	
    while((opt = getopt(argc, argv, "hcv")) != -1) 
    {
        switch(opt)
        {
            case 'h':
                /* The help menu was selected */
                print_usage_client();
                exit(EXIT_SUCCESS);
                break;
            case 'c':
            	/* Requests to server to create a new user */
            	// printf("**optar = %s.\n", optarg);
				if(argc < 5)
				{
			        print_error("Invalid arguments.\n");
			        print_usage_client();
			        exit(EXIT_FAILURE);
				}
            	should_create_new_user = 1;
            	break;
            case 'v':
            	/* Debugging */
            	verbose = true;
            	break;
            case '?':
                /* Let this case fall down to default;
                 * handled during bad option.
                 */
            default:
                /* A bad option was provided. */
                print_error("Invalid arguments.\n");
                print_usage_client();
                exit(EXIT_FAILURE);
                break;
        }
    }

    int i;
    // Should be in order: name, ip, and port
    char *name = NULL;
    char *ip = NULL;
    char *port = NULL;

    int count = 0;
    for (i = optind; i < argc; i++)
    {
    	count++;
    	switch(count)
    	{
    		case 1: name = argv[i];
    				break;
    		case 2: ip = argv[i];
    				break;
    		case 3: port = argv[i];
    				break;
    		default: /* error if there are more than 3 arguments */
    				print_error("Invalid arguments.\n");
                	print_usage_client();
                	exit(EXIT_FAILURE);
                	break;
    	}

   		//printf ("Non-option argument %s\n", argv[i]);
    }

    //check to make sure name does not have spaces, if it does, print help menu and exit
    //joon - this is here because i can't do assignment on the constant char ptr in new_ConnectionInfo
    char *dummyPtr = name;
	while(*dummyPtr != '\0')
	{
		if(*dummyPtr == ' ')
		{
			print_error("Username may not contain spaces.\n");
            print_usage_client();
            exit(EXIT_FAILURE);
		}

		dummyPtr++;
	}
	
	/* Initialize ConnectionInfo */
    return new_ConnectionInfo(name, ip, port, should_create_new_user);
}

void execute_user_command(char *input)
{
	/* First get the command, which is the string up to the first ' ' or '\0' */
	char command[100]; // TODO : macro for longest command?
	command[0] = '\0';

	char *space_location = 0;
	if ((space_location = strstr(input, " ")) != 0)
		strncat(command, input, space_location - input);
	else
		strcpy(command, input);

	// Remove spaces at the end of the input
	// Remove spaces at the end of a command
	char iter = input[strlen(input) - 1];
	while (iter == ' ')
	{

		input[strlen(input) - 1] = '\0';
		iter = input[strlen(input) - 1];
	}

	// printf("command: %s\n", command);
	
	/* The functions that start with "send" send verbs to the server. */
	
	/* Ordered alphabetically */
	if (strcmp(command, "/createp") == 0)		/* Create private chatroom with pword */
		send_create_private_chatroom(input);					
	else if(strcmp(command, "/creater") == 0)	/* Create public chatroom */
		send_create_public_chatroom(input);					
	else if(strcmp(command, "/help") == 0)		/* View list of /commands */
		display_user_commands();
	else if(strcmp(command, "/join") == 0)		/* Join a public chatroom */
		send_join_public_chatroom(input);
	else if(strcmp(command, "/joinp") == 0)		/* Join a private chatroom */
		send_join_private_chatroom(input);
	else if(strcmp(command, "/kick") == 0)		/* Kick a user from your chatroom */
		send_kick_user(input);
	else if(strcmp(command, "/leave") == 0)		/* Leave a chatroom. */
		send_leave_chatroom();
	else if(strcmp(command, "/listrooms") == 0)	/* List chatroom names and ids. */
		send_list_chatrooms();
	else if(strcmp(command, "/listusers") == 0)	/* List users in a chatroom. */
		send_list_users_in_chatroom();
	else if(strcmp(command, "/quit") == 0)		/* Log out */
		do_logout_procedure();
	else if(strcmp(command, "/tell") == 0)		/* Send a private message */
		send_private_msg(input);
	else
		fprintf(stderr, COLOR_ERROR"Invalid command:" COLOR_DEFAULT " %s\n", input);
}

//TODO: this function shouldnt return any thing
void receive_verb(char *msg)
{
	// char *return_val = (char *) Malloc(sizeof(char) * MAX_LINE_SIZE);
	Verb *v = decode_Verb(msg);

	// respond to client verbs with functions and then forward server verbs

/***************************************************************************************** !AHOLA */
	if (strcmp(v->name, AHOLA) == 0)
	{

		//This is case where we are not making new user
		if(connectionInfo->is_making_new_user == 0)
		{
			/* When client receiives ALOHA, react by sending IAM <name> */

			/* Send IAM <name> */
			send_IAM_verb(clientfd, connectionInfo->name);
			if (verbose) printf("**Sent IAM %s\n", connectionInfo->name);

			/* Receive HI or ERR/BYE */
			if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
			{
				receive_verb(input_buffer); // HI or ERR/BYE
				// If we reached here, then the verb was HI
				if (verbose) printf("**Received HI\n");	
			}
			else
			{
				print_error("Error receiving HI. Terminating.\n");
				exit(EXIT_FAILURE);
			}
		}
		//we are making a new user
		else
		{
			//we must react by sending IAMNEW <name>
			send_IAMNEW_verb(clientfd, connectionInfo->name);
			if (verbose) printf("**Sent IAMNEW %s\n", connectionInfo->name);

			/* Receive HI or ERR/BYE */
			if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
			{
				receive_verb(input_buffer); // HI or ERR/BYE
				// If we reached here, then the verb was HI
				if (verbose) printf("**Received HI\n");
			}
			else
			{
				print_error("Error receiving HI. Terminating.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
/***************************************************************************************** ECHO */
	else if (strcmp(v->name, ECHO) == 0)
	{
		// translate ECHO 
		
		// receive ECHO 
		// ex: user > Hello, cse320!

		if (motd)
		{
			Rio_writen(1, (char*)COLOR_INFO, strlen(COLOR_INFO));
			write_array(1, v->args + 1, v->argc - 1);
			Rio_writen(1, (char*)"\n", strlen("\n"));
			Rio_writen(1, (char*)COLOR_DEFAULT, strlen(COLOR_DEFAULT));
		}
		else
		{
			Rio_writen(1, (char*)v->args[0], strlen(v->args[0]));
			Rio_writen(1, (char*)" > ", strlen(" > "));

			write_array(1, v->args + 1, v->argc - 1);
			Rio_writen(1, (char*)"\n", strlen("\n"));		
		}


	}
/***************************************************************************************** ECHOP */
	else if (strcmp(v->name, ECHOP) == 0)
	{
		// private message
		// print_private_msg(v->args[0]);
		// print_private_msg(" > ");
		// printf(COLOR_PM);
		// write_array(1, v->args + 1, v->argc - 1);
		// print_private_msg("\n");
		Rio_writen(1, (char*)COLOR_PM, strlen(COLOR_PM));
		Rio_writen(1, (char*)v->args[0], strlen(v->args[0]));
		Rio_writen(1, (char*)" > ", strlen(" > "));

		write_array(1, v->args + 1, v->argc - 1);
		Rio_writen(1, (char*)"\n", strlen("\n"));
		Rio_writen(1, (char*)COLOR_DEFAULT, strlen(COLOR_DEFAULT));
		
	}
/***************************************************************************************** HI */
	else if (strcmp(v->name, HI) == 0)
	{
		if(verbose) printf("**GOT %s\n", v->name);

		/* Receive ECHO server message of the day */
		if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
		{
			receive_verb(input_buffer); // ECHO server motd
			if (verbose) printf("**Received ECHO server motd\n");
		}
		else
		{
			print_error("Error receiving ECHO server motd. Terminating.\n");
			exit(EXIT_FAILURE);
		}
	}
/***************************************************************************************** HINEW */
	else if (strcmp(v->name, HINEW) == 0)
	{
		if (verbose) printf("**HINEW received.\n");

		/*Prompt for password and send NEWPASS verb*/
		printf("Enter password for user: ");

		fgets(input_buffer, MAX_LINE_SIZE, stdin);

		//remove the newline from password
		char *dummyPtr = input_buffer;

		while(*dummyPtr != '\n')
			dummyPtr++;
		*dummyPtr = '\0';

		send_NEWPASS_verb(clientfd, input_buffer);
		if (verbose) printf("**NEWPASS sent.\n");

		// This is so the client exists when they input an invalid password.

		logging_in = 2;
		motd = true;

		if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
		{
			receive_verb(input_buffer); // BYE
			// program will exit
		}
		else
		{
			print_error("Error receiving BYE. Terminating.\n");
			exit(EXIT_FAILURE);
		}

		logging_in = 0;
		motd = false;

	}
/***************************************************************************************** BYE */
	else if (strcmp(v->name, BYE) == 0)
	{
		if (verbose) printf("**BYE received.\n");

		print_info("Logging out...\n");

		// Now we can exit! 
		exit(EXIT_SUCCESS); // EXIT HERE!

	}
/***************************************************************************************** RTSIL */
	else if (strcmp(v->name, RTSIL) == 0)
	{
		if (verbose) printf("**RTSIL received.\n");

		if (strcmp(v->args[0], "no_rooms -1") == 0)
		{
			// no rooms
			print_info("There are no available rooms.\n");
			char buffer[MAX_MESSAGE_SIZE];
			Rio_readlineb(&rio, buffer, MAX_MESSAGE_SIZE);	// read the next '\r\n'

		}
		else
		{
			/* We currently only have RTSIL and the first item in the list */
			print_info("\nList of available rooms:\n");
			print_info("------------------------\n");
			char *name = strtok(v->args[0], " ");
			char *id = strtok(NULL, " ");
			int is_private = atoi(strtok(NULL, " "));
			
			//if public room
			if(is_private == 1)
			{
				// printf("printing public room.\n");
				print_info(name);
				print_info("		");
				print_info(id);
				print_info("\n");
			}
			//if private room
			else
			{
				// printf("printing private room\n");
				print_private_room(name);
				print_private_room("		");
				print_private_room(id);
				print_private_room("\n");
			}

			/* Read the rest of the list */
			char buffer[MAX_MESSAGE_SIZE];
			while(Rio_readlineb(&rio, buffer, MAX_MESSAGE_SIZE)) 
			{
				if (strcmp(buffer, "\r\n") == 0)
				{
					// Finished reading input
					break;
				}
				char *name = strtok(buffer, " ");
				char *id = strtok(NULL, " ");
				int is_private = atoi(strtok(NULL, " "));
				
				//if public room
				if(is_private == 1)
				{
					//LEAVE THIS CHECK IN JOON - should really beat up pub/priv display
					if (verbose) printf("printing public room.\n");
					print_info(name);
					print_info("		");
					print_info(id);
					print_info("\n");
				}
				//if private room
				else
				{	
					//LEAVE THIS CHECK IN JOON - should really beat up pub/priv display
					if (verbose) printf("printing private room\n");
					print_private_room(name);
					print_private_room("		");
					print_private_room(id);
					print_private_room("\n");
				}
			}

			fprintf(stdout, COLOR_DEFAULT"\n");
		}
	}
/***************************************************************************************** UTSIL */
	else if (strcmp(v->name, UTSIL) == 0)
	{
		if (verbose) printf("**UTSIL received.\n");

		/* We currently only have UTSIL and the first item in the list */
		print_info("\nAvailable users:\n");
		print_info("----------------\n");
		char *username = v->args[0];
		print_info(username);
		print_info("\n");

		/* Read the rest of the list */
		char buffer[MAX_MESSAGE_SIZE];
		while(Rio_readlineb(&rio, buffer, MAX_MESSAGE_SIZE)) 
		{
			if (strcmp(buffer, "\r\n") == 0)
			{
				// Finished reading input
				break;
			}
			char *username = strtok(buffer, " ");
			print_info(username);
			print_info("\n");
		}

		fprintf(stdout, COLOR_DEFAULT"\n");
	}
/***************************************************************************************** RETAERC */
	else if (strcmp(v->name, RETAERC) == 0)
	{
		if (verbose) printf("**RETAERC received.\n");

		print_info("You have created a public chat room.\n");
	}
/***************************************************************************************** NIOJ */
	else if (strcmp(v->name, NIOJ) == 0)
	{
		if (verbose) printf("**NIOJ received.\n");	

		print_info("You have joined the chat room.\n");
	}
/***************************************************************************************** EVAEL */
	else if (strcmp(v->name, EVAEL) == 0)
	{
		if (verbose) printf("**EVAEL received.\n");	

		print_info("You have left the chat room.\n");
	}
/***************************************************************************************** LLET */
	else if (strcmp(v->name, LLET) == 0)
	{
		if (verbose) printf("**LLET received.\n");	
	}
/***************************************************************************************** KBYE */
	else if (strcmp(v->name, KBYE) == 0)
	{
		if (verbose) printf("**KBYE received.\n");	

		print_info("You have been kicked out of the chat room.\n");
	}
/***************************************************************************************** HINEW */
//	else if (strcmp(v->name, HINEW) == 0)
//	{
//		if (verbose) printf("**HINEW received.\n");

		// TODO	?
//	}
/***************************************************************************************** AUTH */
	else if (strcmp(v->name, AUTH) == 0)
	{
		if (verbose) printf("**AUTH received.\n");

		//Ask user to enter password.
		printf("Enter password for user: ");

		fgets(input_buffer, MAX_LINE_SIZE, stdin);

		//remove the newline from password
		char *dummyPtr = input_buffer;

		while(*dummyPtr != '\n')
			dummyPtr++;
		*dummyPtr = '\0';

		//send password to server
		send_PASS_verb(clientfd, input_buffer);
		if (verbose) printf("**PASS sent.\n");

		// This is so the client exists when they input a wrong password.

		logging_in = 1;
		motd = true;

		if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
		{
			receive_verb(input_buffer); // BYE
			// program will exit
		}
		else
		{
			print_error("Error receiving BYE. Terminating.\n");
			exit(EXIT_FAILURE);
		}

		logging_in = 0;
		motd = false;

	}
/***************************************************************************************** PETAERC */
	else if (strcmp(v->name, PETAERC) == 0)
	{
		if (verbose) printf("**PETAERC received.\n");	

		print_info("You have created a private chat room.\n");
	}
/***************************************************************************************** PNIOJ */
	else if (strcmp(v->name, PNIOJ) == 0)
	{
		if (verbose) printf("**PNIOJ received.\n");	

		print_info("You have joined the chat room.\n");
	}
/***************************************************************************************** ERR */
	else if (strcmp(v->name, ERR) == 0)
	{
		int err_code = atoi(v->args[0]);

		switch(err_code)
		{
			case SORRY_ERROR_CODE:
							// 00   SORRY <name>
							print_error("Sorry! ");
							Rio_writen(1, (char*)COLOR_ERROR, strlen(COLOR_ERROR));
							write_array(1, v->args + 2, v->argc - 2);
							print_error("\n");
							Rio_writen(1, (char*)COLOR_DEFAULT, strlen(COLOR_DEFAULT));
							break;

			case USER_NAME_EXISTS_ERROR_CODE: 
							// 01   USER <name> EXISTS
							// printf("Is making new user? %i\n", connectionInfo->is_making_new_user);

							if(connectionInfo->is_making_new_user == 0)
								print_error("This user is already logged in. Terminating.\n");
							else
								if (logging_in == 1)
								{
									print_error("This user is already logged in. Terminating.\n");
								}
								else
								{
									print_error("This username already exists. Terminating.\n");
								}
							// print_usage_client();
							if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
							{
								receive_verb(input_buffer); // BYE
								// program will exit
							}
							else
							{
								print_error("Error receiving BYE. Terminating.\n");
								exit(EXIT_FAILURE);
							}
							break;
			case NAME_DOES_NOT_EXIST_ERROR_CODE:

							// 02   <name> DOES NOT EXIST
							if(connectionInfo->is_making_new_user == 0)
								print_error("This username does not exist. Terminating.\n");
							else
								print_error("Room does not exist.\n");
							break;
			case ROOM_EXISTS_ERROR_CODE:

							// 10 	 ROOM EXISTS
							print_error("Room with same name already exists.\n");
							break;
			case MAXIMUM_ROOMS_REACHED_ERROR_CODE:

							// 11   MAXIMUM ROOMS REACHED
							print_error("Maximum number of rooms reached.\n");
							break;
			case ROOM_DOES_NOT_EXIST_ERROR_CODE:

							// 20   ROOM DOES NOT EXIST
							print_error("Room does not exist.\n");
							break;
			case USER_NOT_PRESENT_ERROR_CODE:

							// 30   USER NOT PRESENT
							print_error("User is not online.\n");
							break;
			case NOT_OWNER_ERROR_CODE:

							// 40   NOT OWNER
							print_error("You are not the owner of this room.\n");
							break;
			case INVALID_USER_ERROR_CODE:

							// 41   INVALID USER
							print_error("Invalid username.\n");
							break;
			case INVALID_OPERATION_ERROR_CODE:

							// 60   INVALID OPERATION
							print_error("Invalid operation.\n");
							break;
			case INVALID_PASSWORD_ERROR_CODE:

							// 61   INVALID PASSWORD

							if (logging_in == 1) // login bad password
							{
								print_error("User and password do not match.\n");

								if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
								{
									receive_verb(input_buffer); // BYE
									// program will exit
								}
								else
								{
									print_error("Error receiving BYE. Terminating.\n");
									exit(EXIT_FAILURE);
								}
							}
							else if (logging_in == 2) // user creation bad pass
							{
								print_error("Password must meet the following criteria to be valid:\n");
								printf("	Must be at least 5 characters in length\n");
								printf("	Must contain at least 1 uppercase character\n");
								printf("	Must contain at least 1 symbol character\n");
								printf("	Must contain at least 1 number character\n");
								
								if ((read_amt = Rio_readlineb(&rio, input_buffer, MAX_LINE_SIZE)) != 0) 
								{
									receive_verb(input_buffer); // BYE
									// program will exit
								}
								else
								{
									print_error("Error receiving BYE. Terminating.\n");
									exit(EXIT_FAILURE);
								}
							}
							else if (creating_p) // creating private room
							{
								print_error("Password must meet the following criteria to be valid:\n");
								printf("	Must be at least 5 characters in length\n");
								printf("	Must contain at least 1 uppercase character\n");
								printf("	Must contain at least 1 symbol character\n");
								printf("	Must contain at least 1 number character\n");
								
							}
							else
								print_error("Invalid password.\n");

							break;
			default: 
							// 100 INTERNAL SERVER ERROR
							print_error("Internal server error.\n");
		}

	}

	destroy_Verb(v); // must destroy every time
}

/**************************** User Commands ***************************/

void display_user_commands()
{
	// /help

	print_info("List of user commands:\n"
		 "/createp <name> <password>  Create a private chatroom with a name and password.\n"
		 "/creater <name>             Create a public chatroom with a name.\n"
		 "/help                       View list of user commands.\n"
		 "/join <id>                  Join a public chatroom using an ID.\n"
		 "/joinp <id> <password>      Join a private chatroom using an ID and password.\n"
		 "/kick <name>                Kick a user from your chatroom.\n"
		 "/leave                      Leave a chatroom if you are in one.\n"
		 "/listrooms                  List chatroom names and IDs.\n"
		 "/listusers                  List users in a chatroom if you are in one.\n"
		 "/quit                       Log out.\n"
		 "/tell <name> <message>      Send a private message to an online user.\n");
}

void send_join_public_chatroom(char *input)
{
	// /join <id>
	// 
	// send_JOIN

	/* If id not given, do not continue */
	char *space = strstr(input, " ");
	if (!space)
	{
		print_error("/join: You must specify a room ID!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	/* Start /join protocol */

	// Parse the id number
	char *id = space + 1;

	send_JOIN_verb(clientfd, id);

	if (verbose) printf("**Sent JOIN %s.\n", id);

	// Receive NIOJ, ERR 20, or ERR 60
	char input_text[MAX_MESSAGE_SIZE];
	if (Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
	}

	if (verbose) printf("**Finished join room protocol.\n");
}

void send_join_private_chatroom(char *input)
{
	// /joinp <id> <password>
	// 
	// send_JOINP

	/* If id not given, do not continue */
	char *space = strstr(input, " ");
	if (!space)
	{
		print_error("/joinp: You must specify a room ID and enter a password!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	/* If room password not given, do not continue */
	space = strstr(space + 1, " ");
	if (!space)
	{
		print_error("/joinp: You must enter a password!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	//parse out room name and password
	//strtok once to get past /createp
	strtok(input, " ");

	//strtok again to get room name
	char *room_id = strtok(NULL, " ");
	//strtok last time to get room password
	char *room_password = strtok(NULL, " ");

	/* Start /join protocol */
	//TODO
	send_JOINP_verb(clientfd, room_id, room_password);

	if (verbose) printf("**Sent JOINP %s %s.\n", room_id, room_password);

	// Receive NIOJ, ERR 20, or ERR 60
	char input_text[MAX_MESSAGE_SIZE];
	if (Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// printf("got response from server for password. It is: %s\n", input_text);
		// Respond to client's verb call
		receive_verb(input_text);
	}

	if (verbose) printf("**Finished join private room protocol.\n");
}

void send_kick_user(char *input)
{
	// /kick <name>
	// 
	// send_KICK
	
	/* If a user's name not given, do not continue */
	char *space = strstr(input, " ");
	if (!space)
	{
		print_error("/kick: You must specify a user!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	/* Start /kick protocol */
	// Parse the username number
	char *username = space + 1;

	send_KICK_verb(clientfd, username);

	if (verbose) printf("**Sent KICK %s.\n", username);

	// Receive KCIK, ERR 40, or ERR 41
	char input_text[MAX_MESSAGE_SIZE];
	if (Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
	}

	if (verbose) printf("**Finished kick user protocol.\n");

}

void send_leave_chatroom()
{
	// /leave
	// 
	// send_LEAVE
	
	/* Start /leave protocol */
	send_LEAVE_verb(clientfd);

	if (verbose) printf("Sent LEAVE.\n");

	/* Server should respond with EVAEL verb */

	//wait for EVAEL or ERR 30
	char input_text[MAX_MESSAGE_SIZE];
	if(Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
		// printf("%s", input_text);
	}
}

void send_create_private_chatroom(char *input)
{
	// /createp <name> <password>
	// 
	// send_CREATEP

	/* If room name not given, do not continue */
	char *space = strstr(input, " ");
	if (!space)
	{
		print_error("/createp: Private rooms must have a name and password!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	/* If room password not given, do not continue */
	space = strstr(space + 1, " ");
	if (!space)
	{
		print_error("/createp: Private rooms must have a password!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	//Get the room name and password name
	//strtok once to get past /createp
	strtok(input, " ");

	//strtok again to get room name
	char *room_name = strtok(NULL, " ");
	//strtok last time to get room password
	char *room_password = strtok(NULL, " ");

	creating_p = true;

	/* Start /createp protocol */
	send_CREATEP_verb(clientfd, room_name, room_password);

	if(verbose) printf("***Sent CREATEP %s %s\n", room_name, room_password);

	//wait on response to come back from server
	// Receive PETAERC or ERR 60.
	char input_text[MAX_MESSAGE_SIZE];
	bzero(input_text, MAX_MESSAGE_SIZE);
	if (Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
	}

	creating_p = false;

	if (verbose) printf("**Finished create private room protocol.\n");
}

// TODO : Within a chat room only
void send_create_public_chatroom(char *input)
{
	// /creater <name> - waiting room command
	// 
	// send CREATER

	/* If room name not given, do not continue */
	char *space = strstr(input, " ");
	if (!space)
	{
		print_error("/creater: Rooms must have a name!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	/* Start /creater protocol */

	// Parse the room name
	char *room_name = space + 1;

	send_CREATER_verb(clientfd, room_name);

	if (verbose) printf("**Sent CREATER %s.\n", room_name);

	// Receive RETAERC or ERR 60.
	char input_text[MAX_MESSAGE_SIZE];

	if (Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
	}

	if (verbose) printf("**Finished create room protocol.\n");
}

void send_list_chatrooms()
{
	// /listrooms
	// 
	// send_LISTR
	
	send_LISTR_verb(clientfd);

	/* Server should respond with RTSIL verb */
	/* followed by a list of room names and IDs separated by \r\n */
	/* Stop listening when we received the last '\r\n' */
	
	//wait for RTSIL

	char input_text[MAX_MESSAGE_SIZE];

	if(Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
		// printf("%s", input_text);
	}
}

void send_list_users_in_chatroom()
{
	// /listusers
	// 
	// send_LISTU

	send_LISTU_verb(clientfd);

	/* Server should respond with UTSIL verb */
	/* followed by a list of room names and IDs separated by \r\n */
	/* Stop listening when we received the last '\r\n' */
	
	//wait for UTSIL

	char input_text[MAX_MESSAGE_SIZE];

	if(Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text);
		// printf("%s", input_text);
	}
}

void do_logout_procedure()
{
	// /quit
	// 
	// send_BYE

	if (verbose)  printf("**Sending BYE\n");
	send_BYE_verb(clientfd);
	
	/* Server should respond with BYE verb */

	char input_text[MAX_MESSAGE_SIZE];

	if(Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's BYE call
		receive_verb(input_text);
		
	}

	// If we reached here something is wrong
	
	//received bye, so now we can exit properly
	exit(0);
}

void send_private_msg(char *input)
{
	// /tell
	// 
	// send_TELL

	/* If user's name is not given, do not continue */
	char *space = strstr(input, " ");
	if (!space)
	{
		print_error("/tell: You must specify a user and enter a message to send!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	char *name = space + 1;

	/* If a message is not given, do not continue */
	space = strstr(space + 1, " ");
	if (!space)
	{
		print_error("/tell: You must enter a message to send!\n");
		print_error("Type '/help' for more information.\n");
		return;
	}

	// null terminame name
	*space = '\0';

	/* Start /tell protocol */

	// TODO: Message can have spaces so parse correctly.
	/* Start /kick protocol */
	// Parse the id number
	char *message = space + 1;

	send_TELL_verb(clientfd, name, message);

	if (verbose) printf("**Sent TELL %s %s.\n", name, message);

	// Receive LLET, or ERR 30
	char input_text[MAX_MESSAGE_SIZE];
	if (Rio_readlineb(&rio, input_text, MAX_MESSAGE_SIZE)) 
	{
		// Respond to client's verb call
		receive_verb(input_text); // LLET or ERR 30
	}

	if (verbose) printf("**Finished private message protocol.\n");

}