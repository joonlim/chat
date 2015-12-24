//C client for server
#include "server_verb.c"

int main(int argc, char ** argv) 
{
	init_globals();

	// Holds server_port an motd
	serverInfo = parse_parameters(argc, argv);
	
	if (verbose > 1) printf("**serverInfo: %s %s\n", serverInfo->port_s, serverInfo->motd);

	// If the port number is not a well known port, exit
	if(serverInfo->port < 1024)	
	{
		print_error("Can not use \"well known port numbers.\" Terminating.\n");
		print_usage_server();

		return EXIT_SUCCESS;
	}

	// Open listening file descriptor
	listenfd = Open_listenfd(serverInfo->port_s);

	/* SUCCESS! */

	// print the port number the server is currently listening on
	printf("Currently listening on port %i.\n", serverInfo->port);

	// This main thread is ALWAYS waiting for clients attempting to connect.
	while(1)
	{
		struct sockaddr_in client_address;
		socklen_t client_length = sizeof(client_address);
		
		// open a the connection
		// accept is blocking so will wait for connection to appear
		int *connfd = (int*)Malloc(sizeof(int));
		*connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_length);

		if(connfd < 0)
		{
			print_error("Error accepting input from server. Terminating.\n");
			// print_usage_server();

			return EXIT_SUCCESS;
		}
	
		pthread_t thread_id;	// id of next thread to be created

		// create a thread to handle the log in process
		if(pthread_create(&thread_id, NULL, login_thread, connfd) != 0)
		{
			print_error("Error creating login thread, terminating.\n");
			// print_usage_server();
			return EXIT_FAILURE;
		}

		if (verbose > 1) printf("**Created login thread.\n");
		//join the thread to run it
		// pthread_join(thread_id, NULL);
	}

	destroy_ServerInfo(serverInfo);
    return EXIT_SUCCESS;
}

ServerInfo *parse_parameters(int argc, char **argv)
{
	/* PORT_NUMBER, and MOTD are mandatory. */
	if(argc < 3 || argc > 7)
	{
        print_error("Invalid arguments.\n");
        print_usage_server();
        exit(EXIT_FAILURE);
	}

	int opt; // used to check flags and arguments

    /* Parse short options */	
    while((opt = getopt(argc, argv, "heN:v")) != -1) 
    {
        switch(opt)
        {
            case 'h':
                /* The help menu was selected */
                print_usage_server();
                exit(EXIT_SUCCESS);
                break;
            case 'e':
            	/* Echo messages received on server's stdout. */
            	verbose_echo = 1;
            	break;
            case 'N':
            	/* Specifies maximum number of chat rooms allowed on server. */
            	// printf("**optar = %s.\n", optarg);
				if(argc < 5)
				{
			        print_error("Invalid arguments.\n");
			        print_usage_server();
			        exit(EXIT_FAILURE);
				}

            	max_num_rooms = atoi(optarg);
            	break;
            case 'v':
            	/* Debugging */
            	verbose++;
            	break;
            case '?':
                /* Let this case fall down to default;
                 * handled during bad option.
                 */
            default:
                /* A bad option was provided. */
                fprintf(stderr, "Invalid arguments.\n");
                print_usage_server();
                exit(EXIT_FAILURE);
                break;
        }
    }

    int i;
    // Should be in order: port, motd
    char *port = NULL;
    char *motd = NULL;

    int count = 0;
    for (i = optind; i < argc; i++)
    {
    	count++;
    	switch(count)
    	{
    		case 1: port = argv[i];
    				break;
    		case 2: motd = argv[i];
    				break;
    		default: /* error if there are more than 2 arguments */
    				fprintf(stderr, "Invalid arguments.\n");
                	print_usage_server();
                	exit(EXIT_FAILURE);
                	break;
    	}

   		//printf ("Non-option argument %s\n", argv[i]);
    }

	/* Initialize ServerInfo */
    return new_ServerInfo(port, motd);
}

void interrupt_handler(int sig)
{
	signal(sig, SIG_IGN);

	if (verbose > 1) printf("**CTRL+C hit detected. Cleaning up. \n");

	// Send BYE to all open sockets
	// TO ALL CONNECTED USERS!
	struct User *u = all_users_list->head;
	while (u)
	{
		send_BYE_verb(u->connfd);
		u = u->next_link;
	}

	//TODO: Implement cleaning of ports
	//for now it only makes sense to close the connection port as it is the only one we have implemented
	Close(listenfd);

	destroy_ServerInfo(serverInfo);

	// exit as per ctrl + c
	exit(EXIT_SUCCESS);
}

// void interrupt_chatroom_select_handler(int sig)
// {
// 	signal(sig, SIGUSR1);

// 	if (verbose > 1) printf("This is a mistake.\n");

// 	return;
// }

void init_globals()
{
	verbose = 0;
	next_chat_room_id = 0;
	verbose_echo = 0;
	max_num_rooms = DEFAULT_MAX_NUM_ROOMS;

	total_num_rooms = 0;

	/* mutex = 1 */
	Sem_init(&mutex, 0, 1);

	//clear the read set and active set for echo
	FD_ZERO(&echo_thread_active_set);
	FD_ZERO(&echo_thread_ready_set);

	//clear the read set and active set for waiting room
	FD_ZERO(&waiting_room_thread_active_set);
	FD_ZERO(&waiting_room_thread_ready_set);

	/* Time */
	timeout.tv_sec = 1;
	timeout.tv_usec = 3;

	echo_fd_count = 0;
	waiting_room_fd_count = 0;


	//add signal handler for Ctrl + C
	signal(SIGINT, interrupt_handler);

	// TODO: IS THIS CORRECT?
	/*
	You should look to handle:
			EINTR
			SIGPIPE / EPIPE
			ICONNREFUSED
			ECHILD
			EINVL
			etc
	 */
	signal(EINTR, interrupt_handler);
	signal(SIGPIPE, interrupt_handler);
	signal(EPIPE, interrupt_handler);
	signal(ECHILD, interrupt_handler);

	// Create the user list that the server uses to keep track of ALL connected users
	all_users_list = new_UserList();

	// Create the user list that keeps track of users in waiting room.
	waiting_room_users_list = new_UserList();
	
	// Create the chat room list that is to keep track of active chat rooms
	chatroom_list = new_ChatRoomList();
}

