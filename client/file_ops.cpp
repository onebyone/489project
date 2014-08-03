#include "file_ops.h"

#define PIECESIZE (4096*1024)

using namespace std;

void gethash(unsigned char* input,char* hash);

int combine_tmp(const char* file_name, long piece_num){
	FILE* fd1;
	FILE* fd2;
	long size;
	char tmp_name[255];
	char buff[PIECESIZE];

	fd1 = fopen(file_name,"wb");
	if (fd1==NULL) 
	{
		fputs ("File error\n",stderr); 
		return -1;
	}

	for (int i=0; i<piece_num;i++){
		sprintf(tmp_name,"%s_%d",file_name, i);

		fd2 = fopen(tmp_name,"rb");


		fseek (fd2 , 0 , SEEK_END);
  		size = ftell (fd2);
  		rewind (fd2);

		fread(buff, sizeof(char), size, fd2);

		fwrite(buff, sizeof(char), size, fd1);

		
		fclose(fd2);

		//remove(tmp_name);
	}

	fclose(fd1);
	return 0;	
}

int create_torrent(char* file_name, char* tracker_name)
{
	FILE* fd;
	FILE* ftorrent;
	long size;
	unsigned char buff[PIECESIZE];		

	char torrent_name[256];
	strcpy(torrent_name,file_name);
	strcat(torrent_name,".torrent"); 

	fd = fopen(file_name,"r");
	if (fd==NULL) 
	{
		fputs ("File error\n",stderr); 
		return -1;
	}
	ftorrent=fopen(torrent_name,"w");
	if (ftorrent==NULL) 
	{
		fputs ("File error\n",stderr); 
		return -1;
	}

	// obtain file size
	fseek (fd , 0 , SEEK_END);
  	size = ftell (fd);
  	rewind (fd);


	long part_num = size / PIECESIZE + (size % PIECESIZE != 0);

	char size_str[256];
	char part_str[256];
	sprintf(size_str,"%ld\n", size);
	sprintf(part_str,"%ld\n", part_num);
	strcat(file_name,"\n"); 
	strcat(torrent_name,"\n"); 

	fwrite(file_name, sizeof(char), sizeof(char)*strlen(file_name), ftorrent);
	fwrite(size_str, sizeof(char), sizeof(char)*strlen(size_str), ftorrent);
	fwrite(part_str, sizeof(char), sizeof(char)*strlen(part_str), ftorrent);



	for (int i=0; i<part_num; i++) {
		if (i==part_num-1 && (size % PIECESIZE))
			fread (buff,1,size % PIECESIZE, fd);
		else
			fread (buff,1, PIECESIZE,fd);
		char hash[255];
		gethash(buff,hash);
		char hash_str[256];
		sprintf(hash_str,"%s\n", hash);
		fwrite(hash_str, sizeof(char), sizeof(char)*strlen(hash_str), ftorrent);		
	}


	fwrite(tracker_name, sizeof(char), sizeof(char)*strlen(tracker_name), ftorrent);

	fclose(fd);
	fclose(ftorrent);
}

int verify_piece(char* torrent_addr, char* piece_addr, long piece_num){
	
	FILE* fd;
	FILE* ftorrent;
	long size;
	unsigned char buff[PIECESIZE];


	// open both files

	fd = fopen(piece_addr,"r");
	if (fd==NULL) 
	{
		fputs ("File error\n",stderr); 
		return -1;
	}
	ftorrent=fopen(torrent_addr,"r");
	if (ftorrent==NULL) 
	{
		fputs ("File error\n",stderr); 
		return -1;
	}

	// obtain file size
	fseek (fd , 0 , SEEK_END);
  	size = ftell (fd);
	if (size==PIECESIZE+1) size--;
  	rewind (fd);
	
	// verify hash code
	char hash_c[255];
	char hash_g[255];

	for (int i=0; i<(piece_num+4);i++)
	 	fgets(hash_c, 255, ftorrent);
	*strrchr(hash_c, '\n')='\0';
	fread(buff, 1, size, fd);
	gethash(buff,hash_g);
	
	fclose(fd);
	fclose(ftorrent);

	return !strcmp(hash_c,hash_g);
}

