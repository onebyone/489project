#ifndef __CREATE_TORRENT_H_
#define __CREATE_TORRENT_H_

#include<string.h>
#include<cstring>

#define pieceSize (4096*1024)

using namespace std;

void gethash(unsigned char* input,char* hash);
void create_torrent(char* file_name, char* tracker);

#endif