void *chatroom_thread(void *vargp)
{
	// signal(SIGUSR1, interrupt_chatroom_select_handler);

	struct ChatRoom *chatroom = (struct ChatRoom*)vargp;

	fd_set ready_set;
	FD_ZERO(&ready_set);

	/* There is a unique chatroom_thread for each */
	while(1)
	{
		if (chatroom->users->size == 0)
		{
			if (verbose > 1) printf("**Detected empty chat room %s. Killing thread.\n", chatroom->room_name);

			// Delete chatroom from list
			remove_by_id_ChatRoomList(chatroom_list, chatroom->room_id);
			
			total_num_rooms--;

			pthread_exit(NULL);
		}

		ready_set = chatroom->connected_users_set;
		Select(chatroom->fd_count + 1, &ready_set, NULL, NULL, &timeout);

		int i;
		int max = chatroom->fd_count;
		for (i = 0; i < max + 1; i++)
		{
			if(FD_ISSET(i, &ready_set))
			{
				char input_text[MAX_INPUT_SIZE];
				rio_t rp;
				Rio_readinitb(&rp, i);
				if(Rio_readlineb(&rp, input_text, MAX_INPUT_SIZE)) 
				{

					if (verbose > 1) printf("**In chatroom %s, read: %s", chatroom->room_name ,input_text);

					receive_verb(i, input_text);
				}
				else
				{
					print_error("Chatroom thread: Error reading from socket. Terminating.\n");
					// print_usage_server();

					exit(EXIT_SUCCESS);					
				}
			}
		}
	}

	return (void *) malloc(sizeof(void *)); // TODO ????
}

/* unique to each user */
void *login_thread(void *vargp)
{
	// copy a local copy fo the passed file descriptor
	int client_fd = *((int *) vargp);
	
	// while there is text, read it
	char input_text[MAX_INPUT_SIZE];
	rio_t rp;
	Rio_readinitb(&rp, client_fd);

	if(Rio_readlineb(&rp, input_text, MAX_INPUT_SIZE)) 
	{
		/* Client initiates log in with ALOHA! */
		/* In receive_verb(), server should respond with !AHOLA */
		receive_verb(client_fd, input_text);
	}
	if(Rio_readlineb(&rp, input_text, MAX_INPUT_SIZE)) 
	{
		/* Client responds with IAM/IAMNEW */

		/* Client responds with IAM <name> */
		/* In receive_verb(), we validate name */

		// If user exists already, 
		// 		send the ERR verb with correct code.
		// 		Then send bye and terminate this thread.
		// Else, 
		// 		send the HI verb confirming a successful log in.
		
		

		// Connect the user and add user to list of connected users.
		receive_verb(client_fd, input_text);
	}
	if(Rio_readlineb(&rp, input_text, MAX_INPUT_SIZE)) 
	{
		/* Client responds with PASS/NEW PASS*/

		// Then send an ECHO message to all connected clients
		// alerting them that a new user has connected.

		// Send the user that just connected the MOTD.
		receive_verb(client_fd, input_text);

		pthread_exit(NULL);
	}

	return (void *) malloc(sizeof(void *)); // TODO ????
}

void *waiting_room_thread(void *vargp)
{
	/* 
		In the waiting room users can
		1	Create chat rooms
		2	List the available chat rooms
		3	Join a chat room
		4	Log out of the server
	*/
	while(1)
	{
		//check to see if the waiting room flag is ever set to 0, if so kill the thread
		if (waiting_room_flag == 0)
		{
			if (verbose > 1) printf("**Killing waiting room thread.\n");
			pthread_exit(NULL);
		}

		if (waiting_room_fd_count > 0)
		{
			waiting_room_thread_ready_set = waiting_room_thread_active_set;
			//add one for whatever reason to highest file descriptor we listen too
			Select(waiting_room_fd_count + 1, &waiting_room_thread_ready_set, NULL, NULL, &timeout);		
			
			//process every connection that is ready to be served
			int i;
			for(i = 0; i < waiting_room_fd_count + 1; i++)
			{
				/* This loop will keep executing until wating room thread is killed */

				if(FD_ISSET(i, &waiting_room_thread_ready_set))
				{
					char input_text[MAX_INPUT_SIZE];
					rio_t rp;
					Rio_readinitb(&rp, i);

					if(Rio_readlineb(&rp, input_text, MAX_INPUT_SIZE)) 
					{

						if (verbose > 1) printf("**In waiting room thread, read: %s", input_text);

						/* Client in waiting room sends verb here */

						receive_verb(i, input_text);
					}
					else
					{
						print_error("Waiting room thread: Error reading from socket. Terminating.\n");
						// print_usage_server();

						exit(EXIT_SUCCESS);					
					}
				}
			}	
		}
	}
}

void *echo_thread(void *vargp)
{
	while(1)
	{
		//check to see if the echo flag is ever set to 0, if so kill the thread
		if (echo_flag == 0)
		{
			if (verbose > 1) printf("**Killing echo thread.\n");
			pthread_exit(NULL);
		}

		//look at file descriptors and check if any of them are ready for reading
		//int select(int nfds, fd_set *readfds, fd_set *writefds,
          // fd_set *exceptfds, struct timeval *timeout);
		//if we have ready descriptors to read
		if (echo_fd_count > 0)
		{
			echo_thread_ready_set = echo_thread_active_set;
			//add one for whatever reason to highest file descriptor we listen too
			Select(echo_fd_count + 1, &echo_thread_ready_set, NULL, NULL, &timeout);

			// printf("In echo thread: ready_val: %i\n", ready_val);

			//process every connection that is ready to be served
			int i;
			for(i = 0; i < echo_fd_count + 1; i++)
			{
				/* This loop will keep executing until echo thread is killed */

				if(FD_ISSET(i, &echo_thread_ready_set))
				{
					char input_text[MAX_INPUT_SIZE];
					rio_t rp;
					Rio_readinitb(&rp, i);

					if(Rio_readlineb(&rp, input_text, MAX_INPUT_SIZE)) 
					{

						if (verbose > 1) printf("**In echo thread, read: %s", input_text);
						/* Client initiates log in with ALOHA! */
						/* In receive_verb(), server should respond with !AHOLA */
						receive_verb(i, input_text);
					}
					else
					{
						print_error("Echo thread: Error reading from socket. Terminating.\n");
						// print_usage_server();

						exit(EXIT_SUCCESS);					
					}

				}
			}
		}
	}

	return (void *) malloc(sizeof(void *));
}

