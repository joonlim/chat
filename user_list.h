#ifndef __USER_LIST
#define __USER_LIST

//This data structure stores all users currently active on server
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//remove later - only here for testing
#include <stdio.h>

struct User
{
	char* user_name;
	char* pass_word;
	int connfd;
	struct User *previous_link;
	struct User *next_link;
};

struct UserList
{
	struct User *head;
	struct User *tail;
	struct User *pointer;
	int size;
};

/********************** Functions of UserList ***************************/

/**
 * Constructor for User
 * @param node      [description]
 * @param user_name [description]
 * @param fd        [description]
 */
struct User* new_User(char *user_name, char *pass_word, int fd);

/**
 * Constructor for UserList
 * @return [description]
 */
struct UserList* new_UserList();

/**
 * Check to see if User exists in UserList
 * @param  name      [description]
 * @param  user_list [description]
 * @return           true if user exists, else false
 */
bool user_exists_in_UserList(struct UserList* user_list, char *name);

/**
 * Given a user's information, insert into UserList.
 * @param  user_name [description]
 * @param  fd        [description]
 * @param  position       [description]
 * @param  list      [description]
 * @return           [description]
 */
char* insert_into_UserList(struct UserList *user_list, char *user_name, char *pass_word, int fd, int position);

/**
 * Given the filedescriptor of a user in the user list, update his password to provided value and return 1, or do nothign and return -1
 * on error
 */
int set_password_of_user_by_fd(struct UserList *user_list, int fd, char *password);

/**
 * Given a connected file descriptor, return the username of the user attached to it.
 * @param  user_list [description]
 * @param  fd        [description]
 * @return           [description]
 */
char *get_username_using_fd_UserList(struct UserList* user_list, int fd);

/**
 * Given a username, return the file descriptor of the user attached to it.
 * This returns -1 if the username does not exist in the list.
 * @param  user_list [description]
 * @param  username  [description]
 * @return           [description]
 */
int get_fd_using_username_UserList(struct UserList* user_list, char *username);

/**
 * Given a connected file descriptor, return the User struct object of the user attached to it.
 * @param  user_list [description]
 * @param  fd        [description]
 * @return           [description]
 */
struct User *get_User_using_fd_UserList(struct UserList* user_list, int fd);

//given an index, remove the right item from the list (0 indexed)

/**
 * Remove User at given position from UserList.
 * @param  position  [description]
 * @param  user_list [description]
 * @return           [description]
 */
char* remove_by_position_UserList(struct UserList *user_list, int position);

/**
 * Remove User with given file descriptor from UserList.
 * @param  user_list [description]
 * @param  fd        [description]
 * @return           [description]
 */
char* remove_by_fd_UserList(struct UserList *user_list, int fd);

/**ser
 * Given a position, return the User in that position of the UserList.
 * @param  position  [description]
 * @param  user_list [description]
 * @return           [description]
 */
char* get_position_UserList(struct UserList *user_list, int position);

#endif