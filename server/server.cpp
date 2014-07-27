#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero


#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string>

#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <vector>


using namespace std;
#define HELLO_WORLD_SERVER_PORT    6666
#define LENGTH_OF_LISTEN_QUEUE  20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 256
#define TORRENT_LIST "torrent_list"
#define MAX_CONNECTIONS 512
#define PEER_INFO_LEN 512
#define MAX_FILE 512
#define MAX_COMMAND_LEN 20

mutex torrent_file, threads_stats;
mutex peer_list_lock[MAX_FILE];


int send_file(char* file_name, int receiver_socket);
int receive_file(char* file_name, int receiver_socket, long file_size);
long get_file_size(char* file_name);
void update_generate_torrent_list(char* file_name, long file_size);
int get_free_thread_id();
void update_peerlist(char* file_name, string ip, string piece_info);
string generate_piece_info(long size);
long get_file_size_from_torrent(char* torrent_name);

struct thread_data_t{
   int thread_id;
   int socket;
   string ip;
   int port;
};
bool thread_used[MAX_CONNECTIONS];
pthread_t thread_pool[MAX_CONNECTIONS];
thread_data_t thread_args[MAX_CONNECTIONS];

void *process_client(void *thread_data_ptr_arg)
{
    thread_data_t thread_data = *(thread_data_t*) thread_data_ptr_arg;

    int new_server_socket = thread_data.socket;
    int thread_id = thread_data.thread_id;
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    while (1)
    {
        cout << "Listen to the client" << endl;
        int length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
        if (length < 0)
        {
            printf("Server Recieve Data Failed!\n");
            break;
        }
    
        char command[MAX_COMMAND_LEN];
        char file_name[FILE_NAME_MAX_SIZE];
        long file_size;
        int index = 0;
        memcpy(command, buffer, MAX_COMMAND_LEN);
        index += MAX_COMMAND_LEN;

        memcpy(file_name, buffer + index, FILE_NAME_MAX_SIZE);
        index += FILE_NAME_MAX_SIZE;
   
        
        if (0 == strcmp(command, "sendTorrent") )
        {
	        cout << "The command is " << command << endl;
        	cout << "File name is " << file_name << endl;

            memcpy(&file_size, buffer + index, sizeof(file_size));
            cout << "File size is " << file_size << endl;

            receive_file(file_name, new_server_socket, file_size);
            cout << "Finish receiving" << endl;

            file_size = get_file_size_from_torrent(file_name);
            update_generate_torrent_list(file_name, file_size);
            string peer_info = generate_piece_info(file_size);

            update_peerlist(file_name, thread_data.ip, peer_info);
        }
        else if (0 == strcmp(command, "request"))
        {
	        cout << "The command is " << command << endl;
            cout << "Going to send file " << file_name << endl;

    		file_size = get_file_size(file_name);
            bzero(buffer, BUFFER_SIZE);
            index = 0;
    		memcpy(buffer, "OK", MAX_COMMAND_LEN);
            index += MAX_COMMAND_LEN;
            memcpy(buffer + index, &file_size, sizeof(file_size));
            send(new_server_socket,buffer,BUFFER_SIZE,0);
            send_file(file_name, new_server_socket);
    		
        }
        else if (0 == strcmp(command, "update"))
        {
            cout << "The command is " << command << endl;
            cout << "Going to update peer list of file " << file_name << endl;
            
            char peer_info[PEER_INFO_LEN];
            memcpy(peer_info, buffer + index, PEER_INFO_LEN); 

            update_peerlist(file_name, thread_data.ip, string(peer_info));
        }
        else
        {
            close(new_server_socket);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }
    cout << "Thread " << thread_id << " finish\n";
    threads_stats.lock();
    pthread_detach(pthread_self());
    thread_used[thread_id] = false;
    threads_stats.unlock();

}

int main(int argc, char **argv)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

    int server_socket = socket(PF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        exit(1);
    }


    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT);
        exit(1);
    }

    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!");
        exit(1);
    }


    

    fill_n(thread_used, MAX_CONNECTIONS, false);

    while (1) 
    {
	    printf("in the loop listening.....\n");

        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);

        if ( new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
        int thread_id = get_free_thread_id();
        if (thread_id == MAX_CONNECTIONS)
        {
            cout << "Server is busy\n";
            close(new_server_socket);
            continue;
        }
        
        cout << "Going to assemble\n";
        
        thread_args[thread_id].thread_id = thread_id;
        thread_args[thread_id].socket = new_server_socket;
        thread_args[thread_id].ip = string(inet_ntoa(client_addr.sin_addr));
        thread_args[thread_id].port = (int) ntohs(client_addr.sin_port);
 
        cout << "Going to create thread\n";
        int rc = pthread_create(&thread_pool[thread_id], NULL, 
                          process_client, (void *)&thread_args[thread_id]);

        threads_stats.lock();
        thread_used[thread_id] = true;
        threads_stats.unlock();
        
    }

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
            printf("file_block_length = %d\n",file_block_length);

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
    cout << "Recieve File:\t "<<file_name<<" From client Finished\n";
    
    fclose(fp);
}

