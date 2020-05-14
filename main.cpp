//
// Created by os on 14.5.2020.
//
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <fstream>
#include <vector>
#include "account.h"
#include "bank.h"
#include "atm.h"

using namespace std;

void *atmRoutine(void *atmInfo);

void *commissionRoutine(void *theBank);

void *statusRoutine(void *theBank);

//********************************************
// Struct name: atmData
// Description: used for sending data to the atm threads.
// Parameters:  atmNum - ID of the ATM, theBank - reference to the system's bank object
//              inFile - path to the commands file for the ATM
//**************************************************************************************
typedef struct atmData {
    string atmNum;
    bank &theBank;
    string inFile;

    atmData(bank &Bank) : theBank(Bank) {} // atmData constructor, for initializing the bank's reference
} atmData;


int main(int argc, const char *argv[]) {

    // parsing the command
    if (argc < 2) {
        std::cout << "illegal arguments" << std::endl;
        return -1;
    }
    if (atoi(argv[1]) != argc - 2) //  ATM numbers should match the number of ATMs called in the initialization
    {
        std::cout << "illegal arguments" << std::endl;
        return -2;
    }

    // verify given file names exists
    for (int i = 2; i < argc; ++i) {
        std::ifstream in_file(argv[i]);
        if (!in_file) {
            std::cout << "illegal arguments" << std::endl;
            return -3;
        }
        in_file.close();
    }

    // in case init-input is valid - initiate bank






}