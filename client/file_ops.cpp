#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include "file_ops.h"

#define pieceSize (4096*1024)

using namespace std;

void gethash(unsigned char* input,char* hash);

int combine_tmp(const char* file_name, long piece_num){
	FILE* fd1;
	FILE* fd2;
	long size;
	char tmp_name[255];
	char buff[pieceSize];

	fd1 = fopen(file_name,"wb");
	if (fd1==NULL) {fputs ("File error\n",stderr); exit (1);}

	for (int i=0; i<piece_num;i++){
		sprintf(tmp_name,"%s_%d",file_name, i);

		fd2 = fopen(tmp_name,"rb");


		fseek (fd2 , 0 , SEEK_END);
  		size = ftell (fd2);
  		rewind (fd2);

		fread(buff, sizeof(char), size, fd2);

		fwrite(buff, sizeof(char), size, fd1);

		
		fclose(fd2);

		remove(tmp_name);
	}

	fclose(fd1);
	return 0;	
}

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

int verify_piece(char* torrent_addr, char* piece_addr, long piece_num){
	
	FILE* fd;
	FILE* ftorrent;
	long size;
	unsigned char buff[pieceSize];


	// open both files

	fd = fopen(piece_addr,"r");
	if (fd==NULL) {fputs ("File error\n",stderr); exit (1);}
	ftorrent=fopen(torrent_addr,"r");
	if (ftorrent==NULL) {fputs ("File error\n",stderr); exit (1);}

	// obtain file size
	fseek (fd , 0 , SEEK_END);
  	size = ftell (fd);
	if (size==pieceSize+1) size--;
  	rewind (fd);
	
	// verify hash code
	char hash_c[255];
	char hash_g[255];

	for (int i=0; i<(piece_num+3);i++)
	 	fgets(hash_c, 255, ftorrent);
	*strrchr(hash_c, '\n')='\0';
	fread(buff, 1, size, fd);
	gethash(buff,hash_g);
	
	fclose(fd);
	fclose(ftorrent);

	return !strcmp(hash_c,hash_g);
}

long get_file_size(char* file_name)
{
    FILE * fp = fopen(file_name,"r");
    fseek (fp, 0, SEEK_END);
    long file_size_long = ftell (fp);
    fclose(fp);

    return file_size_long;
}