long get_file_size(char* file_name)
{
    cout << "in get size" << endl;
    FILE * fp = fopen(file_name,"r");
    if (fp)
    {
        fseek (fp, 0, SEEK_END);
        long file_size_long = ftell (fp);
        fclose(fp);

        return file_size_long;
    }
    cout << "Cannot open file " << file_name << endl;
    exit(1);
}



void update_generate_torrent_list(char* file_name, long file_size)
{
    ofstream outfile;

    torrent_file.lock();

    outfile.open(TORRENT_LIST, ios_base::app);
    outfile << file_name << " size: " << file_size << '\n'; 
    outfile.close();
    torrent_file.unlock();
    cout << "File " << file_name << " added to torrent list.\n";
}

int get_free_thread_id()
{
    threads_stats.lock();
    int i = 0;
    for (; i < MAX_CONNECTIONS; i++)
    {
        if (!thread_used[i])
        {
            break;
        }
    }
    threads_stats.unlock();
    return i;
}


string generate_piece_info(long size)
{
    string piece_info = "";
    double size_in_K = size/1024.0;

    int piece_size = 4096;
    int char_size = 4096 * 4;
    
    while (size_in_K > char_size)
    {
        size_in_K -= char_size;
        piece_info += "f";
    }

    int last_char = 0;
    int init = 8;
    
    while (size_in_K > 0)
    {
        size_in_K -= piece_size;
        last_char += init;
        init /= 2;
    }
    
    stringstream ss;
    ss << std::hex << last_char;
    piece_info += string(ss.str());
    return piece_info;
}


void update_peerlist(char* file_name, string ip, string piece_info)
{
    string peer_list = string(file_name);
    int pos = peer_list.find(".torrent");
    if (-1 == pos && peer_list.size() - pos == 8)
    {
        peer_list += ".peers";
    }
    else
    {
        cout << "in else" <<  endl;
        peer_list = peer_list.substr(0, pos) + ".peers";
    }
    cout << "In update peers file name is " << peer_list << endl;

    string temp_list = peer_list + ".temp";

    ifstream read_list(peer_list);
    ofstream write_list(temp_list);

    bool add_done = false;
    if (read_list)
    {
        string line, ip_temp, new_peer_info;
        int line_num = 0;
        while (getline(read_list, line))
        {
            istringstream line_stream;
            line_stream.str(line);

            line_stream >> ip_temp;
            if (0 == ip.compare(ip_temp))
            {
                line_stream >> new_peer_info;

                line = ip + " " + piece_info;
                add_done = true;
            }
            write_list << line + "\n";
        }
        if (!add_done)
        {
            line = ip + " " + piece_info;
            write_list << line + "\n";
        }
        read_list.close();
        remove(peer_list.c_str());
    }
    else
    {
        write_list << ip + " " + piece_info + "\n";
    }
    write_list.close();
    rename(temp_list.c_str(), peer_list.c_str());
}

long get_file_size_from_torrent(char* torrent_name)
{
    ifstream fin(torrent_name);
    if (NULL == fin)
    {
        cout << "Cannot open torrent " << torrent_name << endl;
        return -1;
    }
    string line;
    getline(fin, line);
    getline(fin, line);
    long file_size = stol(line.c_str());
    return file_size;
}