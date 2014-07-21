#include<stdio.h>
#include<stdlib.h>
#include<cstring>

#define pieceSize (4096*1024)

void gethash(unsigned char* input,char* hash);

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


