#include<stdio.h>
#include<stdlib.h>
#include<cstring>

#define pieceSize (4096*1024)

int combine_tmp(char* file_name, long piece_num){
	FILE* fd1;
	FILE* fd2;
	long size;
	char tmp_name[255];
	unsigned char buff[pieceSize+1];

	fd1 = fopen(file_name,"w");
	if (fd1==NULL) {fputs ("File error\n",stderr); exit (1);}

	for (int i=1; i<(piece_num+1);i++){
		sprintf(tmp_name,"%s_%d",file_name, i);
		fd2 = fopen(tmp_name,"r");

		fseek (fd2 , 0 , SEEK_END);
  		size = ftell (fd2);
  		rewind (fd2);

		fread(buff, 1, size, fd2);
		if (i<piece_num) *(buff+size-1)='\0';
		fwrite(buff, 1, size-1, fd1);
		
		fclose(fd2);
		remove(tmp_name);
	}

	fclose(fd1);
	return 0;	
}
