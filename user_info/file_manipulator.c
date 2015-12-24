#include "file_manipulator.h"

int open_file_for_reading()
{
	user_list_ptr = fopen(FILE_PATH, "r");

	return GOOD_WRITE_OP;
}

int open_file_for_writing()
{
	user_list_ptr = fopen(FILE_PATH, "a+");

	return GOOD_WRITE_OP;
}

int write_user_info_to_file(const char *username, const char *password, const char *salt)
{
	// printf("called write user info to file.\n");

	open_file_for_writing();

	//write user into file
	fprintf(user_list_ptr, "%s", username); 	//write username
	fprintf(user_list_ptr, "\n");	 			//write seperator
	fprintf(user_list_ptr, "%s", password);		//write password
	fprintf(user_list_ptr, "\n");	 			//write seperator
	fprintf(user_list_ptr, "%s", salt);			//write salt
	fprintf(user_list_ptr, "\n"); 				//write line terminator

	//close file
	fclose(user_list_ptr);

	return GOOD_WRITE_OP;
}

int check_if_uname_pw_match(char *username, char *password)
{
	open_file_for_reading();

	//check for username
	char *read_string = (char *) malloc(sizeof(char) * WRITE_LINE_SIZE);
	char *inner_dummy = (char *) malloc(sizeof(char) * WRITE_LINE_SIZE);
	
	int num_bytes_read = 0;
	size_t buffer_size = WRITE_LINE_SIZE;

	while((num_bytes_read = getline(&read_string, &buffer_size, user_list_ptr)) != -1)
	{
		char *null = read_string;
		while(*null != '\n')
			null++;
		*null = '\0'; 

		//if we have found the user check for password match
		if(strcmp(read_string, username) == 0)
		{	
			//get password and remove newline at end of it
			if(getline(&inner_dummy, &buffer_size, user_list_ptr) == -1) return BAD_WRITE_OP;
	
			char *pw = inner_dummy;
			char *ptr = pw;
			while(*ptr != '\n')
				ptr++;
			*ptr = '\0';

			//check if they are a match
			if(memcmp(pw, password, strlen(pw)) == 0)
				return PW_MATCH;
		}
	}
	
	return BAD_WRITE_OP;
}

char *get_salt_for_user(char *username)
{
	open_file_for_reading();

	//check for username
	char *read_string = (char *) malloc(sizeof(char) * WRITE_LINE_SIZE);
	char *inner_dummy = (char *) malloc(sizeof(char) * WRITE_LINE_SIZE);
	
	int num_bytes_read = 0;
	size_t buffer_size = WRITE_LINE_SIZE;

	while((num_bytes_read = getline(&read_string, &buffer_size, user_list_ptr)) != -1)
	{	
		char *null = read_string;
		while(*null != '\n')
			null++;
		*null = '\0'; 

		if(strcmp(username, read_string) == 0)
		{
			// printf("found user\n");

			//zoom past PW
			if(getline(&inner_dummy, &buffer_size, user_list_ptr) == -1) return NULL;
			//get salt
			if(getline(&inner_dummy, &buffer_size, user_list_ptr) == -1) return NULL;
			
			//remove newline from salt
			char *null = inner_dummy;
			while(*null != '\n')
				null++;
			*null = '\0'; 

			char *salt = inner_dummy;
			fclose(user_list_ptr);

			return salt;
		}
	}

	//close file
	fclose(user_list_ptr);

	return NULL;
}

int check_if_username_exists_in_file(char *username)
{
	open_file_for_reading();

	//check for username
	char *read_string = (char *) malloc(sizeof(char) * WRITE_LINE_SIZE);
	int num_bytes_read = 0;
	size_t buffer_size = WRITE_LINE_SIZE;

	while((num_bytes_read = getline(&read_string, &buffer_size, user_list_ptr)) != -1)
	{
		char *null = read_string;
		while(*null != '\n')
			null++;
		*null = '\0'; 


		if(strcmp(username, read_string) == 0)
		{
			fclose(user_list_ptr);

			return USER_NAME_TAKEN;
		}
	}

	//close file
	fclose(user_list_ptr);

	return GOOD_WRITE_OP;
}