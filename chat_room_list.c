//This data structure stores all chat rooms currently active on server
#include "chat_room_list.h"
#include "user_list.h"

struct ChatRoom *new_ChatRoom(char* name, int chat_room_id)
{
	struct ChatRoom *chatroom = (struct ChatRoom *) malloc(sizeof(struct ChatRoom));
	chatroom->room_name = (char*) malloc(strlen(name) + 1);
	strcpy(chatroom->room_name, name);

	chatroom->room_id = chat_room_id;

	chatroom->previous_link = NULL;
	chatroom->next_link = NULL;

	chatroom->users = new_UserList();

	FD_ZERO(&chatroom->connected_users_set);			/* Clear read set */
	
	chatroom->is_public = 1; // by default rooms are public
	chatroom->password = NULL;

	return chatroom;
}

struct ChatRoom *new_private_ChatRoom(char* name, int chat_room_id, char *password)
{
	struct ChatRoom *chatroom = new_ChatRoom(name, chat_room_id);
	chatroom->is_public = 0;

	chatroom->password = (char*) malloc(strlen(password) + 1);
	strcpy(chatroom->password, password);

	return chatroom;
}

//initialize list to NULL
struct ChatRoomList* new_ChatRoomList()
{
	struct ChatRoomList* chat_room_list = (struct ChatRoomList *)malloc(sizeof(struct ChatRoomList));
	
	chat_room_list->head = NULL;
	chat_room_list->tail = NULL;
	chat_room_list->pointer = NULL;
	chat_room_list->size = 0;

	return chat_room_list;
}

// given a room id, remove it from the list if possible and return the name, or NULL if name not found
char* remove_by_id_ChatRoomList(struct ChatRoomList *chat_room_list, int chat_room_id)
{
	if(chat_room_list->size <= 0)
		return NULL;
	
	int position = 0;
	struct ChatRoom *node = chat_room_list->head;
	while(node && node->room_id != chat_room_id && position < chat_room_list->size)
	{
		node = node->next_link;
		position++;
	}

	//could not find user in list
	if(position == chat_room_list->size)
		return NULL;

	//remove position from list
	return remove_by_position_ChatRoomList(chat_room_list, position);
}

