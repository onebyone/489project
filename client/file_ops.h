#ifndef __FILE_OPS_H_
#define __FILE_OPS_H_

#include <stdlib.h>
#include <cstring>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <dirent.h>

using namespace std;

enum Have_piece { HAVE_FILE, HAVE_PIECE, NONE };

void gethash(unsigned char* input,char* hash);
int create_torrent(char* file_name, char* tracker);
int combine_tmp(const char* file_name, long piece_num);
int verify_piece(char* torrent_addr, char* piece_addr, long piece_num);
long get_file_size(char* file_name);
Have_piece is_file_piece_exist(char* file_name, int piece_num = -1);
int check_prev_download(vector<bool> &completion_record, int &completion_counts, char* file_name);

#endif