int receive_verb(int connfd, char *msg)
{
	/* Print the Verb we received to stdout */
	if (verbose)
	{
		print_client_to_server(C_S);
		print_client_to_server(msg);
	}

	int ret_val = 0;
	Verb *v = decode_Verb(msg);
	
	// if user is currently in waiting room, send error 60 mes
	if(FD_ISSET(connfd, &waiting_room_thread_active_set) 
		&& !is_allowed_in_waiting_room(v->name))	// Verb is not allowed while this client is in waiting room
	{
		//C < S | ERR 60 <message> \r\n
		send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
	}
	else
	{				
		// else do normal procedure
		// respond to client verbs with functions and then forward server verbs

/***************************************************************************************** ALOHA! */
		if (strcmp(v->name, ALOHA) == 0)
		{
			// Respond to AHOLA! with !AHOLA
			send_AHOLA_verb(connfd);
		}
/***************************************************************************************** IAM */
		else if(strcmp(v->name, IAM) == 0)
		{
			// If user exists already, 
			// 		send the ERR verb with correct code.
			// 		Then send bye and terminate this thread.

			char *username = v->args[0];
			if (user_exists_in_UserList(all_users_list, username))
			{
				if (verbose > 1) printf("**User already logged in.\n");

				send_ERR_verb(connfd, SORRY_ERROR_CODE, "That username is already logged in."); 	// Is
				send_BYE_verb(connfd);
				pthread_exit(NULL); // terminates login thread.
			}
			else if (check_if_username_exists_in_file(username) != USER_NAME_TAKEN)
			{
				// Username does not exist.
				if (verbose > 1) printf("**Username does not exist.\n");

				send_ERR_verb(connfd, NAME_DOES_NOT_EXIST_ERROR_CODE, username); 	// Is
				send_BYE_verb(connfd);
				pthread_exit(NULL); // terminates login thread.	
			}
			else
			{
				// Insert into front of UserList all_users_list, under the assumption we will remove user if pw authentication fails
				insert_into_UserList(all_users_list, username, NULL, connfd, 0);

				// Insert into front of UserList waiting_room_users_list, under the assumption we will remove user if pw authentication fails
				insert_into_UserList(waiting_room_users_list, username, NULL, connfd, 0);

				// send the HI verb confirming a successful log in.
				//send_HI_verb(connfd, username);

				// send the AUTH verb to request pw
				send_AUTH_verb(connfd, username);
			}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PART 1
/// 
			// If echo thread exists,
			// send an ECHO message to all connected clients
			// alerting them that a new user has connected.
			// if (echo_flag)
			// {
			// 	// send "server > user has connected."
			// 	char *user = "server";
			// 	char *has_connected = "has connected.";
			// 	char buff[strlen(has_connected) + strlen(username) + 2];
			// 	sprintf(buff, "%s %s", username, has_connected);
				
			// 	if (verbose_echo)
			// 		print_verbose_echo(user, buff);

			// 	// TO ALL CONNECTED USERS!
			// 	struct User *u = all_users_list->head;
			// 	while (u)
			// 	{
			// 		send_ECHO_verb(u->connfd, user, buff);
			// 		u = u->next_link;
			// 	}
			// }
			// else
			// {
			// 	// Else, create the echo thread
			// 	echo_flag = 1; // set flag

			// 	//create echo thread
			// 	if (pthread_create(&echo_thread_id, NULL, echo_thread, NULL) != 0) {
			// 		printf("Error creating echo thread, terminating.\n");
			// 		// print_usage_server();

			// 		exit(EXIT_FAILURE);
			// 	}
			// 	if (verbose > 1) printf("**Created echo thread.\n");
			// }
/////////////////////////////////////////////////////////////////////////////////////////////////////////


		}
/***************************************************************************************** MSG */
		else if (strcmp(v->name, MSG) == 0)
		{
			//TODO fix
			// to clients
			char *username = get_username_using_fd_UserList(all_users_list, connfd);

			/* convert array to string to pass */
			char buf[MAX_LINE_SIZE];
			buf[0] = '\0';
			strcat(buf, v->args[0]);
			int j = 1;
			for (; j < v->argc; j++)
			{
				strcat(buf, " ");
				strcat(buf, v->args[j]);
			}

			if (verbose_echo)
				print_verbose_echo(username, buf);

			// Echo this to whichever chatroom our username is in.
			struct ChatRoom *chatroom = chatroom_list->head;
			while(chatroom)
			{
				// check if user's name is in the chatroom
				char *name = get_username_using_fd_UserList(chatroom->users, connfd);
				if (strcmp(name, username) == 0)
					break;

				chatroom = chatroom->next_link;
			}

			struct User *u = chatroom->users->head;
			while (u)
			{
				send_ECHO_verb(u->connfd, username, buf);
				u = u->next_link;
			}

		}
/***************************************************************************************** TELL */
		else if(strcmp(v->name, TELL) == 0)
		{
			char *user_to_msg = v->args[0];
			// TODO max length 1000 bytes.
			// 
			// First, grab the ChatRoom that the user belongs to
			char *username = get_username_using_fd_UserList(all_users_list, connfd);
			struct ChatRoom *chatroom = chatroom_list->head;
			while(chatroom)	
			{
				// check if user's name is in a chatroom
				char *user = get_username_using_fd_UserList(chatroom->users, connfd);
				if (!user)
				{
					chatroom = chatroom->next_link;
					continue;
				}
				if (strcmp(user, username) == 0)
					break;

				chatroom = chatroom->next_link;
			}

			if (!user_exists_in_UserList(chatroom->users, user_to_msg))
			{
				// If there is no user with given name in room
				// ERR 30
				send_ERR_verb(connfd, USER_NOT_PRESENT_ERROR_CODE, NULL);
			}
			else
			{
				// Send private message
				/* convert array to string to pass */
				char buf[MAX_LINE_SIZE];
				buf[0] = '\0';
				strcat(buf, v->args[1]);
				int j = 2;
				for (; j < v->argc; j++)
				{
					strcat(buf, " ");
					strcat(buf, v->args[j]);
				}

				// LLET to sender
				send_LLET_verb(connfd, user_to_msg, buf);
				
				if (verbose_echo)	// TODO: should we make this pink?
					print_verbose_echo(username, buf);

				// ECHOP to receiver
				int msg_fd = get_fd_using_username_UserList(all_users_list, user_to_msg);
				send_ECHOP_verb(msg_fd, username, buf);
			}

		}
/***************************************************************************************** BYE */
		else if(strcmp(v->name, BYE) == 0)
		{
			remove_user_from_chatrooms(connfd, false);

			// TODO FIX
			send_BYE_verb(connfd);

			char *username = get_username_using_fd_UserList(all_users_list, connfd);

			// now remove the user from username list
			remove_by_fd_UserList(all_users_list, connfd);

			// // Remove from active echo set
			// if (FD_ISSET(connfd, &echo_thread_active_set))
			// 	FD_CLR(connfd, &echo_thread_active_set);

			// Remove from active waiting room set
			remove_from_waiting_room(connfd);

			// // if there are no more users, kill the echo thread and set its flag to 1
			// // also kill waiting room flag
			// if(all_users_list->size == 0 || waiting_room_users_list->size == 0)
			// {
			// 	waiting_room_flag = 0;
			// 	echo_flag = 0;
			// }
			if (waiting_room_flag)
			{
				// ECHO server <username> has disconnected. \r\n
				char *user = "server";
				char *has_disconnected = "has disconnected.";
				char buff[strlen(has_disconnected) + strlen(username) + 2];
				sprintf(buff, "%s %s", username, has_disconnected);
				
				if (verbose_echo)
					print_verbose_echo(user, buff);

				// TODO: to all users or users in waiting room or users in chat room?
				// waiting_room_users_list
				// TO ALL CONNECTED USERS!
				struct User *u = all_users_list->head;
				while (u)
				{
					send_ECHO_verb(u->connfd, user, buff);
					u = u->next_link;
				}
			}
		}
/***************************************************************************************** LISTR */
		else if(strcmp(v->name, LISTR) == 0)
		{
			// If the client is not in the waiting room,
			// send ERR 60
			struct User *user = get_User_using_fd_UserList(waiting_room_users_list, connfd);
			if (user)
			{
				// server will respond with UTSIL followed by a list
				// of room name and ids. The list will be termianted 
				// by a double \r\n\r\n.
				
				send_RTSIL_verb(connfd, chatroom_list);
			}
			else
			{
				// Server is not in waiting room.
				send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
			}
		}
/***************************************************************************************** LISTU */
		else if(strcmp(v->name, LISTU) == 0)
		{
			// If the client is not in the waiting room,
			// send ERR 60
			struct User *u = get_User_using_fd_UserList(waiting_room_users_list, connfd);
			if (!u)
			{
				// First, grab the ChatRoom that the user belongs to
				char *username = get_username_using_fd_UserList(all_users_list, connfd);
				struct ChatRoom *chatroom = chatroom_list->head;
				while(chatroom)	
				{
					// check if user's name is in a chatroom
					char *user = get_username_using_fd_UserList(chatroom->users, connfd);
					if (!user)
					{
						chatroom = chatroom->next_link;
						continue;
					}
					if (strcmp(user, username) == 0)
						break;

					chatroom = chatroom->next_link;
				}

				// server will respond with UTSIL followed by a list
				// of usernames. The list will be termianted 
				// by a double \r\n\r\n.
				
				send_UTSIL_verb(connfd, chatroom->users);
			}
			else
			{
				// Server is not in waiting room.
				send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
			}
		}
/***************************************************************************************** CREATEP */
		else if(strcmp(v->name, CREATEP) == 0)
		{
			//TODO- for now just makes pulbic room. Change to private later

			if (verbose > 1) printf("**About to create private room\n");
			char *username = get_username_using_fd_UserList(all_users_list, connfd);
			char *room_name = v->args[0];
			char *room_password = v->args[1];

			// Lock here
			P(&mutex);

			if (!(username = get_username_using_fd_UserList(waiting_room_users_list, connfd)))
			{
				// If client is already inside room (not in waiting room)
				// ERR 60	
				send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
			}
			else if (name_exists_in_ChatRoomList(chatroom_list, room_name))
			{
				// If a room with same name exists,
				// respond with ERR 10
				send_ERR_verb(connfd, ROOM_EXISTS_ERROR_CODE, NULL);
			}
			else if (total_num_rooms >= max_num_rooms)
			{
				// If the maximum number of rooms is reached, 
				// respond with ERR 11.		
				send_ERR_verb(connfd, MAXIMUM_ROOMS_REACHED_ERROR_CODE, NULL);
			}
			else if (!validatePassword(room_password))
			{
				send_ERR_verb(connfd, INVALID_PASSWORD_ERROR_CODE, NULL);
			}
			else
			{
				create_new_private_room(room_name, room_password, connfd);

				// Remove user from list of users in waiting room.
				remove_from_waiting_room(connfd);

				/* Tell client that the new room has been created. */
				send_PETAERC_verb(connfd, room_name);

				// Inform all users in waiting room that a new room has been added.
				char *user = "server";
				char *newchat = "New Private Chat room ";
				char *added = " added";
				char buff[strlen(newchat) + strlen(added) + strlen(room_name) + 1];
				sprintf(buff, "%s%s%s", newchat, room_name, added);

				if (verbose_echo)
					print_verbose_echo(user, buff);
				
				struct User *u = waiting_room_users_list->head;
				while (u)
				{
					send_ECHO_verb(u->connfd, user, buff);
					u = u->next_link;
				}	
			}

			// Unlock here	
			V(&mutex);	
		}
/***************************************************************************************** CREATER */
		else if(strcmp(v->name, CREATER) == 0)
		{
			if (verbose > 1) printf("**About to create room\n");
			char *username = get_username_using_fd_UserList(all_users_list, connfd);
			char *room_name = v->args[0];

			// Lock here
			P(&mutex);

			if (!(username = get_username_using_fd_UserList(waiting_room_users_list, connfd)))
			{
				// If client is already inside room (not in waiting room)
				// ERR 60	
				send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
			}
			else if (name_exists_in_ChatRoomList(chatroom_list, room_name))
			{
				// If a room with same name exists,
				// respond with ERR 10
				send_ERR_verb(connfd, ROOM_EXISTS_ERROR_CODE, NULL);
			}
			else if (total_num_rooms >= max_num_rooms)
			{
				// If the maximum number of rooms is reached, 
				// respond with ERR 11.		
				send_ERR_verb(connfd, MAXIMUM_ROOMS_REACHED_ERROR_CODE, NULL);
			}
			else
			{
				create_new_room(room_name, connfd);

				// Remove user from list of users in waiting room.
				remove_from_waiting_room(connfd);

				/* Tell client that the new room has been created. */
				send_RETAERC_verb(connfd, room_name);

				// Inform all users in waiting room that a new room has been added.
				char *user = "server";
				char *newchat = "New Chat room ";
				char *added = " added";
				char buff[strlen(newchat) + strlen(added) + strlen(room_name) + 1];
				sprintf(buff, "%s%s%s", newchat, room_name, added);
				
				if (verbose_echo)
					print_verbose_echo(user, buff);

				struct User *u = waiting_room_users_list->head;
				while (u)
				{
					send_ECHO_verb(u->connfd, user, buff);
					u = u->next_link;
				}	
			}

			// Unlock here	
			V(&mutex);
		
		}
///***************************************************************************************** CREATEP */
//		else if(strcmp(v->name, CREATEP) == 0)
//		{
//			if (verbose > 1) printf("**About to create private room\n");
//
//			//TODO finish this.
//			remove_user_from_chatrooms(connfd, false); // remove from chatroom if they are already in one
//			// send_PETAERC_verb(asdf);
//		}
/***************************************************************************************** KICK */
		else if(strcmp(v->name, KICK) == 0)
		{
			char *username_to_kick = v->args[0];
			// First, grab the ChatRoom that the user belongs to
			char *username = get_username_using_fd_UserList(all_users_list, connfd);
			struct ChatRoom *chatroom = chatroom_list->head;
			while(chatroom)	
			{
				// check if user's name is in a chatroom
				char *user = get_username_using_fd_UserList(chatroom->users, connfd);
				if (!user)
				{
					chatroom = chatroom->next_link;
					continue;
				}
				if (strcmp(user, username) == 0)
					break;

				chatroom = chatroom->next_link;
			}

			char *owner_name = chatroom->owner->user_name;
			if (strcmp(owner_name, username) != 0)
			{
				// If this user is not the owner of the room
				// send ERR 40	
				send_ERR_verb(connfd, NOT_OWNER_ERROR_CODE, NULL);		
			}
			else if (!user_exists_in_UserList(chatroom->users, username_to_kick))
			{
				// If there is no user with a given username,
				// send ERR 41	
				send_ERR_verb(connfd, INVALID_USER_ERROR_CODE, NULL);			
			}
			else
			{
				// we are good to kick username_to_kick.
				if (strcmp(username_to_kick, username) == 0)
				{
					fprintf(stdout, "**Kicking self out of room.");
				}

				int kick_fd = get_fd_using_username_UserList(all_users_list, username_to_kick);
				if (kick_fd >= 0)
				{
					// remove and ECHO server <username> has been kicked out.
					remove_user_from_chatrooms(kick_fd, true);

					// Add this user to the waiting room

					// If the waiting_room doesn't exist, create it.
					if (!waiting_room_flag)
					{
						// Create the waiting room thread
						waiting_room_flag = 1; // set flag

						//create echo thread
						if (pthread_create(&waiting_room_thread_id, NULL, waiting_room_thread, NULL) != 0) {
							printf("Error creating waiting room thread, terminating.\n");
							// print_usage_server();

							exit(EXIT_FAILURE);
						}
						if (verbose > 1) printf("**Created waiting room thread.\n");
					}

					// Insert into front of UserList waiting_room_users_list
					insert_into_UserList(waiting_room_users_list, username_to_kick, NULL, kick_fd, 0);

					/* Set up I/O Multiplexing for connecting_file_descriptor */
					//put descriptor in waiting room set
					FD_SET(kick_fd, &waiting_room_thread_active_set);		/* Add clientfd to waiting room read set */
					
					//set the value of the highest file descriptor in echo
					if(waiting_room_fd_count < kick_fd)
						waiting_room_fd_count = kick_fd;

					// KCIK to caller
					send_KCIK_verb(connfd, username_to_kick);

					// KBYE
					send_KBYE_verb(kick_fd);
				}
				else
				{
					if (verbose > 1) print_error("There was an error kicking user.\n");
				}
			}


		}
/***************************************************************************************** LEAVE */
		else if(strcmp(v->name, LEAVE) == 0)
		{
			char *username = get_username_using_fd_UserList(all_users_list, connfd);

			// If the client is not in a room (they are in waiting room), send ERR 30
			if (user_exists_in_UserList(waiting_room_users_list, username))
			{
				// TODO: But client cannot send LEAVE if they are in waiting room
				// so how will this ever get called?
				send_ERR_verb(connfd, USER_NOT_PRESENT_ERROR_CODE, NULL);
			}
			else
			{
				remove_user_from_chatrooms(connfd, false);

				// Add this user to the waiting room

				// If the waiting_room doesn't exist, create it.
				if (!waiting_room_flag)
				{
					// Create the waiting room thread
					waiting_room_flag = 1; // set flag

					//create echo thread
					if (pthread_create(&waiting_room_thread_id, NULL, waiting_room_thread, NULL) != 0) {
						printf("Error creating waiting room thread, terminating.\n");
						// print_usage_server();

						exit(EXIT_FAILURE);
					}
					if (verbose > 1) printf("**Created waiting room thread.\n");
				}

				// Insert into front of UserList waiting_room_users_list
				insert_into_UserList(waiting_room_users_list, username, NULL, connfd, 0);

				/* Set up I/O Multiplexing for connecting_file_descriptor */
				//put descriptor in waiting room set
				FD_SET(connfd, &waiting_room_thread_active_set);		/* Add clientfd to waiting room read set */
				
				//set the value of the highest file descriptor in echo
				if(waiting_room_fd_count < connfd)
					waiting_room_fd_count = connfd;

				send_EVAEL_verb(connfd);
			}
		}
/***************************************************************************************** JOIN */
		else if(strcmp(v->name, JOIN) == 0)
		{
			if (verbose > 1) printf("**About to join a room\n");

			int id;

			// As long as the first character is a number, we will check if this is in the list.
			if(v->args[0][0] >= '0' && v->args[0][0] <= '9') 
				id = atoi(v->args[0]);
			else
				id = -1;

			//TODO : make sure v->args[0] is a number?

			// Lock here
			P(&mutex);

			char *username = NULL;

			if (total_num_rooms <= 0 || !id_exists_in_ChatRoomList(chatroom_list, id))
			{
				// If room does not exist
				// ERR 20
				send_ERR_verb(connfd, ROOM_DOES_NOT_EXIST_ERROR_CODE, NULL);
			}
			else if (!(username = get_username_using_fd_UserList(waiting_room_users_list, connfd)))
			{
				// If client is already inside room (not in waiting room)
				// ERR 60	
				send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
			}
			else if (is_room_private(id))
			{
				// If room is private (password protected),
				// ERR 00 TODO: which error?
				send_ERR_verb(connfd, SORRY_ERROR_CODE, "This room is private! Use /joinp");
			}
			else
			{
				// Join the room
				join_room(id, connfd);

				// Remove user from list of users in waiting room.
				remove_from_waiting_room(connfd);

				/* Tell client that they joined the room */
				send_PNIOJ_verb(connfd, v->args[0]);

				// Inform all users in the joined room except this user that a new user has joined.
				char *user = "server";
				char *hasjoined = " has joined the room.";
				char buff[strlen(username) + strlen(hasjoined) + 1];
				sprintf(buff, "%s%s", username, hasjoined);

				// Get room using id
				struct ChatRoom *room = get_id_ChatRoomList(chatroom_list, id);

				struct User *u = room->users->head;
				while (u)
				{
					if (!(u->connfd == connfd))
						send_ECHO_verb(u->connfd, user, buff);
					u = u->next_link;
				}

				// TODO: We must somehow Select in the chatroom thread so that messages from
				// this fd can go through without having to wait for another client to talk first.
			}

			// Unlock here
			V(&mutex);
		}
/***************************************************************************************** JOINP */
		else if(strcmp(v->name, JOINP) == 0)
		{
			if (verbose > 1) printf("**About to join a private room\n");
			//TODO
			// send_PNIOJ_verb(asdf);

			int id;
			char *pass_word;

			// As long as the first character is a number, we will check if this is in the list.
			if(v->args[0][0] >= '0' && v->args[0][0] <= '9') 
				id = atoi(v->args[0]);
			else
				id = -1;

			//TODO : make sure v->args[0] is a number?

			pass_word = v->args[1];

			// printf("received ID and password of %i, %s\n", id, pass_word);

			// Lock here
			P(&mutex);

			char *username = NULL;

			if (total_num_rooms <= 0 || !id_exists_in_ChatRoomList(chatroom_list, id))
			{
				// If room does not exist
				// ERR 20
				send_ERR_verb(connfd, ROOM_DOES_NOT_EXIST_ERROR_CODE, NULL);
			}
			else if (!(username = get_username_using_fd_UserList(waiting_room_users_list, connfd)))
			{
				// If client is already inside room (not in waiting room)
				// ERR 60	
				send_ERR_verb(connfd, INVALID_OPERATION_ERROR_CODE, NULL);
			}
			else
			{
				if(join_private_room(id, connfd, pass_word) == -1)
				{
					//send ERR 61 <msg>
					send_ERR_verb(connfd, INVALID_PASSWORD_ERROR_CODE, "Wrong private room password");
				
					goto unlock;
				}

				// Remove user from list of users in waiting room.
				remove_from_waiting_room(connfd);

				/* Tell client that they joined the room */
				send_NIOJ_verb(connfd, v->args[0]);

				// Inform all users in the joined room except this user that a new user has joined.
				char *user = "server";
				char *hasjoined = " has joined the room.";
				char buff[strlen(username) + strlen(hasjoined) + 1];
				sprintf(buff, "%s%s", username, hasjoined);

				// Get room using id
				struct ChatRoom *room = get_id_ChatRoomList(chatroom_list, id);

				struct User *u = room->users->head;
				while (u)
				{
					if (!(u->connfd == connfd))
						send_ECHO_verb(u->connfd, user, buff);
					u = u->next_link;
				}

				// TODO: We must somehow Select in the chatroom thread so that messages from
				// this fd can go through without having to wait for another client to talk first.
			}

			// Unlock here
			unlock:
			V(&mutex);
		}
/***************************************************************************************** IAMNEW */
		else if(strcmp(v->name, IAMNEW) == 0)
		{
			if (verbose > 1) printf("**About to handle IAMNEW.\n");

			char *user_name = v->args[0];

			//check if the username is already taken
			if(check_if_username_exists_in_file(user_name) == USER_NAME_TAKEN)
			{
				if(verbose > 1) printf("**Username is already taken!\n");

				//send err 01 sorry <name>
				send_ERR_verb(connfd, USER_NAME_EXISTS_ERROR_CODE, user_name);
				//send BYE
				send_BYE_verb(connfd);
			}
			else
			{
				// Insert into front of UserList all_users_list, under the assumption we will remove user if pw authentication fails
				insert_into_UserList(all_users_list, user_name, NULL, connfd, 0);

				// Insert into front of UserList waiting_room_users_list, under the assumption we will remove user if pw authentication fails
				insert_into_UserList(waiting_room_users_list, user_name, NULL, connfd, 0);

				send_HINEW_verb(connfd, user_name);			
			}
		}
/***************************************************************************************** NEWPASS */
		else if(strcmp(v->name, NEWPASS) == 0)
		{
			if (verbose > 1) printf("**About to handle NEWPASS.\n");

			//TODO - check t see if passwrod is valid
			if(verbose > 1) printf("**Password given was %s\n", v->args[0]);
			
			//if the password is not valid, send ERR 01 MSG and remove the user from the user list
			if(!validatePassword(v->args[0]))
			{
				//from all users
				remove_by_fd_UserList(all_users_list, connfd);

				//from waiting room users
				remove_by_fd_UserList(waiting_room_users_list, connfd);

				//send error verb
				send_ERR_verb(connfd, INVALID_PASSWORD_ERROR_CODE, NULL);
				send_BYE_verb(connfd);
			}
			//else continue with login
			else 
			{
				//get the username of the user we have just authenticated
				char *username = get_username_using_fd_UserList(all_users_list, connfd);

				//hash and salt PW
				char *salt = make_salt(v->args[0]);

				char *hashedPW = hash(v->args[0], salt);
				if(verbose > 1) printf("hashed PW\n");

				//update his password to provided password
				if(set_password_of_user_by_fd(all_users_list, connfd, hashedPW) == -1)
				{
					printf("Could not set the current password of user in all users list, terminating.\n");

					exit(1);
				}

				if(set_password_of_user_by_fd(waiting_room_users_list, connfd, hashedPW) == -1)
				{
					printf("Could not set the current password of user in waiting room users list, terminating.\n");

					exit(1);
				}
				
				//write new user information to file
				if(write_user_info_to_file(username, hashedPW, salt) == -1)
				{
					printf("could not enter new user credentials in database. Terminating.\n");

					exit(1);
				}

				//complete protocol
				send_HI_verb(connfd, username);
				complete_login(connfd, username);
			}
		}
/***************************************************************************************** PASS */
		else if(strcmp(v->name, PASS) == 0)
		{
			if (verbose > 1) printf("**About to handle PASS.\n");

			if(verbose > 1) printf("password given was %s\n", v->args[0]);

			//find the user in the all users list and get his username by connfd
			char *username = get_username_using_fd_UserList(all_users_list, connfd);
			//find the salt
			char *salt = get_salt_for_user(username);

			if(verbose > 1) printf("**Got salt\n");

			//hashPW
			char *hashedPW = hash(v->args[0], salt);
			if(verbose > 1) printf("**hashed PW\n");

			//verify that pw matchs
			if(check_if_uname_pw_match(username, hashedPW) != PW_MATCH)
			{
				if(verbose > 1) printf("**Provided password was not valid.\n");

				//password was not valid, so remove the user from logged in users
				//from all users
				remove_by_fd_UserList(all_users_list, connfd);

				//from waiting room users
				remove_by_fd_UserList(waiting_room_users_list, connfd);

				send_ERR_verb(connfd, INVALID_PASSWORD_ERROR_CODE, NULL);
				send_BYE_verb(connfd);
			}
			else
			{
				send_HI_verb(connfd, username);

				//TODO: handle storage of user name
				complete_login(connfd, username);
			}
		}
		destroy_Verb(v); // must destroy every time
	}

	return ret_val;
}

