#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cstring>
#include "create_torrent.h"

#define pieceSize (4096*1024)

using namespace std;

void gethash(unsigned char* input,char* hash);

void create_torrent(char* file_name, char* tracker_name)
{
	FILE* fd;
	FILE* ftorrent;
	long size;
	unsigned char buff[pieceSize];		

	char torrent_name[256];
	strcpy(torrent_name,file_name);
	strcat(torrent_name,".torrent"); 

	fd = fopen(file_name,"r");
	if (fd==NULL) {fputs ("File error\n",stderr); exit (1);}
	ftorrent=fopen(torrent_name,"w");
	if (ftorrent==NULL) {fputs ("File error\n",stderr); exit (1);}

	// obtain file size
	fseek (fd , 0 , SEEK_END);
  	size = ftell (fd);
  	rewind (fd);


	long part_num = size / pieceSize + (size % pieceSize != 0);

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
		if (i==part_num-1 && (size % pieceSize))
			fread (buff,1,size % pieceSize, fd);
		else
			fread (buff,1, pieceSize,fd);
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
