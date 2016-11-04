/*Header file*/

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <math.h>
#include <fstream>

class Predictor {
public:
	unsigned int address;
	unsigned int index;
	int index_bits;
	int history_bits;
	unsigned int history_table;

	int table_rows;
	int *table;
	unsigned int PCmask;		//For bimodal
	unsigned int PCmask_gshare;
	unsigned int Hmask;			//For gshare

	int predictions;
	int mispredictions;

Predictor (int, int);
void Access_bimodal(unsigned int address, const char *op);
void Access_gshare(unsigned int address, const char *op);

void printStats(int);
void initialize();
};

class HPredictor {
public:
	unsigned int address;
	unsigned int index;
	int index_bits_bimodal;
	int index_bits_gshare;
	int history_bits;
	int chooser_bits;
	unsigned int history_table;

	int table_rows_bimodal;
	int table_rows_gshare;
	int table_rows_hybrid;

	int *table_bimodal;
	int *table_gshare;
	int *table_hybrid;

	unsigned int PCmask_bimodal;		//For bimodal
	unsigned int PCmask_gshare;
	unsigned int PCmask_hybrid;
	unsigned int Hmask;			//For gshare

	int predictions;
	int mispredictions;

HPredictor (int, int, int, int);
void Access_hybrid(unsigned int address, const char *op);

void HprintStats();
void Hinitialize();
};