int validatePassword(char *password)
{
	// *     Has at least 1 symbol character
	
	// *     5 characters long
	int pwLen = strlen(password);
	if(pwLen < 5)
		return 0;

	char *dummy = password;
	int i = 0 , hasUppercase = 0, hasSymbolChar = 0, hasNumberChar=  0;
	while(i < pwLen)
	{
		// *     Has at least 1 symbol character
		if(!isalnum(*dummy))
			hasSymbolChar = 1;

		// *     Has at least 1 number character
		if(isdigit(*dummy))
			hasNumberChar = 1;

		// *     Has at least 1 uppercase letter
		if(isupper(*dummy))
			hasUppercase = 1;

		dummy++;
		i++;
	}

	return hasSymbolChar && hasNumberChar && hasUppercase;
}


void complete_login(int connfd, char *username)
{
	if (!waiting_room_flag)
	{
		// Create the waiting room thread
		waiting_room_flag = 1; // set flag

		//create echo thread
		if (pthread_create(&waiting_room_thread_id, NULL, waiting_room_thread, NULL) != 0) {
			printf("Error creating waiting room thread, terminating.\n");
			// print_usage_server();

			exit(EXIT_FAILURE);
		}
		if (verbose > 1) printf("**Created waiting room thread.\n");
	}
	else
	{
		// send "server > user has connected."
		char *user = "server";
		char *has_connected = "has connected.";
		char buff[strlen(has_connected) + strlen(username) + 2];
		sprintf(buff, "%s %s", username, has_connected);
				
		if (verbose_echo)
			print_verbose_echo(user, buff);

		// TO ALL USERS in waiting room.
		struct User *u = waiting_room_users_list->head;
		while (u)
		{
			if (u->connfd != connfd)
				send_ECHO_verb(u->connfd, user, buff);
			u = u->next_link;
		}
			
	}

	if (verbose > 1) printf("**Adding user %s to userlist.\n", username);
	// Connect the user and add user to list of connected users.

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PART 1
			/* Set up I/O Multiplexing for connecting_file_descriptor */
			//put descriptor in echo set
			// FD_SET(connfd, &echo_thread_active_set);		/* Add clientfd to read set */
			
			// //set the value of the highest file descriptor in echo
			// if(echo_fd_count < connfd)
			// 	echo_fd_count = connfd;
/////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Set up I/O Multiplexing for connecting_file_descriptor */
	//put descriptor in waiting room set
	FD_SET(connfd, &waiting_room_thread_active_set);		/* Add clientfd to waiting room read set */
			
	//set the value of the highest file descriptor in echo
	if(waiting_room_fd_count < connfd)
		waiting_room_fd_count = connfd;

	// Send the user that just connected the MOTD.
	send_message_of_the_day(connfd);
}

