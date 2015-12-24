/*
 Stores all information about usernames and passwords in the file  "user_list.txt"
 Each line is of format username\tpassword, where username is a registered username and 
 password is a the password assosciated with that username
 */
#ifndef __FILE_MANIPULATOR
#define __FILE_MANIPULATOR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 FILE *user_list_ptr;

#define GOOD_WRITE_OP 	100
#define BAD_WRITE_OP  	200
#define USER_NAME_TAKEN 300
#define PW_MATCH		401

#define WRITE_LINE_SIZE 1024

#define FILE_PATH "./user_info/user_list.txt"

//storage for current username and password we are writing
char *username_to_write;
char *password_to_write;

/**
 *Given a username and password, write the information to the 
 *file. Note this assumes the username is not present in the file,
 *so all calls to function must be preceded by check_if_username_exists_in_file.
 * 
 * @return. If write operation was successful, returns GOOD_WRITE_OP, else returns 
 * BAD_WRITE_OP
 */
int write_user_info_to_file(const char *username, const char *password, const char *salt);

char *get_salt_for_user(char *username);

/**
 *Given a username Check if the user exists in the storage file.
 * 
 * @return. If write operation was successful, returns GOOD_WRITE_OP, else returns 
 * BAD_WRITE_OP
 */
int check_if_username_exists_in_file(char *username);

/**
 * Given a username and provided password, check if they match
 */
int check_if_uname_pw_match(char *username, char *password);


#endif