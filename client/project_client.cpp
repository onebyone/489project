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
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
*/

#define HELLO_WORLD_SERVER_PORT    6666 
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 256

using namespace std;

int send_file(char* file_name, int receiver_socket);
int receive_file(char* file_name, int receiver_socket, long file_size);
long get_file_size(char* file_name);
int job(char* command, char * serverAddress);

int main(int argc, char **argv){

    char command[10];
    char serverAddress[16];
    while (1)
    {
        cout << "What do you want to do?: " ;
        cin >> command;
        if (strcmp(command, "quit") == 0)
        {
            break;
        }
        else if (strcmp(command, "send") == 0)
        {
            cout<<"Input Server IP: ";
            cin >> serverAddress;
            job(command,serverAddress);
        }
        else if (strcmp(command, "request") == 0)
        {
            cout<<"Input Server IP: ";
            cin >> serverAddress;            
            job(command,serverAddress);
        }
    }

}


int job(char* command, char * serverAddress)
{

    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(serverAddress,&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n",serverAddress);
        exit(1);
    }

    
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    //char* file_name;
    char file_name[FILE_NAME_MAX_SIZE];
    //strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
    //向服务器发送buffer中的数据

    if (strcmp(command, "send") == 0)
    {
        cout << "Which file to send?: ";
        cin >> file_name;
        cout << file_name << endl;


        //get_file_size(file_name);
        int index = 0;
        memcpy(buffer, command, 10);
        index += 10;

        memcpy(buffer + index, file_name, FILE_NAME_MAX_SIZE);
        index += FILE_NAME_MAX_SIZE;

        long file_size = get_file_size(file_name);
        cout << "File size is " << file_size << endl;
        memcpy(buffer + index, &file_size, sizeof(file_size));

        send(client_socket,buffer , BUFFER_SIZE,0);
        send_file(file_name, client_socket);
    }
    else if (strcmp(command, "request") == 0)
    {
        cout << "Which file to request?:";
        cin >> file_name;

        int index = 0;
        memcpy(buffer, command, 10);
        index += 10;
        memcpy(buffer + index, file_name, FILE_NAME_MAX_SIZE);

        //strncpy(buffer, command, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(command));
        send(client_socket,buffer,BUFFER_SIZE,0);

        bzero(buffer, BUFFER_SIZE);
        recv(client_socket,buffer,BUFFER_SIZE,0);
        long file_size;
        index = 10 + FILE_NAME_MAX_SIZE;
        memcpy(&file_size, buffer + index, sizeof(file_size));   
        receive_file(file_name, client_socket, file_size);
    }

    cout << "Disconnectting\n";
    close(client_socket);
    return 0;
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
