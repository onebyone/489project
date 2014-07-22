#include "analyze_peerlist.h"
#include <stdio.h>  
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
using namespace std;

bool have_piece(string bitmap, int piece_num)
{
	int i,j;
	string binary_num;
	// piece_num start from 1
	i = piece_num / 4;
	j = piece_num % 4;
	char hex_num;
	if(j == 0) 
		hex_num = bitmap[i-1];
	else
		hex_num = bitmap[i];
	//cout<<hex_num<<endl;
	if(hex_num == '0')	binary_num = "0000";
	if(hex_num == '1')	binary_num = "0001";
	if(hex_num == '2')	binary_num = "0010";
	if(hex_num == '3')	binary_num = "0011";
	if(hex_num == '4')	binary_num = "0100";
	if(hex_num == '5')	binary_num = "0101";
	if(hex_num == '6')	binary_num = "0110";
	if(hex_num == '7')	binary_num = "0111";
	if(hex_num == '8')	binary_num = "1000";
	if(hex_num == '9')	binary_num = "1001";
	if(hex_num == 'A' || hex_num == 'a' )	binary_num = "1010";
	if(hex_num == 'B' || hex_num =='b'  )	binary_num = "1011";
	if(hex_num == 'C' || hex_num == 'c' )	binary_num = "1100";
	if(hex_num == 'D' || hex_num =='d'  )	binary_num = "1101";
	if(hex_num == 'E' || hex_num == 'e' )	binary_num = "1110";
	if(hex_num == 'F' || hex_num =='f'  )	binary_num = "1111";
	if(j == 0) 
	{
		if(binary_num[3]=='1') 
			return 1;
		else
			return 0;
	}
	else
	{
		if(binary_num[j-1]=='1') 
			return 1;
		else
			return 0;
	}
}

string choose_peer(int piece_num,  char* peerlist_file)
{
	ifstream file_in(peerlist_file);
	vector<string> ipvec;
	if(file_in == NULL)
    {
        cout << "Cannot open peer list file " << peerlist_file << endl;
        cout << "Exiting" << endl;
        exit(1);
    }

	string ip;
	string bitmap;
	while(file_in>>ip>>bitmap)
	{
		cout<<ip<<" "<<bitmap<<endl;
		if( have_piece( bitmap , piece_num ) )
		{
			ipvec.push_back(ip);
		}
	}
	if(ipvec.size() <= 0)
		return "";
	else
	{
		srand( (unsigned)time(NULL)); //生成种子
		int index;
		index = rand()%(ipvec.size()) ;
		return ipvec[index];
	}
}