#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"stdafx.h"
#include<cstring>

using namespace std;

int main(int argc, char *argv[]){

	FILE* fd;
	FILE* ftorrent;
	long size;
	char file_name[256];
	char tracker_name[256];
	unsigned char buff[4194304];		

	if (argc <3) {fputs ("Too few arguments",stderr); exit (1);}
	else {
		strcpy(file_name, argv[1]);
		strcpy(tracker_name,argv[2]);
	}


	char torrent_name[256];
	strcpy(torrent_name,file_name);
	strcat(torrent_name,".torrent"); 

	fd = fopen(file_name,"r");
	if (fd==NULL) {fputs ("File error\n",stderr); exit (1);}
	ftorrent=fopen(torrent_name,"w");
	if (ftorrent==NULL) {fputs ("File error2\n",stderr); exit (1);}

	// obtain file size
	fseek (fd , 0 , SEEK_END);
  	size = ftell (fd);
  	rewind (fd);


	long part_num = size / 4194303 + (size % 4194303 != 0);

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
		if (i==part_num-1 && (size % 4194303))
			fread (buff,1,size % 4194303, fd);
		else
			fread (buff,1,4194303,fd);
		char hash[255];
		gethash(buff,hash);
		char hash_str[256];
		sprintf(hash_str,"%s\n", hash);
		fwrite(hash_str, sizeof(char), sizeof(char)*strlen(hash_str), ftorrent);		
	}


	fwrite(tracker_name, sizeof(char), sizeof(char)*strlen(tracker_name), ftorrent);

	fclose(fd);
	fclose(ftorrent);

	return 0;

}


