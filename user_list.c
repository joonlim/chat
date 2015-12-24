//This data structure stores all users currently active on server
#include "user_list.h"

/* Constructor for User */
struct User* new_User(char *user_name, char *pass_word, int fd)
{
	struct User *user = (struct User*)malloc(sizeof(struct User));
		
	user->user_name = (char*) malloc(strlen(user_name));
	strcpy(user->user_name, user_name);

	//provided password may be NULL, it is ok. But check to make sure 
	//it is not null before using strcpy as it will segfault.
	if(pass_word != NULL)
	{
		user->pass_word = (char*) malloc(sizeof(char *) * strlen(pass_word));

		strcpy(user->pass_word, pass_word); 
	}
	else
	{
		user->pass_word = (char *) malloc(sizeof(char *) * 1024);
		user->pass_word = NULL;
	}
	
	user->connfd = fd;

	user->previous_link = NULL;
	user->next_link = NULL;

	return user;
}

//initialize list to NULL
struct UserList* new_UserList()
{
	struct UserList* user_list = (struct UserList *)malloc(sizeof(struct UserList));
	
	user_list->head = NULL;
	user_list->tail = NULL;
	user_list->pointer = NULL;
	user_list->size = 0;

	return user_list;
}

//given a name, remove it from the list if possible and return the name, or NULL if name not found
char* remove_by_fd_UserList(struct UserList *user_list, int fd)
{
	if(user_list->size <= 0)
		return NULL;
	
	int position = 0;
	struct User *user = user_list->head;
	while(user && user->connfd != fd && position < user_list->size)
	{
		user = user->next_link;
		position++;
	}

	//could not find user in list
	if(position == user_list->size)
		return NULL;

	//remove position from list
	return remove_by_position_UserList(user_list, position);
}

//given an index, remove the right item from the list (0 indexed)
char* remove_by_position_UserList( struct UserList *user_list, int position)
{
	if(position > user_list->size)
	{
		//STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}

	char* ret_val = NULL;

	//if we're removing the head, make the next item the head
	if(position == 0)
	{
		struct User *old_head = user_list->head;
		struct User *new_head = old_head->next_link;

		if(new_head != NULL)
		{
			user_list->head = new_head;
			new_head->previous_link = NULL;
		}
		//the list is empty now
		else
		{
			user_list->head = NULL;
			user_list->tail = NULL;
			user_list->pointer = NULL;
			user_list->size = 0;

			return old_head->user_name;
		}

		free(old_head);
	}
	//if we're removing the tail, do this
	else if(position == user_list->size - 1)
	{
		struct User *old_tail = user_list->tail;
		struct User *new_tail = old_tail->previous_link;

		//note, the list cant really be empty after deletion since that condition is checked in the previous if block
		user_list->tail = new_tail;
		new_tail->next_link = NULL;

		ret_val = old_tail->user_name;
		free(old_tail);
	}
	//deletion is occuring inside the linked list but it is not the head or tail
	else
	{
		struct User *previous_link = user_list->head, *deletion_link, *next_link;
		int i = 0;

		while(i < position - 1)
			previous_link = previous_link->next_link;
		
		deletion_link = previous_link->next_link;
		next_link = previous_link->next_link->next_link;

		//set the links of previous and next
		previous_link->next_link = next_link;
		next_link->previous_link = previous_link;

		//free the link to be deleted
		ret_val = deletion_link->user_name;
		free(deletion_link);
	}

	user_list->size--;
	return ret_val;
}

//insert the given item into the list (0 indexed)
char* insert_into_UserList(struct UserList *user_list, char *user_name, char *pass_word, int fd, int position)
{
	if(position > user_list->size)
	{
		//STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}
	
	//make the node
	struct User *node = new_User(user_name, pass_word, fd);
	//node->data = data;

	//if there are no items in the list (head is NULL) new node is head and tail
	if(user_list->head == NULL)
	{
		node->next_link = NULL;
		node->previous_link = NULL;

		user_list->head = node;
		user_list->tail = node;

		user_list->pointer = node;
	}

	//if the position is the last in the list
	else if(position == user_list->size)
	{
		struct User *tail = user_list->tail;
		tail->next_link = node;
		node->previous_link = tail;

		user_list->tail = node;
	}
	//if you are inserting into the head of the list
	else if(position == 0)
	{
		struct User *head = user_list->head;
		
		head->previous_link = node;
		node->next_link = head;

		user_list->head = node;
	}
	//else insert it into the middle of the list
	else
	{
		struct User *previous_link = user_list->head;
		int i = 0;

		while(i < position - 1)
			previous_link = previous_link->next_link;
		
		struct User *next_link = previous_link->next_link;

		previous_link->next_link = node;
		node->previous_link = previous_link;

		next_link->previous_link = node;
		node->next_link = next_link;
	}

	user_list->size++;

	return "EXIT_SUCCESS";
}

//get item in position of list (list is zero indexed)
char* get_position_UserList(struct UserList* user_list, int position)
{
	if(user_list->size == 0)
	{
		//STDOUT("ERROR: Linked list does not have any members\n");
		return NULL;
	}
	else if(position >= user_list->size)
	{
		//STDOUT("ERROR: Linked list index out of bounds\n");
		return NULL;
	}

	//if the position is zero, return data at the head
	if(position == 0)
		return user_list->head->user_name;
	//if the position is the tial, return the data at the tail
	else if(position == user_list->size - 1)
		return user_list->tail->user_name;
	//otherwise go to the position in the list and return the item data
	else
	{
		int i = 0;
		struct User *node = user_list->head;
		while(i < position)
		{
			node = node->next_link;
			i++;
		}
		return node->user_name;
	}
}

bool user_exists_in_UserList(struct UserList* user_list, char *name)
{
	//if the list is empty return false
	if(user_list->size == 0)
		return false;

	//else loop through and see if we have a match on name
	struct User *node = user_list->head;
	while(node != NULL)
	{
		//check name to see if the user exists
		if(strcmp(node->user_name, name) == 0)
			return true;

		node = node->next_link;
	}

	return false;
}

int set_password_of_user_by_fd(struct UserList *user_list, int fd, char *password)
{
	//if the list is empty return false
	if(user_list->size == 0)
		return -1;

	//else loop through and see if we have a match on name
	struct User *node = user_list->head;
	while(node != NULL)
	{
		//check name to see if the user exists
		if(node->connfd == fd)
			break;

		node = node->next_link;
	}

	//if we found no one, return -1
	if(node == NULL)
		return -1;

	//else we have our user, update his password
	//malloc space for pw if it is nul
	if(node->pass_word == NULL)
		node->pass_word = (char*) malloc(sizeof(char *) * strlen(password));
	
	strcpy(node->pass_word, password);
	
	return 1;
}


char *get_username_using_fd_UserList(struct UserList* list, int fd)
{
	struct User *user = list->head;
	while (user)
	{
		if (user->connfd == fd)
		{
			return user->user_name;
		}

		user = user->next_link;
	}

	return NULL;
}

int get_fd_using_username_UserList(struct UserList* user_list, char *username)
{
	struct User *user = user_list->head;
	while (user)
	{
		if (strcmp(user->user_name, username) == 0)
		{
			return user->connfd;
		}

		user = user->next_link;
	}

	return -1;	
}

struct User *get_User_using_fd_UserList(struct UserList* user_list, int fd)
{
	struct User *user = user_list->head;
	while (user)
	{
		if (user->connfd == fd)
		{
			return user;
		}

		user = user->next_link;
	}

	return NULL;
}