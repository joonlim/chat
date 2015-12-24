#ifndef __CHATROOM_LIST
#define __CHATROOM_LIST

//This data structure stores all chat rooms currently active on server

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//remove later - only here for testing
#include <stdio.h>

//Return values used when logging into private rooms
#define NON_PRIVATE_ROOM  100
#define PASSWORD_MISMATCH 101
#define PASSWORD_VERIFIED 102

struct ChatRoom
{
	char* room_name;
	int room_id;
	struct User *owner;

	fd_set connected_users_set;
	int is_public; // 1 or 0
	char *password;

	struct UserList *users;	// All the users in the list, including the owner.
	int fd_count; // this should be set to the highest fd in connected_users_set
	
	struct ChatRoom *previous_link;
	struct ChatRoom *next_link;

	pthread_t thread_id;
};

struct ChatRoomList
{
	struct ChatRoom *head;
	struct ChatRoom *tail;
	struct ChatRoom *pointer;
	int size;
};

/**
 * Constructor for ChatRoomList
 */
struct ChatRoomList *new_ChatRoomList();

/**
 * Constructor for a public ChatRoom
 * @param  chat_room_name [description]
 * @param  chat_room_id   [description]
 * @return                [description]
 */
struct ChatRoom *new_ChatRoom(char *chat_room_name, int chat_room_id);

/**
 * Constructor for a private ChatRoom
 * @param  name         [description]
 * @param  chat_room_id [description]
 * @param  password     [description]
 * @return              [description]
 */
struct ChatRoom *new_private_ChatRoom(char* name, int chat_room_id, char *password);

/**
 * Return true if a chatroom with a given name exists in a ChatRoomList
 */
bool name_exists_in_ChatRoomList(struct ChatRoomList* chat_room_list, char *name);

/**
 * Return true if a chatroom with a given ID exists in a ChatRoomList
 */
bool id_exists_in_ChatRoomList(struct ChatRoomList* chat_room_list, int id);

/**
 * Insert a new ChatRoom into a ChatRoomList.
 * Return EXIT_SUCESS or NULL on failure.
 * 
 * @param  list           [description]
 * @param  chatroom       [description]
 * @param  position       [description]
 * @return                [description]
 */
char* insert_into_position_ChatRoomList(struct ChatRoomList *list, struct ChatRoom *chatroom, int position);

/**
 * Remove the ChatRoom at the given position of a ChatRoomList.
 * Returns the name of the ChatRoom on success, else NULL.
 * 
 * @param  list     [description]
 * @param  position [description]
 * @return          [description]
 */
char* remove_by_position_ChatRoomList(struct ChatRoomList *list, int position);

/**
 * Given an id, remove the ChatRoom associated with it in the ChatRoomList.
 * Returns the name of the ChatRoom on success, else NULL.
 * 
 * @param  list         [description]
 * @param  chat_room_id [description]
 * @return              [description]
 */
char* remove_by_id_ChatRoomList(struct ChatRoomList *list, int chat_room_id);

/**
 * Given a position, return the ChatRoom in that position of the ChatRoomList.
 * @param  list     [description]
 * @param  position [description]
 * @return          [description]
 */
struct ChatRoom* get_position_ChatRoomList(struct ChatRoomList *list, int position);

/**
 * Given an id, return the ChatRoom with that id in a ChatRoomList
 * @param  list     [description]
 * @param  position [description]
 * @return          [description]
 */
struct ChatRoom* get_id_ChatRoomList(struct ChatRoomList *list, int id);

/**
 * Given a room and a possible password, determine if that given password is a match and 
 * PASSWORD_VERIFIED, PASSWORD_MISMATCH, or NON_PRIVATE ROOM depending on result.
 */
int password_matches(struct ChatRoom *node, char *roompw);



#endif