#ifndef __FILE_OPS_H_
#define __FILE_OPS_H_

#include <stdlib.h>
#include <cstring>

void gethash(unsigned char* input,char* hash);
void create_torrent(char* file_name, char* tracker);
int combine_tmp(const char* file_name, long piece_num);
int verify_piece(char* torrent_addr, char* piece_addr, long piece_num);
long get_file_size(char* file_name);

#endif