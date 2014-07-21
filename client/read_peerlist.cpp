#include <stdio.h>  
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
using namespace std;

bool have_piece(string bitmap, int piece_num);
string choose_peer(int piece_num,  string peerlist_file);

int main()  
{  
    char   szValue[]  =   "11";    
char ch[32]; 
    int    nValude    =   0;         
    sscanf(szValue,"%x",&nValude);    //十六进制转数字 
sprintf(ch,"%d",nValude); //数字转字符
    // printf("%d/n",nValude);  
    // cout<<endl;
    // cout<<ch<<endl;
//string shit = choose_peer(2, "peerlist.txt");
string shit = choose_peer(4, "peerlist.txt");
    //cout<<"ans is "<<shit<<endl;
	vector<int> v;
	v.push_back(1);
	cout<<v.size()<<endl;
	int temp = v.back();
	v.pop_back();
	srand( (unsigned)time(NULL)); //生成种子
	int gaoshenme;
	gaoshenme = rand()%8;
	cout<<"random number is"<<gaoshenme<<endl;
	cout<<"size is "<<v.size()<<endl;
	cout<<"num is "<< temp<<endl;


    return 0;     
} 

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

string choose_peer(int piece_num,  string peerlist_file)
{
//	ifstream file_in(peerlist_file);
	ifstream file_in(peerlist_file.c_str());
	vector<string> ipvec;
	if(file_in == NULL)
		cout<<"No File Found"<<endl;
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
		return "No Piece Found";
	else
	{
		srand( (unsigned)time(NULL)); //生成种子
		int index;
		index = rand()%(ipvec.size()) ;
		return ipvec[index];
	}
}

//small piece
// else if (strcmp(command, "request_p") == 0)
//  {
//      cout<<"which file are you going to download?";
//      string file_name;
//      cin>>file_name;
//      cout<<"Track from peerlist"<<endl; 
//      cout<<"Input peerlist file name: ";
//      cin >> peerlist_file;  
//      vector<int> notfoundlist;
//      for(int i=1 ;  i<=piece_size  ; i++)      //TODO : last piece size
//      {
//         string  clientAddress = choose_peer( i,  peerlist_file);
//         if(clientAddress != "No Piece Found")
//         {
//                 cout<<"receiving piece "<<i<<" from "<<clientAddress<<endl;
//                 job1(command,clientAddress,file_name,piece_size);    
//         }
//         else
//         {
//             notfoundlist.push_back(i);
//         }
//      }

//      for(int i=notfoundlist.back() ;  notfoundlist.size() != 0  ; i=notfoundlist.back())      //wait notfound piece
//      {
//         string  clientAddress = choose_peer( i,  peerlist_file);
//         if(clientAddress != "No Piece Found")
//         {
//                 cout<<"receiving piece "<<i<<" from "<<clientAddress<<endl;
//                 notfoundlist.pop_back();
//                 job1(command,clientAddress);    
//         }
//      }
    
// }