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

#include <pthread.h>
#include <map>
#include <fstream>
#include <sstream>
#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


private:
pthread_mutex_t loglock, balanceLock, accountsWritelock, accountsReadlock; // accountsWritelock - for limiting when you can create new accounts
unsigned int _bankBalance, _accountsReaders;
std::map<unsigned int, account> _accounts;
std::ofstream _log;


class bank : {

private:
    pthread_mutex_t loglock, balanceLock, accountsWritelock, accountsReadlock; // accountsWritelock - for limiting when you can create new accounts
    unsigned int _bankBalance, _accountsReaders;
    std::map<unsigned int, account> _accounts;
    std::ofstream _log;


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

    void create_account();

    bool is_account_exists(unsigned int account_id);

    bool check_enough_balance(unsigned int account_id, unsigned int amount_of_money);

    int collect_fee();

    void print_status();

    int transfer_money(unsigned int account_id1, unsigned int id1_pass, unsigned int account_id2, unsigned int id2_pass,
                       unsigned int amount_of_money);

    void
    lockMap(std::string rw); // Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.
    void unlockMap(
            std::string rw); // Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.
    bool _done;
};


#endif //HW2_BANK_H
