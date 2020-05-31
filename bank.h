/*
 * "bank.h"
 * Declaration of the bank class
 * The bank manages all of the accounts in the system,
 * only the bank is allowed to directly perform actions on the accounts, and is required to ensure mutual exclusions.
 * The bank will execute all commands coming from the ATMs in the system.
 * The bank is also in charge of all logging done by the system.
 */

#ifndef HW2_BANK_H
#define HW2_BANK_H
#include <string>
#include <string.h>
#include <pthread.h>
#include <map>
#include <fstream>
#include <sstream>
#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;



class bank {

private:
    pthread_mutex_t loglock, balanceLock, accountsWritelock, accountsReadlock; // accountsWritelock - for limiting when you can create new accounts
    unsigned int _accountsReaders;
    unsigned int _bankBalance;
    map<unsigned int, account> _accounts;
    ofstream _log;


public:
    //c'tor + initialize mutex + open log file
    bank() : _bankBalance(0), _accountsReaders(0), _done(false) {
        _log.open("log.txt");
        if (pthread_mutex_init(&balanceLock, NULL) || pthread_mutex_init(&accountsWritelock, NULL)
            || pthread_mutex_init(&accountsReadlock, NULL) ||
            pthread_mutex_init(&loglock, NULL)) {   // init allocates memory, need to make sure sys call didnt fail
            perror("system call error:");
            exit(1);
        }
    }

    // d'tor + close file + destroy locks
    ~bank() {
        pthread_mutex_destroy(&balanceLock);
        pthread_mutex_destroy(&accountsReadlock);
        pthread_mutex_destroy(&accountsWritelock);
        pthread_mutex_destroy(&loglock);
        _log.close();
    }

    void create_account(unsigned int acntNum, unsigned int initBalance, string pass, string atmId);

    void delete_account(unsigned int acntNum, string pass, string atmID);

    bool is_account_exists(unsigned int account_id);

    bool check_enough_balance(unsigned int account_id, unsigned int amount_of_money);

    // exists on accounts
    void check_balance(unsigned int account_id, string pass, string atmId);

    void deposit(unsigned int acntNum, string pass, unsigned int amount,
                 std::string atmID); // Deposite to account as requested by an ATM
    void withdrawal(unsigned int acntNum, string pass, unsigned int amount,
                    string atmID);  // Deposite to account as requested by an ATM

    int transfer_money(unsigned int source_account_id, std::string source_account_pass, unsigned int dest_account_id,
                       unsigned int amount_of_money, string atmId);

    void
    lockMap(string rw); // Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.
    void unlockMap(
            string rw); // Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.

    bool _done;

    void getStatus(); // Print full bank status to standard output.
    void collect_fee();

    void print_status();

    void log(std::string tolog);


};


#endif //HW2_BANK_H