/**
 * chatrooms is plural because in the case that we ever have a client in more than one chatroom,
 * this function will cause them to leave all of them.
 */
void remove_user_from_chatrooms(int connfd, bool kicked)
{
	char *name= get_username_using_fd_UserList(all_users_list, connfd);
	struct ChatRoom *chatroom = chatroom_list->head;
	while(chatroom)
	{
		// check if user's name is in a chatroom
		char *user = get_username_using_fd_UserList(chatroom->users, connfd);
		if (!user)
		{
			chatroom = chatroom->next_link;
			continue;
		}

		if (strcmp(user, name) == 0)
		{
			// Remove this user from the chatroom! (remove from fd_set and UserList)
			if (FD_ISSET(connfd, &(chatroom->connected_users_set)))
				FD_CLR(connfd, &(chatroom->connected_users_set));

			remove_by_fd_UserList(chatroom->users, connfd);

			if (chatroom->users->size > 0)
			{
				// if there are still users in the room
				
				// Check if this user is/was the owner.
				// If yes, set a new owner if there is one.
				int owner_fd = chatroom->owner->connfd;
				bool new_owner = false;
				if (owner_fd == connfd)
				{
					chatroom->owner = chatroom->users->head; // Set new owner!

					new_owner = true;
					if (verbose > 1) fprintf(stdout, "**New owner of room: %s.\n", chatroom->owner->user_name);
				}

				// inform all users that this user is leaving
				struct User *u = chatroom->users->head;
				while (u)
				{
					char *user = "server";
					char *has_left = NULL;

					if (kicked)
						has_left = " has been kicked out.";
					else
					 	has_left = " has left the room.";
					 
					char buf[strlen(name) + strlen(has_left) + 1];
					sprintf(buf, "%s%s", name, has_left);

					if (verbose_echo)
						print_verbose_echo(user, buf);

					send_ECHO_verb(u->connfd, user, buf);

					// Inform users that there is a new owner.
					if (new_owner)
					{
						char *promoted = " has been promoted to owner.";
						char buf2[strlen(chatroom->owner->user_name) + strlen(promoted) + 1];
						sprintf(buf2, "%s%s", chatroom->owner->user_name, promoted);

					if (verbose_echo)
						print_verbose_echo(user, buf2);

						send_ECHO_verb(u->connfd, user, buf2);
					}

					u = u->next_link;
				}
			}
		}
		chatroom = chatroom->next_link;
	}
}

