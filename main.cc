//Main entry point function

#include "predictor.h"

using namespace std;

int main(int argc, char const *argv[])
{
	ifstream traceFile;
	char mystr[40];	
	char *p;
	string addressFull;
	int k;

	p = &mystr[0];
	
	int address;
	string status; //Denotes branch actually taken or not taken

	//Command line args
	string type;
	int index_bits;
	int history_bits;
	int chooser_bits;
	int index_bits_gshare;
	int index_bits_bimodal;
	string fileName;

	if (argc == 4)			//Bimodal
	{
		type = argv[1];
		index_bits = atoi(argv[2]);
		fileName = argv[3];	
		history_bits = 0;
		chooser_bits = 2;
		index_bits_gshare = 2;
		history_bits = 2;
		index_bits_bimodal = 2;
		k = 0;
	}

	else if (argc == 5)		//Gshare
	{
		type = argv[1];
		index_bits = atoi(argv[2]);
		history_bits = atoi(argv[3]);
		fileName = argv[4];
		chooser_bits = 2;
		index_bits_gshare = 2;
		index_bits_bimodal = 2;		
		k = 1;	
	}

	else if(argc == 7)		//Hybrid
	{
		type = argv[1];
		chooser_bits = atoi(argv[2]);
		index_bits_gshare = atoi(argv[3]);
		history_bits = atoi(argv[4]);
		index_bits_bimodal = atoi(argv[5]);
		fileName = argv[6];
		k = 2;
	}

	//Print Command
	cout << "COMMAND" << endl;
	if(argc == 4)
	{
		cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
	}

	else if (argc == 5)
	{
		cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
	}

	else if (argc == 7)
	{
		cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " " << argv[5] << " " << argv[6] << endl;	
	}

	//Instantiate object of Predictor class for bi, gshare
	Predictor pred(index_bits, history_bits);

	//Instantiate for hybrid predictor
	HPredictor hybrid(chooser_bits, index_bits_gshare, history_bits, index_bits_bimodal);

	//Open traceFile for reading
	strncpy(mystr, fileName.c_str(), fileName.length());
	mystr[fileName.length()]=0;

	traceFile.open(p);
	
	if ( (argc == 4) | ( (argc == 5) & (history_bits == 0) ) )
	{

	if(traceFile.is_open())
	{ 
		while(traceFile>>hex>>address>>status)
		{
			//Call bimodal
			//cout << "Address :" << address << " Branch: " << status << endl;
			pred.Access_bimodal(address, status.c_str());
		}
	}
	}	
	
	else if (argc == 5)
	{

	if(traceFile.is_open())
	{ 
		while (traceFile>>hex>>address>>status)
		{
			//Call gshare
			pred.Access_gshare(address, status.c_str());
		}
	}
	}	

	else if (argc == 7)
	{

	if(traceFile.is_open())
	{ 
		while (traceFile>>hex>>address>>status)
		{
			//Call hybrid
			hybrid.Access_hybrid(address, status.c_str());
		}
	}
	}

	traceFile.close();

	if ( argc != 7)
	pred.printStats(k);

	else hybrid.HprintStats();
	
	return 0;
}
