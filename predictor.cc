/*Branch Predictor fuctionality*/

#include "predictor.h"

using namespace std;

Predictor::Predictor (int m, int n)
{
	index_bits = m;
	history_bits = n;
	table_rows = pow(2, index_bits);
	table = new int[table_rows];  //Branch predictor table

	predictions = mispredictions = 0;

	//30 bit 0x11.. Right shift for mask.
	PCmask = (1073741823 >> (30-index_bits) ); //For n=0.

	PCmask_gshare = (1073741823 >> ( 30 - (index_bits - history_bits))) ;		//For m-n bits in gshare
	Hmask = (1073741823 >> (30 - history_bits) );   //For n bits in gshare.
	initialize();
}

HPredictor::HPredictor (int k, int m1, int n, int m2)
{
	index_bits_gshare = m1;
	history_bits = n;
	index_bits_bimodal = m2;
	chooser_bits = k;
	
	table_rows_bimodal = pow(2, index_bits_bimodal);
	table_rows_gshare = pow(2, index_bits_gshare);
	table_rows_hybrid = pow(2, chooser_bits);

	table_bimodal = new int[table_rows_bimodal];  
	table_gshare = new int[table_rows_gshare];
	table_hybrid = new int[table_rows_hybrid];

	predictions = mispredictions = 0;

	//30 bit 0x11.. Right shift for mask.
	PCmask_bimodal = (1073741823 >> (30-index_bits_bimodal)); //For n=0.

	PCmask_gshare = (1073741823 >> ( 30 - (index_bits_gshare - history_bits)) );		//For m-n bits in gshare
	Hmask = (1073741823 >> (30 - history_bits) );   //For n bits in gshare.

	PCmask_hybrid = (1073741823 >> (30 - chooser_bits));
	Hinitialize();
}

void Predictor::Access_bimodal(unsigned int addr, const char *op)
{
	predictions++;
	int prediction;
	int actual;

	//cout << hex << addr << '\t' << op << endl;

	address = addr;
	index = (address >> 2) & PCmask;

    //cout << " BP: " << dec << index << "  " << table[index] << endl;

	//1 is taken, 0 is not-taken.
	if (table[index] > 1)
		prediction = 1;
	else if (table[index] < 2)
		prediction = 0;

	if ( *op == 'n')
		actual = 0;
	else if ( *op == 't')
		actual = 1;
    
    if ( prediction != actual)
    {
        mispredictions++;
    }

	if ( actual == 1 )
	{
		table[index] = table[index] + 1;
		if (table[index] > 3)
			table[index] = 3;
	}

	else if( actual == 0 )
	{
		table[index] = table[index] - 1;
		if (table[index] < 0)
			table[index] = 0;
	}
    
    //cout << " BU: " << dec << index << "  " << table[index] << endl;

}

void Predictor::Access_gshare(unsigned int addr, const char *op)
{
	predictions++;
	int prediction;
	int actual;
	unsigned int p =0;
	unsigned int x =0;

	//cout << hex << addr << '\t' << op << endl;
	//cout << "PCmask_gshare " << PCmask_gshare << " Hmask " << Hmask <<  endl;

	address = addr;

	p = history_table ^ ( (address >> ( 2 + index_bits - history_bits)) & Hmask );

	index = (p << (index_bits - history_bits)) + ( (address >> 2) & PCmask_gshare );

	//cout << " GP: " << dec << index << "  " << table[index] << endl;

	//1 is taken, 0 is not-taken.
	if (table[index] > 1)
		prediction = 1;
	else if (table[index] < 2)
		prediction = 0;

	if ( *op == 'n')
		actual = 0;
	else if ( *op == 't')
		actual = 1;
    
    if ( prediction != actual)
    {
        mispredictions++;
    }

    //Update branch prediction table.
	if ( actual == 1 )
	{
		table[index] = table[index] + 1;
		if (table[index] > 3)
			table[index] = 3;
	}

	else if( actual == 0 )
	{
		table[index] = table[index] - 1;
		if (table[index] < 0)
			table[index] = 0;
	}

	//Update global branch history register.
	//Make MSB equal to actual.
	x = history_table >> 1;
	history_table = (actual << (history_bits-1))  + x;

	//cout << " GU: " << dec << index << "  " << table[index] << endl;

}

