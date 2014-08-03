#include <cstring>
#include <cstdio>

#include <errno.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <pthread.h>
#include <mutex>
#include <vector>
#include "file_ops.h"
#include "netw_util.h"
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

struct download_piece_info_t
{
    char* ip;//[20];
    int port;
    char* file_name;//[FILE_NAME_MAX_SIZE];
    int piece_num;
    long piece_size;
};

void *listen_seed_request(void*);
void generate_publish_torrent();
void download_torrent(char* file_name);
void* download_file(void* file_name_arg);
void *seed_file(void* seeding_data);
void analyze_download_file(char* torrent);
bool request_download_piece(const download_piece_info_t&);
void getTorrentList();
void showTorrent();
string update_piece_info(const vector<bool> &piece_record);

pthread_t seed_thread;
pthread_t download_thread;
pthread_t listen_request;
char server_ip[20];


int main(int argc, char **argv){

    char command[MAX_COMMAND_LEN];
    char serverAddress[16];
    cout << "Program start\n";
    cout << "Please set the server ip first\n";
    cout << "Server ip: ";
    cin >> server_ip;
    cout << "Server ip is set to " << server_ip << endl;

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
        else if (strcmp(command, "getTorrentList") == 0)
        {
            getTorrentList();
        }
        else if (strcmp(command, "downloadTorrent") == 0)
        {
            char file_name[FILE_NAME_MAX_SIZE];
            cout << "Which file's torrent to download? ";
            cin >> file_name;
            download_torrent(file_name);
        }
        else if (strcmp(command, "downloadFile") == 0)
        {
            char *file_name;
            file_name = new char[FILE_NAME_MAX_SIZE];
            cout << "Which file to download? ";
            cin >> file_name;
            int rc = pthread_create(&download_thread, NULL,
                download_file, (void*)file_name);
            pthread_create(&listen_request, NULL, 
                listen_seed_request, NULL);		
        }
        else if (strcmp(command, "changeServerIP") == 0)
        {
            cin >> server_ip;
            cout << "Server ip is set to " << server_ip << endl;
        }

        else
        {
            cout << "Usage:\n";
            cout << "Available commands are quit, seed, sendTorrent, getTorrentList, downloadTorrent, downloadFile, contDownloadFile\n";

        }
    }

}



