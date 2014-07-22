#ifndef __ANALYZE_PEERLIST_H_
#define __ANALYZE_PEERLIST_H_


#include <string>

using namespace std;

bool have_piece(string bitmap, int piece_num);

string choose_peer(int piece_num,  string peerlist_file);


#endif