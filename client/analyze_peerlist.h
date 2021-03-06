#ifndef __ANALYZE_PEERLIST_H_
#define __ANALYZE_PEERLIST_H_


#include <string>
#include <vector>
#include <stdio.h>  
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

struct ip_struct {
	string ip;
	int connect;
};

bool have_piece(string bitmap, int piece_num);

string choose_peer(int piece_num,  char* peerlist_file,vector<ip_struct>  &active_ip);

bool ifactive(vector<ip_struct> &active_ip, string ip);

void add_ip_from_peerlist(vector<ip_struct> &active_ip, char* peerlist_file);

int remove_ip(vector<ip_struct> &active_ip, string ip);

int generate_rand_array(int size, int* array);

#endif