void generate_publish_torrent()
{
    char file_name[FILE_NAME_MAX_SIZE];
    char buffer[BUFFER_SIZE];
    cout << "Which file to generate torrent? ";
    cin >> file_name;
    if(create_torrent(file_name, server_ip) < 0)
    {
        cout << "create Torrent " << file_name << "failed\n";
    }
  
    string torrent_name_str = string(file_name);
    torrent_name_str.erase(torrent_name_str.size() -1 );
    torrent_name_str += ".torrent";
    cout << torrent_name_str << endl;

    cout << torrent_name_str.size() << endl;
    char torrent_name[FILE_NAME_MAX_SIZE];
    strcpy(torrent_name, torrent_name_str.c_str());

    int client_socket = form_connection(server_ip, 6666);
    if (client_socket < 0)
    {
        "Publish torrent failed\n";
        return;
    }
    int index = 0;
    memcpy(buffer, "sendTorrent", MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;

    memcpy(buffer + index, torrent_name, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    long file_size = get_file_size(torrent_name);
    if (file_size < 0)
    {
        cout << "Error occured in get " << torrent_name << "\'s size\n";
        return;
    }

    memcpy(buffer + index, &file_size, sizeof(file_size));

    send(client_socket,buffer , BUFFER_SIZE,0);
    send_file(torrent_name, client_socket);
    close(client_socket);
}

void download_torrent(char* file_name)
{
    strcat(file_name, ".torrent");
    download_from_server(file_name, server_ip);

}
void* download_file(void* file_name_arg)
{
    char* file_name = (char*) file_name_arg;
    strcat(file_name, ".torrent");
    analyze_download_file(file_name);
    delete [] file_name;
}


void analyze_download_file(char* torrent_file)
{
    cout << "Enter analyze_download_file\n";
    ifstream read_torrent(torrent_file);

    string line;
    char file_name[FILE_NAME_MAX_SIZE], tracker[256];
    long file_size;
    int num_piece;
    vector<string> hash_code;
    vector<ip_struct> active_ip;

    if (read_torrent)
    {
        getline(read_torrent, line);
        strcpy(file_name, line.c_str());

        getline(read_torrent, line);
        file_size = stol(line.c_str());
        getline(read_torrent, line);
        num_piece = atoi(line.c_str());
        cout << "num_piece is " << num_piece << endl;
        
        for(int i=0; i < num_piece; i++)
        {
            getline(read_torrent, line);
            hash_code.push_back(line);
        }
        getline(read_torrent, line);
        strcpy(tracker, line.c_str());
        cout << "Tracker is " << tracker << endl;
        read_torrent.close();
    }
    else
    {
        cout << "Cannot open torrent file " << torrent_file << endl;
        return;
    }
    
    cout << "Going to reach peer to download\n";
    vector<bool> completion_record(num_piece, false);
    int completion_counts = 0;
    if ( 0 > check_prev_download(completion_record, completion_counts, file_name))
    {
        cout << "File " << file_name << " has already been downloaded\n";
        return;
    }
    char peerlist[FILE_NAME_MAX_SIZE];
    strcpy(peerlist, file_name);
    strcat(peerlist, ".peers");
    download_from_server(peerlist, tracker);
    add_ip_from_peerlist(active_ip, peerlist);

    int loop_count = 0;
    //bool file_complete = true;
    while (completion_counts < num_piece)
    {
        for (int i = 0; i <  num_piece; i++)
        {
            if (completion_record[i])
            {
                continue;
            }
            string ip = choose_peer(i, peerlist, active_ip);

	    loop_count++;
            if ("" != ip)
            {
                cout << "ip is " << ip << endl;
                long piece_size;
                if (i == num_piece - 1)
                {
                    piece_size = file_size - i * PIECE_SIZE;
                }
                else
                {
                    piece_size = PIECE_SIZE;
                }
                cout << "After set piece_size " <<piece_size << endl;

                download_piece_info_t download_piece_info;
                
                download_piece_info.file_name = file_name;
                download_piece_info.port = 2080;
                download_piece_info.piece_size = piece_size;
                download_piece_info.piece_num = i;
                char ip_arr[20];
                strcpy(ip_arr, ip.c_str());
                download_piece_info.ip = ip_arr;


                if (request_download_piece(download_piece_info))
                {
	            completion_counts ++ ;
                    completion_record[i] = true;

                    sendUpdate(file_name, tracker, update_piece_info(completion_record));
                }
		else
		  remove_ip(active_ip, ip);
            }

            if (loop_count>19) {  
                download_from_server(peerlist, tracker);
    		add_ip_from_peerlist(active_ip, peerlist);  
		loop_count=0;
	    }
        }
    	if (active_ip.empty()) {
    	    cout << "currently there is no more seeds availble for " << file_name << endl;
    	    break;
    	}
    }
    if (completion_counts == num_piece)
    {
        if (combine_tmp(file_name,num_piece) <0)
        {
            	cout << "Combine piece file failed\n"; 
        }
    }  
    //for (bool sta : completion_record)
    //{
       // cout << sta << endl;
    //}
}

string update_piece_info(const vector<bool> &piece_record)
{
    int base = 8;
    int value_in_dec = 0;
    string piece_info = "";
    int num_piece = piece_record.size();

    for (int i = 0; i< num_piece; i++)
    {
        if (i%4 == 0 && i > 0)
        {
            base = 8;
            stringstream ss;
            ss << std::hex << value_in_dec;
            piece_info += string(ss.str());
            value_in_dec = 0;
        }
        if (piece_record[i])
        {
            value_in_dec += base;
        }
        base /= 2;
    }
    if (0 != value_in_dec)
    {
        stringstream ss;
        ss << std::hex << value_in_dec;
        piece_info += string(ss.str());
    }
    return piece_info;
}

bool request_download_piece(const download_piece_info_t &download_info)
{
    int index = 0;
    char buffer[BUFFER_SIZE];
    char file_name [FILE_NAME_MAX_SIZE];

    memcpy(file_name, download_info.file_name, FILE_NAME_MAX_SIZE);
    bzero(buffer, BUFFER_SIZE);

    memcpy(buffer, "requestPiece", MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;

    cout << "file_name is " << download_info.file_name << endl;
    memcpy(buffer + index, download_info.file_name, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    int piece_num = download_info.piece_num;
    cout << "piece is " << piece_num << endl;
    memcpy(buffer + index, &piece_num, sizeof(piece_num));
    index += sizeof(piece_num);

    
    long piece_size = download_info.piece_size;
    cout << "file size is " << piece_size << endl;
    memcpy(buffer + index, &piece_size, sizeof(piece_size));

    cout << "Before peer socket ip is " << download_info.ip << endl;   
    int peer_socket = form_connection(download_info.ip, download_info.port);

    if (peer_socket < 0)
    {
        cout << "Cannot connect to peer " << download_info.ip << endl;
        return false;
    }

    cout << "Set up connection done\n";

    send(peer_socket, buffer, BUFFER_SIZE, 0);


    bzero(buffer, BUFFER_SIZE);
    recv(peer_socket, buffer, BUFFER_SIZE, 0);
    char ack[MAX_COMMAND_LEN];
    memcpy(ack, buffer, MAX_COMMAND_LEN);
    
    char piece_name[FILE_NAME_MAX_SIZE];
    sprintf(piece_name,"%s_%d",file_name, piece_num);
    if (0 == strcmp(ack, "OK"))
    {
        cout << "It is OK\n";
        cout << "file piece name is " << piece_name << endl;
        receive_file(piece_name, peer_socket, piece_size);
        char torrent_name[FILE_NAME_MAX_SIZE];

        sprintf(torrent_name,"%s.torrent",file_name);
        if (!verify_piece(torrent_name, piece_name, piece_num))
        {
            cout << "Error occured in downloading piece " << piece_num << endl;
            return false;
        }
        return true;
    }   
    else
    {
        cout << "No acknowledga from peer\n";
        return false;

    }
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
        pthread_exit(0);
    }

    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        pthread_exit(0);
    }
    cout << "Seeding start\n";

    if ( listen(client_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("client Start Listen Peer Failed!");
        pthread_exit(0);
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
            seed_file, (void*)peer_socket);
    }
    pthread_detach(pthread_self());
}

void *seed_file(void* seeding_data)
{
    long peer_socket = (long)seeding_data;

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    int length = recv(peer_socket,buffer,BUFFER_SIZE,0);
    if (length < 0)
    {
        printf("Server Recieve Data Failed!\n");
        close(peer_socket);
        pthread_exit(0);
    }

    char command[MAX_COMMAND_LEN];
    char file_name[FILE_NAME_MAX_SIZE];
    long file_size;
    int piece_num;
    int index = 0;
    memcpy(command, buffer, MAX_COMMAND_LEN);
    index += MAX_COMMAND_LEN;

    memcpy(file_name, buffer + index, FILE_NAME_MAX_SIZE);
    index += FILE_NAME_MAX_SIZE;

    memcpy(&piece_num, buffer + index, sizeof(piece_num));
    index += sizeof(piece_num);

    memcpy(&file_size, buffer + index, sizeof(file_size));

    Have_piece have_piece= is_file_piece_exist(file_name, piece_num);
    if (0 == strcmp(command, "requestPiece") && have_piece < 2)
    {
        bzero(buffer, BUFFER_SIZE);
        memcpy(buffer, "OK", MAX_COMMAND_LEN);
        send(peer_socket,buffer, BUFFER_SIZE, 0);
        cout << "send OK done\n";
        int send_success = 0;
        if (have_piece == HAVE_FILE)
        {
            send_success = send_piece_from_file(file_name, file_size, piece_num, peer_socket);
        }
        else
        {
            sprintf(file_name,"%s_%d",file_name, piece_num);
            send_success = send_file(file_name, peer_socket);
        }
        if (send_success < 0)
        {
            cout << "Send piece " << file_name << " " << piece_num << "failed" << endl;
        }
    }
    else
    {
        cout << "Not seeding the piece\n";
        cout << "Result num is " << have_piece << endl;
        cout << "Requested piece is " << file_name << " " << piece_num << endl;
    }
    close(peer_socket);
    pthread_detach(pthread_self());

}


void getTorrentList()
{
    char file_name[FILE_NAME_MAX_SIZE] = "torrent_list";
    download_from_server(file_name, server_ip);
    showTorrent();
}

void showTorrent()
{
    ifstream inf("torrent_list");
    if (inf == NULL)
    {
        cout << "Error cannot open torrent list\n";
        return;
    }
    string line;
    while (getline(inf, line))
    {
        cout << line << endl;
    }
}
