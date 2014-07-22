#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <cstring>
#include <cstdio>

#include <errno.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <mutex>
#include <string>
#include "create_torrent.h"
#include "analyze_peerlist.h"

// create directory
#include <sys/stat.h>
#include <sys/types.h>


#define HELLO_WORLD_SERVER_PORT    6666 
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 256
#define MAX_CONNECTIONS 512
#define MAX_COMMAND_LEN 20
#define LENGTH_OF_LISTEN_QUEUE  20
#define MAX_SEEDING 128
#define MAX_DOWNLOAD 8
#define PIECE_SIZE 4096*1024

using namespace std;


int send_file(char* file_name, int receiver_socket);
int receive_file(char* file_name, int receiver_socket, long file_size);
long get_file_size(char* file_name);
void *listen_seed_request(void*);
int form_connection(char* ip, int port);
void generate_publish_torrent();
void download_file(char* file_name);
char* download_torrent_peer(char* file_name, bool torrent);
void *seed_file(void* seeding_data);
void anaylsis_download_file(char* file_name);
int send_piece(char* file_name, long size, long piece_num, int socket);

pthread_t seed_thread;
pthread_t download_thread;
pthread_t listen_request;


int main(int argc, char **argv){

    char command[MAX_COMMAND_LEN];
    char serverAddress[16];
    cout << "Program start\n";
    int dumb;
    while (1)
    {
        cout << "What do you want to do?: " ;
        cin >> command;
        if (strcmp(command, "quit") == 0)
        {
            break;
        }
        else if (strcmp(command, "seed") == 0)
        {
            int rc = pthread_create(&listen_request, NULL, 
                listen_seed_request, NULL);

        }
        else if (strcmp(command, "sendTorrent") == 0)
        {
            generate_publish_torrent();
        }
        else if (strcmp(command, "download") == 0)
        {
            char file_name[FILE_NAME_MAX_SIZE];
            cout << "Which file to download? ";
            cin >> file_name;
            download_file(file_name);
            // int rc = pthread_create(&download_thread, NULL, 
            //     download_file, (void*)file_name);
            // pthread_join(download_thread);
        }
    }

}