void HPredictor::Access_hybrid(unsigned int addr, const char *op)
{
	predictions++;
	int actual;
	int prediction_bimodal;
	int prediction_gshare;
	int prediction_overall;
	int correct_bimodal;
	int correct_gshare;

	unsigned int p =0;
	unsigned int x =0;

	address = addr;

	if ( *op == 'n')
		actual = 0;
	else if ( *op == 't')
		actual = 1;
	
	//Get bimodal prediction
	//1 is taken, 0 is not-taken.
	index = (address >> 2) & PCmask_bimodal;

	if (table_bimodal[index] > 1)
		prediction_bimodal = 1;
	else if (table_bimodal[index] < 2)
		prediction_bimodal = 0;

	if ( prediction_bimodal == actual)
		correct_bimodal = 1;
	else correct_bimodal = 0;

	//Get gshare prediction
	p = history_table ^ ( (address >> ( 2+ index_bits_gshare - history_bits)) & Hmask );

	index = (p << (index_bits_gshare - history_bits)) + ( (address >> 2) & PCmask_gshare );

	if (table_gshare[index] > 1)
		prediction_gshare = 1;
	else if (table_gshare[index] < 2)
		prediction_gshare = 0;

	if ( prediction_gshare == actual)
		correct_gshare = 1;
	else correct_gshare = 0;


	//Obtain overall prediction
	index = (address >> 2) & PCmask_hybrid;

	if (table_hybrid[index] > 1)
		prediction_overall = prediction_gshare;
	else if (table_hybrid[index] < 2)
		prediction_overall = prediction_bimodal;

	if ( prediction_overall != actual)
		mispredictions++;

	//Update choosen branch predictor.
	if (table_hybrid[index] > 1)  //gshare
	{
		p = history_table ^ ( (address >> ( 2+ index_bits_gshare - history_bits)) & Hmask );
		index = (p << (index_bits_gshare - history_bits)) + ( (address >> 2) & PCmask_gshare );	
		if ( actual == 1 )
		{
			table_gshare[index] = table_gshare[index] + 1;
			if (table_gshare[index] > 3)
				table_gshare[index] = 3;
		}

		else if( actual == 0 )
		{
			table_gshare[index] = table_gshare[index] - 1;
			if (table_gshare[index] < 0)
			table_gshare[index] = 0;
		}

	}

	else if (table_hybrid[index] < 2)		//bimodal
	{
		index = (address >> 2) & PCmask_bimodal;

		if ( actual == 1 )
		{
		table_bimodal[index] = table_bimodal[index] + 1;
		if (table_bimodal[index] > 3)
			table_bimodal[index] = 3;
		}

		else if( actual == 0 )
		{
			table_bimodal[index] = table_bimodal[index] - 1;
			if (table_bimodal[index] < 0)
			table_bimodal[index] = 0;
		}

	}	

	//Update gshare global branch history register
	x = history_table >> 1;
	history_table = (actual << (history_bits-1))  + x;

	//Update branch chooser table
	index = (address >> 2) & PCmask_hybrid;

	if (( correct_gshare == 1 ) & ( correct_bimodal == 0))
	{
		table_hybrid[index] = table_hybrid[index] + 1;
		if ( table_hybrid[index] > 3 )
			table_hybrid[index] = 3;
	}
	else if (( correct_gshare == 0 ) & ( correct_bimodal == 1))
	{
		table_hybrid[index] = table_hybrid[index] - 1;
		if ( table_hybrid[index] < 0 )
			table_hybrid[index] = 0;
	}

}

//Make initial counter values = 2.
void Predictor::initialize()
{
	for(int i=0;i<table_rows;i++)
	{
		table[i] = 2;
	}

	//Branch history register init to 0.
	history_table = 0;
}

void HPredictor::Hinitialize()
{
	for(int i=0;i<table_rows_bimodal;i++)
	{
		table_bimodal[i] = 2;
	}

	for(int i=0;i<table_rows_gshare;i++)
	{
		table_gshare[i] = 2;
	}

	for(int i=0;i<table_rows_hybrid;i++)
	{
		table_hybrid[i] = 1;
	}

	//Branch history register init to 0.
	history_table = 0;
}

void Predictor::printStats(int c)
{
	float miss_rate = (float)((float)(100*mispredictions)/(float)(predictions));
	cout << "OUTPUT" << endl;
	cout << "number of predictions: " << predictions << endl;
	cout << "number of mispredictions: " << mispredictions << endl;
	cout.precision(2);
	cout.setf(ios::fixed, ios::floatfield);
	cout << "misprediction rate: " << miss_rate << "%" << endl;
	cout.unsetf(ios::floatfield);
	if ( c == 0)
		cout << "FINAL\t" << "BIMODAL CONTENTS" << endl;
	else if ( c == 1 )
		cout << "FINAL\t" << "GSHARE CONTENTS" << endl;

	for(int i=0;i<table_rows;i++)
	{
		cout << i << '\t' << table[i] << endl; 
	}
}

void HPredictor::HprintStats()
{
	float miss_rate;
	miss_rate = (float)((float)(100*mispredictions)/(float)(predictions));
	cout << "OUTPUT" << endl;
	cout << "number of predictions: " << predictions << endl;	
	cout << "number of mispredictions: " << mispredictions << endl;
	cout.precision(2);	
	cout.setf(ios::fixed, ios::floatfield);
	cout << "misprediction rate: " << miss_rate << "%" << endl;
	cout.unsetf(ios::floatfield);
	cout << "FINAL\t" << "CHOOSER CONTENTS" << endl;

	for(int i=0;i<table_rows_hybrid;i++)
	{
		cout << i << '\t' << table_hybrid[i] << endl; 
	}

	cout << "FINAL\t" << "GSHARE CONTENTS" << endl;
	for(int i=0;i<table_rows_gshare;i++)
	{
		cout << i << '\t' << table_gshare[i] << endl; 
	}

	cout << "FINAL\t" << "BIMODAL CONTENTS" << endl;
	for(int i=0;i<table_rows_bimodal;i++)
	{
		cout << i << '\t' << table_bimodal[i] << endl; 
	}
}
