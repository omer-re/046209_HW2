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

void *fee_collection_routine(void *theBank);

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

    bank Bank;
    int atmNum = atoi(argv[1]);
    // create the pthread objects
    vector <pthread_t> atmThreads(atmNum);
    vector <atmData> atmInfo(atmNum, atmData(Bank));
    pthread_t FeeCollectionThread, statusThread;

    //  Create a thread to each ATM
    for (int i = 0; i < atmNum; ++i) {
        atmInfo[i].atmNum = (i + 1);
        atmInfo[i].inFile = argv[i + 2];
        if (pthread_create(&atmThreads[i], NULL, atmRoutine, &atmInfo[i])) {
            perror("Error : ");
            return -4;
        }
    }

    //  create another threads for status and fee collection
    if (pthread_create(&FeeCollectionThread, NULL, fee_collection_routine, &Bank)
        || pthread_create(&statusThread, NULL, statusRoutine, &Bank)) {
        perror("Error : ");
        return -4;
    }

    for (int i = 0; i < atmNum; ++i)
        pthread_join(atmThreads[i], NULL);

    Bank._done = true;
    pthread_join(FeeCollectionThread, NULL);
    pthread_join(statusThread, NULL);


    //  arrives here once EOF commands
    return 0;


}


/**
 * fee_collection_routine runs by FeeCollectionThread,
 * it charges fee from all accounts every 3 seconds
 * @param theBank = ref to bank
 * @return
 */
void *fee_collection_routine(void *theBank) {    // routine to be run by the bank's commission thread
    bank *Bank = (bank *) theBank;
    // run until a done indication is received from the main thread
    while (!(Bank->_done)) {
        //Bank->getCommission();
		Bank->collect_fee();
        sleep(3);
    }

    return NULL;
}

/**
 * each atm thread runs the routine below
 * @param  atmNum - atmID for the current ATM
//         inFile - path to the commands file
//         theBank - reference to the system's bank object
 * @return - void
 */
void *atmRoutine(void *atmInfo) {    // routine to be run by each ATM
    atmData *info = (atmData *) atmInfo;
    // for each ATM, initialize atb object (with C'tor) for the required parameters
    atm Atm = atm(info->atmNum, info->inFile, info->theBank);
    // run while there are commands to be executed.
    //  executes a single command every T=100milisec
    while (Atm.execute_cmd())
        usleep(100000);
    return NULL;
}

/**
 * Runs with the status thread,
 * prints bank's snapshot to stdio every T=0.5secs
 * @param theBank - ref to bank
 * @return
 */
void *statusRoutine(void *theBank) {
    bank *Bank = (bank *) theBank;
    // runs until a done flag is received from the main thread
    while (!(Bank->_done)) {
        Bank->getStatus();
        usleep(500000);
    }

    return NULL;
}