// given an index, remove the right item from the list (0 indexed)
char* remove_by_position_ChatRoomList(struct ChatRoomList *chat_room_list, int position)
{
	if(position > chat_room_list->size)
	{
		//STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}

	char* ret_val = NULL;

	//if we're removing the head, make the next item the head
	if(position == 0)
	{
		struct ChatRoom *old_head = chat_room_list->head;
		struct ChatRoom *new_head = old_head->next_link;

		if(new_head != NULL)
		{
			chat_room_list->head = new_head;
			new_head->previous_link = NULL;
		}
		//the list is empty now
		else
		{
			chat_room_list->head = NULL;
			chat_room_list->tail = NULL;
			chat_room_list->pointer = NULL;
			chat_room_list->size = 0;

			return old_head->room_name;
		}

		free(old_head);
	}
	//if we're removing the tail, do this
	else if(position == chat_room_list->size - 1)
	{
		struct ChatRoom *old_tail = chat_room_list->tail;
		struct ChatRoom *new_tail = old_tail->previous_link;

		//note, the list cant really be empty after deletion since that condition is checked in the previous if block
		chat_room_list->tail = new_tail;
		new_tail->next_link = NULL;

		ret_val = old_tail->room_name;
		free(old_tail);
	}
	//deletion is occuring inside the linked list but it is not the head or tail
	else
	{
		struct ChatRoom *previous_link = chat_room_list->head, *deletion_link, *next_link;
		int i = 0;

		while(i < position - 1)
			previous_link = previous_link->next_link;
		
		deletion_link = previous_link->next_link;
		next_link = previous_link->next_link->next_link;

		//set the links of previous and next
		previous_link->next_link = next_link;
		next_link->previous_link = previous_link;

		//free the link to be deleted
		ret_val = deletion_link->room_name;
		free(deletion_link);
	}

	chat_room_list->size--;
	return ret_val;
}

//insert the given item into the list (0 indexed)
char* insert_into_position_ChatRoomList(struct ChatRoomList *chat_room_list, struct ChatRoom *node, int position)
{
	// printf("inserting into list\n");

	if(position > chat_room_list->size)
	{
		//STDOUT("ERROR INSERTING TO LIST, position is greater than size\n");
		return NULL;
	}
	
	//if there are no items in the list (head is NULL) new node is head and tail
	if(chat_room_list->head == NULL)
	{
		node->next_link = NULL;
		node->previous_link = NULL;

		chat_room_list->head = node;
		chat_room_list->tail = node;

		chat_room_list->pointer = node;
	}

	//if the position is the last in the list
	else if(position == chat_room_list->size)
	{
		struct ChatRoom *tail = chat_room_list->tail;
		tail->next_link = node;
		node->previous_link = tail;

		chat_room_list->tail = node;
	}
	//if you are inserting into the head of the list
	else if(position == 0)
	{
		struct ChatRoom *head = chat_room_list->head;
		
		head->previous_link = node;
		node->next_link = head;

		chat_room_list->head = node;
	}
	//else insert it into the middle of the list
	else
	{
		struct ChatRoom *previous_link = chat_room_list->head;
		int i = 0;

		while(i < position - 1)
			previous_link = previous_link->next_link;
		
		struct ChatRoom *next_link = previous_link->next_link;

		previous_link->next_link = node;
		node->previous_link = previous_link;

		next_link->previous_link = node;
		node->next_link = next_link;
	}

	chat_room_list->size++;

	return "EXIT_SUCCESS"; // TODO ?
}

//get item in position of list (list is zero indexed)
struct ChatRoom* get_position_ChatRoomList(struct ChatRoomList* chat_room_list, int position)
{
	if(chat_room_list->size == 0)
	{
		//STDOUT("ERROR: Linked list does not have any members\n");
		return NULL;
	}
	else if(position >= chat_room_list->size)
	{
		//STDOUT("ERROR: Linked list index out of bounds\n");
		return NULL;
	}

	//if the position is zero, return data at the head
	if(position == 0)
		return chat_room_list->head;
	//if the position is the tial, return the data at the tail
	else if(position == chat_room_list->size - 1)
		return chat_room_list->tail;
	//otherwise go to the position in the list and return the item data
	else
	{
		int i = 0;
		struct ChatRoom *node = chat_room_list->head;
		while(i < position)
		{
			node = node->next_link;
			i++;
		}
		return node;
	}
}

struct ChatRoom* get_id_ChatRoomList(struct ChatRoomList *list, int id)
{
	//if the list is empty return NULL
	if(list->size == 0)
		return NULL;

	//else loop through and see if we have a match on name
	struct ChatRoom *node = list->head;
	while(node)
	{
		//check name to see if the user exists
		if(node->room_id == id)
			return node;

		node = node->next_link;
	}
	return NULL;	
}

int password_matches(struct ChatRoom *node, char *roompw)
{
	// printf("%s %s\n", node->password, roompw);

	if(node->is_public != 0)
		return NON_PRIVATE_ROOM;

	if(strcmp(node->password, roompw) != 0)
		return PASSWORD_MISMATCH;

	return PASSWORD_VERIFIED;	
}

bool name_exists_in_ChatRoomList( struct ChatRoomList* chat_room_list, char *name)
{
	//if the list is empty return false
	if(chat_room_list->size == 0)
		return false;

	//else loop through and see if we have a match on name
	struct ChatRoom *node = chat_room_list->head;
	while(node)
	{
		//check name to see if the user exists
		if(strcmp(node->room_name, name) == 0)
			return true;

		node = node->next_link;
	}
	return false;
}

bool id_exists_in_ChatRoomList(struct ChatRoomList* chat_room_list, int id)
{
	//if the list is empty return false
	if(chat_room_list->size == 0)
		return false;

	//else loop through and see if we have a match on name
	struct ChatRoom *node = chat_room_list->head;
	while(node)
	{
		//check name to see if the user exists
		if(node->room_id == id)
			return true;

		node = node->next_link;
	}
	return false;
}