bool is_allowed_in_waiting_room(char *verb_name)
{
	/**
	 * Allowed verbs:
	 * 		CREATER			create room
	 * 		CREATEP			create private room
	 * 		LISTR			list rooms
	 * 		JOIN			join room
	 * 		JOINP			join private room
	 * 		BYE				log out
	 */

	if(strcmp(verb_name, CREATER) == 0 ||
	   strcmp(verb_name, CREATEP) == 0 ||
	   strcmp(verb_name, LISTR) == 0 ||
	   strcmp(verb_name, JOIN) == 0 ||
	   strcmp(verb_name, JOINP) == 0 ||
	   strcmp(verb_name, BYE) == 0	||
	   strcmp(verb_name, LEAVE) == 0
	   )
		return true;
	else 
		return false;
}

void send_message_of_the_day(int connfd)
{
	char *user = "server";

	if (verbose_echo)
		print_verbose_echo(user, serverInfo->motd);

	send_ECHO_verb(connfd, user, serverInfo->motd); // user is "server"
}

void print_verbose_echo(char *username, char *msg)
{
	/* Output what is being echoed */

	Verb *v = make_ECHO_verb(username, msg);
	char *output = encode_Verb(v);

	Rio_writen(1, (char*)COLOR_DEFAULT, strlen(COLOR_DEFAULT));
	Rio_writen(1, (char*)v->args[0], strlen(v->args[0]));
	Rio_writen(1, (char*)" > ", strlen(" > "));

	write_array(1, v->args + 1, v->argc - 1);
	Rio_writen(1, (char*)"\n", strlen("\n"));

	destroy_Verb(v);
	Free(output);
}

