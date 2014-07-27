#include "netw_util.h"
using namespace std;


void download_from_server(char* file_name, char* ip)
{
    cout << "Enter download torrent peer\n";
    
    char buffer[BUFFER_SIZE];

    int client_socket = form_connection(ip, 6666);
    if (client_socket < 0)
    {
        cout << "Download file " << file_name << " from " << ip << " failed\n";
    }

    int index = 0;
    memcpy(buffer, "request", MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;
    memcpy(buffer + index, file_name, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    send(client_socket,buffer,BUFFER_SIZE,0);

    bzero(buffer, BUFFER_SIZE);
    recv(client_socket,buffer,BUFFER_SIZE,0);
    long file_size;
    char response[MAX_COMMAND_LEN];
    index = 0;
    memcpy(response, buffer, MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;

    memcpy(&file_size, buffer + index, sizeof(file_size));

    if (0 == strcmp(response, "OK"))
    {
        receive_file(file_name, client_socket, file_size);
    }

    close(client_socket);
}

int send_piece(char* file_name, long size, int piece_num, int socket)
{
    cout << "Going to send piece\n";
    cout << "The piece size is " << size << endl;
    ifstream input(file_name, ios::binary);
    if (NULL == input)
    {
        cout << "Cannot open file " << file_name << endl;
        cout << "Exiting" << endl;
        exit(1);
    }
    char piece[size];
    char buffer[BUFFER_SIZE];
    input.seekg(piece_num * PIECE_SIZE);
    input.read(piece, size);
    input.close();


    int index = 0;
    int block_size;

    while (size > 0)
    {
        if (size - BUFFER_SIZE < 0)
        {
            block_size = size;
        }
        else
        {
            block_size = BUFFER_SIZE;
        }
        memcpy(buffer, piece + index, block_size);
        if(send(socket,buffer,block_size,0)<0)
        {
            printf("Send File:\t%s Failed\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
        size -= BUFFER_SIZE;
        index += BUFFER_SIZE;
    }
    cout << "send piece done\n";
    return 0;
}

void sendUpdate(char* file_name,char* tracker, string piece_info_str)
{
    char buffer[BUFFER_SIZE];
    int tracker_socket = form_connection(tracker, 6666);
    
    if (tracker_socket < 0)
    {
        cout << "Download file " << file_name << " from " << tracker << " failed\n";
    }

    int index = 0;
    memcpy(buffer, "update", MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;
    memcpy(buffer + index, file_name, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    char piecr_info[512];
    strcpy(piecr_info, piece_info_str.c_str());
    memcpy(buffer + index, piecr_info, 512);
    send(tracker_socket, buffer, BUFFER_SIZE, 0);
}


int form_connection(const char* ip, int port)
{
    cout << "enter form_connection\n";
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

    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof(server_addr);

    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        cout << "inside if?" << endl;
        printf("Can Not Connect To %s!\n",ip);
        return -1;
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
    

    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    while( file_size > 0)
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