CC=gcc
CFLAGS= -Werror -Wall -g
CLIENT_BIN=client
SERVER_BIN=server
BINS=client server
CLIENT_MODULES=client.c utils/csapp.c
SERVER_MODULES=server.c user_list.c chat_room_list.c utils/csapp.c utils/select_pool.c user_info/file_manipulator.c

all: $(BINS)

clean:
	rm -f *.out *.o client server

$(CLIENT_BIN): clean
	$(CC) $(CFLAGS) -pthread $(CLIENT_MODULES) -o $(CLIENT_BIN)

$(SERVER_BIN): clean
	$(CC) $(CFLAGS) -pthread $(SERVER_MODULES) -o $(SERVER_BIN) -lssl -lcrypto