void create_new_private_room(char *room_name, char *room_password, int ownerfd)
{
	// create a room
	// Each room will be assigned a numerical ID and Name
	// Insert into front of list.
	struct ChatRoom *new_private_room = new_private_ChatRoom(room_name, next_chat_room_id++, room_password);

		/* IMPORTANT */
	/* If the user is already inside a room, the user must leave the room */

	/* Assuming that the user can be in only one room */
	remove_user_from_chatrooms(ownerfd, false);
	// end important

	// Add to list of users
	char *name= get_username_using_fd_UserList(all_users_list, ownerfd);
	insert_into_UserList(new_private_room->users, name, NULL, ownerfd, 0);

	// Add owner to fdset
	FD_SET(ownerfd, &(new_private_room->connected_users_set));

	// Set fd_count to highest fd, which is currently just the one.
	new_private_room->fd_count = ownerfd;

	// Set user as owner
	new_private_room->owner = get_User_using_fd_UserList(all_users_list, ownerfd);

	// Insert new ChatRoom into ChatRoomList
	insert_into_position_ChatRoomList(chatroom_list, new_private_room, 0);
	total_num_rooms++;

	// spawn a thread for this chatroom
	// create a thread to handle the log in process
	if(pthread_create(&(new_private_room->thread_id), NULL, chatroom_thread, new_private_room) != 0)
	{
		print_error("Error creating chatroom thread, terminating.\n");
		// print_usage_server();
		EXIT_FAILURE;
	}
	if (verbose > 1) printf("**Created chatroom %s thread.\n", room_name);
}