void generate_publish_torrent()
{
    char file_name[FILE_NAME_MAX_SIZE];
    char buffer[BUFFER_SIZE];
    cout << "Which file to generate torrent? ";
    cin >> file_name;
    char server_ip[16];
    cout << "Enter server ip: ";
    cin >> server_ip;
    create_torrent(file_name, server_ip);
  
    string torrent_name_str = string(file_name);
    torrent_name_str.erase(torrent_name_str.size() -1 );
    torrent_name_str += ".torrent";
    cout << torrent_name_str << endl;

    cout << torrent_name_str.size() << endl;
    char torrent_name[FILE_NAME_MAX_SIZE];
    strcpy(torrent_name, torrent_name_str.c_str());

    int client_socket = form_connection(server_ip, 6666);

    int index = 0;
    memcpy(buffer, "sendTorrent", MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;

    memcpy(buffer + index, torrent_name, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    long file_size = get_file_size(torrent_name);
    cout << "File size is " << file_size << endl;
    memcpy(buffer + index, &file_size, sizeof(file_size));

    send(client_socket,buffer , BUFFER_SIZE,0);
    send_file(torrent_name, client_socket);
    close(client_socket);
}

void download_file(char* file_name)
{
    cout << "enter download file thread\n";
    //TODO
    // string dir = string(file_name) + "+data";
    // mkdir(dir.c_str(), 0777);
    //char* file_name = (char*) file_name_arg;
    char* torrent_file = download_torrent_peer(file_name, true);
    char* peer_list = download_torrent_peer(file_name, false);

    anaylsis_download_file(peer_list);
}

char* download_torrent_peer(char* file_name, bool torrent)
{
    cout << "Enter download torrent peer\n";
    string download_name_str;
    
    char buffer[BUFFER_SIZE];
    if (torrent) 
    {
        download_name_str = string(file_name) + ".torrent";
    }
    else
    {
        download_name_str = string(file_name) + ".peers";
    }

    char download_name[FILE_NAME_MAX_SIZE];
    strcpy(download_name, download_name_str.c_str());

    char server_ip[16];
    cout << "Enter server ip: ";
    cin >> server_ip;
    int client_socket = form_connection(server_ip, 6666);

    int index = 0;
    memcpy(buffer, "request", MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;
    memcpy(buffer + index, download_name, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    send(client_socket,buffer,BUFFER_SIZE,0);

    bzero(buffer, BUFFER_SIZE);
    recv(client_socket,buffer,BUFFER_SIZE,0);
    long file_size;
    char response[MAX_COMMAND_LEN];
    index = 0;
    memcpy(response, buffer, sizeof(MAX_COMMAND_LEN));
    index += MAX_COMMAND_LEN;

    memcpy(&file_size, buffer + index, sizeof(file_size));

    if (0 == strcmp(response, "OK"))
    {
        receive_file(download_name, client_socket, file_size);
    }
    cout << "Returned to download torrent\n";
    close(client_socket);
    cout << "Closed" << endl;
    return download_name;
}



void anaylsis_download_file(char* peerlist, char* torrent_file)
{
   
}


void *listen_seed_request(void*)
{
    cout << "Thread start\n";
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(2080);


    char buffer[BUFFER_SIZE];

    int client_socket = socket(AF_INET,SOCK_STREAM,0);

    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }
    cout << "Seeding start\n";

    if ( listen(client_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!");
        exit(1);
    }

    while(1)
    {
        cout << "Listen to peer\n";
        struct sockaddr_in peer_addr;
        socklen_t length = sizeof(peer_addr);
        int peer_socket = accept(client_socket,(struct sockaddr*)&peer_addr,&length);
        cout << "Connected to a peer\n";
        if ( peer_socket < 0)
        {
            printf("Client Accept Failed!\n");
            break;
        }
        
        int rc = pthread_create(&seed_thread, NULL, 
            seed_file, (void*)&peer_socket);
    }
}

void *seed_file(void* seeding_data)
{
    //string file_name = seeding_data.file_name;
    //long piece = seeding_data.piece;
    int peer_socket = *(int*)seeding_data;

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    //TODO woshou

    int length = recv(peer_socket,buffer,BUFFER_SIZE,0);
    if (length < 0)
    {
        printf("Server Recieve Data Failed!\n");
        close(peer_socket);
        pthread_exit(0);
    }

    char command[10];
    char file_name[FILE_NAME_MAX_SIZE];
    long file_size;
    long piece_num;
    int index = 0;
    memcpy(command, buffer, MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;

    memcpy(file_name, buffer + index, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    memcpy(&piece_num, buffer + index, sizeof(piece_num));
    index += sizeof(piece_num);

    memcpy(&file_size, buffer + index, sizeof(file_size));

    send_piece(file_name, file_size, piece_num, peer_socket);
    
    close(peer_socket);

}

int send_piece(char* file_name, long size, long piece_num, int socket)
{
    ifstream input(file_name, ios::binary);
    char piece[size];
    char buffer[BUFFER_SIZE];
    input.seekg(piece_num * PIECE_SIZE);
    input.read(piece, size);
    
    int index = 0;
    int block_size;
    while (size > 0)
    {
        if (size - BUFFER_SIZE < 0)
        {
            block_size = size - BUFFER_SIZE;
        }
        else
        {
            block_size = BUFFER_SIZE;
        }
        memcpy(piece + index, buffer, block_size);
        if(send(socket,buffer,block_size,0)<0)
        {
            printf("Send File:\t%s Failed\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
        size -= BUFFER_SIZE;
    }
    return 0;
}

int form_connection(char* ip, int port)
{
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);

    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }


    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(ip,&server_addr.sin_addr) == 0)
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);

    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n",ip);
        exit(1);
    }

    return client_socket;
}

int send_file(char* file_name, int receiver_socket)
{
    FILE * fp = fopen(file_name,"r");
    char buffer[BUFFER_SIZE];
    if(NULL == fp )
    {
        printf("File:\t%s Not Found\n", file_name);
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
        int file_block_length = 0;

        while( (file_block_length = fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0)
        {
            //printf("file_block_length = %d\n",file_block_length);

            if(send(receiver_socket,buffer,file_block_length,0)<0)
            {
                printf("Send File:\t%s Failed\n", file_name);
                break;
            }
            bzero(buffer, BUFFER_SIZE);
        }             
        fclose(fp);
        printf("File:\t%s Transfer Finished\n",file_name);
        //close(receiver_socket);
    }
}

int receive_file(char* file_name, int receiver_socket, long file_size)
{
    cout << "In receive_file " << file_name << endl;
    FILE * fp = fopen(file_name,"w");
    if(NULL == fp )
    {
        printf("File:\t%s Can Not Open To Write\n", file_name);
        exit(1);
    }
    
    //从服务器接收数据到buffer中
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    while( file_size > 0)       //循环接收，再写到文件
    {
        length = recv(receiver_socket,buffer,BUFFER_SIZE,0);
        if(length < 0)
        {
            cout << "Recieve Data From Server Failed!\n";
            break;
        }
        int write_length = fwrite(buffer,sizeof(char),length,fp);
        if (write_length<length)
        {
            printf("File:\t%s Write Failed\n", file_name);
            break;
        }
        bzero(buffer,BUFFER_SIZE);  
        file_size -=  length;
    }
    cout << "Recieve File:\t "<<file_name<<" From server Finished\n";
    
    fclose(fp);
}


long get_file_size(char* file_name)
{
    cout << "in get size" << endl;
    FILE * fp = fopen(file_name,"r");
    fseek (fp, 0, SEEK_END);   // non-portable
    long file_size_long = ftell (fp);
    fclose(fp);

    return file_size_long;
}