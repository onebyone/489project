#ifndef __NETW_UTIL_H_
#define __NETW_UTIL_H_

#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#include <arpa/inet.h>
#include <string>

#include <unistd.h>
#include <fstream>
#include <iostream>

using namespace std;
#define BUFFER_SIZE 1024
#define PIECE_SIZE 4096*1024
#define MAX_COMMAND_LEN 20
#define FILE_NAME_MAX_SIZE 256

void download_from_server(char* file_name, char* ip);
void sendUpdate(char* file_name,char* tracker, string piece_info_str);
int send_piece_from_file(char* file_name, long size, int piece_num, int socket);
int form_connection(const char* ip, int port);
int send_file(char* file_name, int receiver_socket);
int receive_file(char* file_name, int receiver_socket, long file_size);



#endif