void create_new_room(char *room_name, int ownerfd)
{
	// create a room
	// Each room will be assigned a numerical ID and Name
	// Insert into front of list.
	struct ChatRoom *new_room = new_ChatRoom(room_name, next_chat_room_id++);

	/* IMPORTANT */
	/* If the user is already inside a room, the user must leave the room */

	/* Assuming that the user can be in only one room */
	remove_user_from_chatrooms(ownerfd, false);
	// end important

	// Add to list of users
	char *name= get_username_using_fd_UserList(all_users_list, ownerfd);
	insert_into_UserList(new_room->users, name, NULL, ownerfd, 0);

	// Add owner to fdset
	FD_SET(ownerfd, &(new_room->connected_users_set));

	// Set fd_count to highest fd, which is currently just the one.
	new_room->fd_count = ownerfd;

	// Set user as owner
	new_room->owner = get_User_using_fd_UserList(all_users_list, ownerfd);

	// Insert new ChatRoom into ChatRoomList
	insert_into_position_ChatRoomList(chatroom_list, new_room, 0);
	total_num_rooms++;

	// spawn a thread for this chatroom
	// create a thread to handle the log in process
	if(pthread_create(&(new_room->thread_id), NULL, chatroom_thread, new_room) != 0)
	{
		print_error("Error creating chatroom thread, terminating.\n");
		// print_usage_server();
		EXIT_FAILURE;
	}
	if (verbose > 1) printf("**Created chatroom %s thread.\n", room_name);
}

//returns -1 on error, else success
int join_private_room(int room_id, int fd, char *pw)
{
	// join a room
	
	// Unlike create_new_room, this function will never be called
	// if the User is already in a room.
	// remove_user_from_chatrooms(fd, false);

	// Get room using id
	struct ChatRoom *room = get_id_ChatRoomList(chatroom_list, room_id);
	if (room)
	{
		//if the passwords don't match, or you are joining a non password protected room, give an error and exit
		if(password_matches(room, pw) != PASSWORD_VERIFIED)
		{
			if (verbose > 1) printf("**The password %s either did not match the room's password, or the room was not private.\n", pw);

			return -1;
		}
		
		if (verbose > 1) printf("**Password matched\n");
		
		// Add to list of users
		char *name= get_username_using_fd_UserList(all_users_list, fd);
		insert_into_UserList(room->users, name, NULL, fd, 0);

		// Add to fdset
		FD_SET(fd, &(room->connected_users_set));

		// Set fd_count to highest fd, which is currently just the one.
		if (room->fd_count < fd)
			room->fd_count = fd;

		// TODO: should there be a queue to see who's next in line to be owner of this group?

		if (verbose > 1) printf("**Joined room %d.\n", room_id);
	}
	else
	{
		if (verbose > 1) printf("**There was an error joining room %d.\n", room_id);

		return -1;
	}

	return 1;
}

bool is_room_private(int room_id)
{
	// If we cannot find a room, return it as private.

	struct ChatRoom *room = get_id_ChatRoomList(chatroom_list, room_id);
	if (room && room->is_public)
		return false;

	return true;
}

void join_room(int room_id, int fd)
{
	// join a room
	
	// Unlike create_new_room, this function will never be called
	// if the User is already in a room.
	// remove_user_from_chatrooms(fd, false);

	// Get room using id
	struct ChatRoom *room = get_id_ChatRoomList(chatroom_list, room_id);
	if (room)
	{
		// Add to list of users
		char *name= get_username_using_fd_UserList(all_users_list, fd);
		insert_into_UserList(room->users, name, NULL, fd, 0);

		// Add to fdset
		FD_SET(fd, &(room->connected_users_set));

		// Set fd_count to highest fd, which is currently just the one.
		if (room->fd_count < fd)
			room->fd_count = fd;

		// TODO: should there be a queue to see who's next in line to be owner of this group?

		if (verbose > 1) printf("**Joined room %d.\n", room_id);
	}
	else
	{
		if (verbose > 1) printf("**There was an error joining room %d.\n", room_id);
	}
}

/*int create_hash_from_salt(char *pw, char *salt, char *storedHashedPW)
{
	//append salt to pw
	char *saltedPW =  strcat(pw, salt);

	char *hashedPW = (char *)malloc(sizeof(char) * strlen(storedHashedPW) + 1);
	SHA_CTX context;
	if(!SHA1_Init(&context))
	{
		if(verbose > 1) printf("**Error initing SHA1 function. returning NULL");

		return -1;
	}

	if(!SHA1_Update(&context, (unsigned char*)saltedPW, strlen(saltedPW)))
	{
		if(verbose > 1) printf("**Error updating SHA1 function. returning NULL");

		return -1;
	}

	if(!SHA1_Final((unsigned char *)hashedPW, &context))
	{
		if(verbose > 1) printf("**Error finalizing SHA1 function. returning NULL");

		return -1;
	}

	return strcmp(storedHashedPW, hashedPW) == 0;
}*/

char *make_salt(char *pw)
{
	if(verbose > 1) printf("**about to make salt\n");

	//append salt to PW
	unsigned char * salt = (unsigned char *) malloc(sizeof(char) * (MAX_LINE_SIZE - sizeof(pw)));

	int i;
	int random_num = rand() % 50;
	for(i = 0; i < 1000; i++)
	{
		if(i == random_num)
		{
			if(verbose > 1) printf("i is equal to rand, it is %i\n", i);
			RAND_bytes(salt, strlen(pw) * random_num);
		}
	}

	return (char *)salt;
}

char *hash(char *pw, char *salt)
{
	if(verbose > 1)	printf("**In hash");
	

	char *saltedPW = (char *)malloc(sizeof(char) * MAX_LINE_SIZE);
	saltedPW = strcat(saltedPW , pw);
	saltedPW = strcat(saltedPW, (char *)salt);

	//make hash
	char *hashedPW = (char *)malloc(sizeof(char) * strlen(pw) + 1);


	SHA_CTX context;
	if(!SHA1_Init(&context))
	{
		if(verbose > 1) printf("**Error initing SHA1 function. returning NULL");

		return NULL;
	}

	if(!SHA1_Update(&context, (unsigned char*)saltedPW, strlen(saltedPW)))
	{
		if(verbose > 1) printf("**Error updating SHA1 function. returning NULL");

		return NULL;
	}

	if(!SHA1_Final((unsigned char *)hashedPW, &context))
	{
		if(verbose > 1) printf("**Error finalizing SHA1 function. returning NULL");

		return NULL;
	}

	char * stdoutHolder = (char*) malloc(sizeof(char) * MAX_LINE_SIZE);

	if(verbose > 1) 
	{
		stdoutHolder = strcat(stdoutHolder, hashedPW);
		stdoutHolder = strcat(stdoutHolder, "\0");
	
		printf("***Hashed!!!");
	}

	return hashedPW;
}


void remove_from_waiting_room(int connfd)
{
	// If this connfd is in the waiting room, remove from list of users in waiting room.
	remove_by_fd_UserList(waiting_room_users_list, connfd);

	// Remove from active waiting room set
	if (FD_ISSET(connfd, &waiting_room_thread_active_set))
		FD_CLR(connfd, &waiting_room_thread_active_set);

	// if there are no more users, kill the echo thread and set its flag to 1
	// also kill waiting room flag
	if(waiting_room_users_list->size == 0)
	{
		waiting_room_flag = 0;
	}
}