int verify_file(char* torrent_addr, char* file_addr){
	
	FILE* fd;
	FILE* ftorrent;
	long size;
	long piece_num;
	unsigned char buff[PIECESIZE];

	// open both files
    	ifstream read_torrent(torrent_addr);
	if(!read_torrent){
		fputs ("File error\n",stderr); 
		return -1;
	}

	fd = fopen(file_addr,"r");
	if (fd==NULL) 
	{
		fputs ("File error\n",stderr); 
		return -1;
	}

	// verify hash code
	char hash_c[255];
	char hash_g[255];

	read_torrent>>hash_c>>size>>piece_num;
	int i;
	for (i=0; i<(piece_num)&& !feof(fd);i++) {
	 	read_torrent>>hash_c;
		if (i<piece_num-1)
			fread(buff,1,PIECESIZE,fd);
		else
			fread(buff,1,size%PIECESIZE,fd);
		gethash(buff,hash_g);
		if (strcmp(hash_c,hash_g)){
			fclose(fd);
			read_torrent.close();
			return 0;
		}
	}

	
	fclose(fd);
	read_torrent.close();
	return (i==piece_num);
}

long get_file_size(char* file_name)
{
    FILE * fp = fopen(file_name,"r");
    if (fp == NULL)
    {
    	cout << "Cannot open file " << file_name << endl;
    	return -1;
    }
    fseek (fp, 0, SEEK_END);
    long file_size_long = ftell (fp);
    fclose(fp);

    return file_size_long;
}

Have_piece is_file_piece_exist(char* file_name, int piece_num)
{
	char piece_name[256];
	if (piece_num >= 0)
	{
		sprintf(piece_name,"%s_%d",file_name, piece_num);
	}
    DIR *dir;

    dir = opendir(".");
    struct dirent *ent;

    if (dir != NULL) {
    /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
        	char* name = ent->d_name;
        	if (0 == strcmp(name, piece_name))
            {
            	closedir (dir);
                return HAVE_PIECE;
            }
            else if (0 == strcmp(name, file_name)) {
            	closedir (dir);
                return HAVE_FILE;
            } 
        }
        closedir (dir);
    } else {
        /* could not open directory */
        cerr << "could not open location " << dir <<endl;
    }
    return NONE;
}

bool is_all_number(char* arr, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (arr[i] > '9' || arr[i] < '0')
		{
			return false;
		}
	}
	return true;
}

int check_prev_download(vector<bool> &completion_record, int &completion_counts, char* file_name)
{
	int file_name_len = strlen(file_name);

    DIR *dir;

    dir = opendir(".");
    struct dirent *ent;
    

    char prefix[30];
    char torrent_name[255];
    char piece_num_arr[20];
    int piece_num;
  
    strcpy(torrent_name,file_name);
    strcat(torrent_name,".torrent");

    if (dir != NULL) {
        while ((ent = readdir (dir)) != NULL) {
        	char* name = ent->d_name;
        	if (strcmp(name, file_name) == 0)
        	{
        		if (verify_file(torrent_name, file_name)) return -1;
        	}
        	if (strlen(name) < file_name_len + 2)
        	{
        		continue;
        	}
        	if (name[file_name_len] != '_')
        	{
        		continue;
        	}

        	bzero(prefix, 20);
        	memcpy(prefix, name, file_name_len);

        	if (strcmp(prefix, file_name) != 0)
        	{
        		continue;
        	}
    		int digit_len = strlen(name) - file_name_len - 1;
    		memcpy(piece_num_arr, name + file_name_len + 1, digit_len);
    		if (!is_all_number(piece_num_arr, digit_len))
    		{
    			continue;
    		}
    		piece_num = atoi(piece_num_arr);
    		if (piece_num > completion_record.size())
    		{
    			continue;
    		}
		if (verify_piece(torrent_name, name, piece_num)) {
	    		completion_record[piece_num] = true;
	    		completion_counts ++;
		}
        }
        closedir (dir);
    } else {
        /* could not open directory */
        cerr << "could not open location " << dir <<endl;
    }
    return 0;
}

