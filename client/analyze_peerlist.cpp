#include "analyze_peerlist.h"

using namespace std;

bool have_piece(string bitmap, int piece_num)
{
	int i,j;
	string binary_num;
	// piece_num start from 1
	i = piece_num / 4;
	j = piece_num % 4;
	char hex_num = bitmap[i];

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


	if(binary_num[j]=='1') 
		return 1;
	else
		return 0;


}

string choose_peer(int piece_num,  char* peerlist_file, vector<ip_struct> &active_ip)
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
		if( have_piece( bitmap , piece_num ) && ifactive(active_ip,ip))
		{
			ipvec.push_back(ip);
		}
	}
	file_in.close();
	if(ipvec.size() <= 0)
		return "";
	else
	{
		srand( (unsigned)time(NULL));
		int index;
		index = rand()%(ipvec.size()) ;
		return ipvec[index];
	}
}

void add_ip_from_peerlist(vector<ip_struct> &active_ip, char* peerlist_file){

	ifstream file_in(peerlist_file);
	string ip;
	string bitmap;
	while(file_in>>ip>>bitmap)
	{
		int i=0;
		while( active_ip.size() != i )
		{
			string tempip = active_ip[i].ip ;
			if(tempip == ip) return;
			i++;
		}
		if (i==active_ip.size()) {
			struct ip_struct entry;
			entry.ip=ip;
			entry.connect=2;
			active_ip.push_back(entry);
		}
	}
	file_in.close();
}

int remove_ip(vector<ip_struct> &active_ip, string ip)
{
	int i=0;
	while( active_ip.size() != i )
	{
		string tempip = active_ip[i].ip ;
		if(tempip == ip) {
			return --active_ip[i].connect;
		}
		i++;
	}
	return 0;
}

bool ifactive(vector<ip_struct> &active_ip, string ip)
{
	int i=0;
	while( active_ip.size() != i )
	{
		string tempip = active_ip[i].ip;
		if(tempip == ip)
			return active_ip[i].connect>0;
		i++;
	}
	return false;
}


int generate_rand_array(int size, int* array){
	for (int i=0;i<size;i++) array[i]=i;
	srand((unsigned)time(NULL));
	for (int i=0;i<size*size;i++) {
		int j=rand()%size;
		int k=rand()%size;
		int tmp=array[j];
		array[j]=array[k];
		array[k]=tmp;
	}
	return 0;
}
