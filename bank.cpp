//
// Created by os on 14.5.2020.
//

/*
 * "bank.cpp"
 * Implementation of the bank class
 * The bank manages all of the accounts in the system,
 * the bank is the only one that is allowed to access the accounts directly, and is required to ensure mutual exclusions.
 * The bank will execute all commands coming from the ATMs in the system.
 */
#include <unistd.h>
#include <math.h>
#include "bank.h"
#include "account.h"
#include "atm.h"
#include <iostream>
#include <string>
#include <string.h>

using namespace std;
ofstream _log;

/**
 * turn our input object into string.
 * @param our input
 * @return string value
 */

string convert_to_string(unsigned int value) {
    //create an output string stream
    ostringstream os;

    //throw the value into the string stream
    os << value;

    //convert the string stream into a string and return
    return os.str();
}

pthread_mutex_t logMutex;


/**
 * Checks if the given bank account id is valid
 * @param account_id
 * @return bool value
 */
bool bank::is_account_exists(unsigned int account_id) {
    lockMap("read");
    if ((_accounts.find(account_id)) == _accounts.end()) //means it didn't find the id
    {
        //free the lock
        unlockMap("read");

        // log the error
        return false;
    }
    // id was found
    unlockMap("read");

    return true;

}

/**
 * Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.
 * @param rw toggle for indicating read or write lock.
 */
void bank::lockMap(const string rw) {
    int result = rw.compare("read");
    if (result == 0) // requested lock is read lock, perform reader lock as in the algorithm
    {
        pthread_mutex_lock(&accountsReadlock);
        if (++_accountsReaders == 1)
            pthread_mutex_lock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);
    } else {  // requested lock is write lock, perform reader lock as in the algorithm
        pthread_mutex_lock(&accountsWritelock);

    }
}

/**
 * // Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.
 * @param rw
 */
void bank::unlockMap(std::string rw) {
    //we assume that we are calling this func after one of the locks has been called, and that we are still dealing with the same flag (read or write)
    int result = rw.compare("read");
    if (result == 0) {   // need to free the read lock

        pthread_mutex_lock(&accountsReadlock);
        if (--_accountsReaders == 0)
            pthread_mutex_unlock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);
    } else {  // need to free the write lock
        pthread_mutex_unlock(&accountsWritelock);
    }
}


/**
 * Create new account after validating for duplications
 * @param acntNum
 * @param initBalance
 * @param pass
 * @param atmID
 */
void bank::create_account(unsigned int acntNum, int initBalance, std::string pass, std::string atmID) {

    //  we are not expecting this case as defined by instructions.
    /** verify valid 4 digits password
	//std::string::const_iterator it = pass.begin();
	//while (it != pass.end() && std::isdigit(*it)) ++it;
	//if (it != pass.end() || pass.length() != 4)
	//	return; **/

    // password is valid
    lockMap("write");

    if (_accounts.find(acntNum) != _accounts.end())  // means this number is already exists
    {
        log("Error " + atmID + ": Your transaction failed - account with the same id exists");
    } else {
        _accounts.insert(pair<unsigned int, account>(acntNum, account(acntNum, initBalance, pass, atmID)));
        // success message
        log(atmID + ": New account id is " + convert_to_string(acntNum) + " with password " + pass +
            " and initial balance " +
            convert_to_string(initBalance));
        //actions take 1 second to perform by definition

    }
    sleep(1);

    unlockMap("write");

}

/**
 * Deposite to account as requested by an ATM
 * @param acntNum
 * @param pass
 * @param amount
 * @param atmID - requesting atm id
 */
void bank::deposit(unsigned int acntNum, string pass, unsigned int amount, string atmID) {
    lockMap("read");

    if ((_accounts.find(acntNum)) == _accounts.end()) //means it didn't find the id
    {
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(acntNum) +
            " does not exist");

        sleep(1); //actions take 1 second to perform by definition
        // success message

        unlockMap("read");

        return;
    }

    // verify password
    if (!_accounts.find(acntNum)->second.check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");

        sleep(1); //actions take 1 second to perform by definition
        // success message

        unlockMap("read");

        return;
    }
    // if we got here- all details are correct.
    //we request the lock
    _accounts.find(acntNum)->second.lock("write");
    _accounts.find(acntNum)->second.deposit(amount);

    sleep(1); //actions take 1 second to perform by definition
    // success message

    // log action success and free the lock.
    unsigned int newbalance = _accounts.find(acntNum)->second.getBalance();
    log(atmID + ": Account " + convert_to_string(acntNum) + " new balance is " + convert_to_string(newbalance) +
        " after " +
        convert_to_string(amount) + " $ was deposited");

    _accounts.find(acntNum)->second.unlock("write");
    unlockMap("read");
}


/**
 *  withdrawal from account as requested by an ATM
 * @param amount_of_money
 */
void bank::withdrawal(unsigned int acntNum, string pass, unsigned int amount, string atmID) {

    lockMap("read");
    if ((_accounts.find(acntNum)) == _accounts.end()) //means it didn't find the id
    {
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(acntNum) +
            " does not exist");
        // log the error and return


        sleep(1); //actions take 1 second to perform by definition
        // success message
        unlockMap("read");

        return;
    }
    // verify password
    if (!_accounts.find(acntNum)->second.check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");

        sleep(1); //actions take 1 second to perform by definition
        // success message
        unlockMap("read");

        return;
    }
    // if we got here- all details are correct.
    //we request the lock
    _accounts.find(acntNum)->second.lock("write");

    // make sure it has enough money in the account
    bool enough_money = false;
    enough_money = _accounts.find(acntNum)->second.withdrawal(amount);
    sleep(1); //actions take 1 second to perform by definition
    if (!enough_money) {
        // log action failure, free the lock.
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(acntNum) +
            " balance is lower than " + convert_to_string(amount));
        _accounts.find(acntNum)->second.unlock("write");

        unlockMap("read");

        return;
    }
    // success message
    // log action success and free the lock.
    unsigned int newbalance = _accounts.find(acntNum)->second.getBalance();
    log(atmID + ": Account " + convert_to_string(acntNum) + " new balance is " + convert_to_string(newbalance) +
        " after " +
        convert_to_string(amount) + " $ was withdrew");
    _accounts.find(acntNum)->second.unlock("write");


    unlockMap("read");


}

/**
 *  transfer money between accounts
 * @param source_account_id
 * @param source_account__pass
 * @param dest_account_id
 * @param amount_of_money
 * @return
 */
int bank::transfer_money(unsigned int source_account_id, string source_account__pass, unsigned int dest_account_id,
                         unsigned int amount_of_money, string atmID) {

    lockMap("write");

    // check source account exists
    if ((_accounts.find(source_account_id)) == _accounts.end()) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed  – account id " + convert_to_string(source_account_id) +
            " does not exist");
        sleep(1);
        unlockMap("write");

        return -1;
    }
    // check source account password
    if (!_accounts.find(source_account_id)->second.check_password(source_account__pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " +
            convert_to_string(source_account_id) +
            " is incorrect");
        sleep(1);
        unlockMap("write");

        return -1;
    }

    // check dest account exists
    if ((_accounts.find(dest_account_id)) == _accounts.end()) {        // log the error and return
        log("Error " + atmID + ": Your transaction failed – account id " + convert_to_string(dest_account_id) +
            " does not exist");
        sleep(1);
        unlockMap("write");

        return -1;
    }
    // dest account password isn't checked- as instructed

    // check source!=dest
    if (source_account_id == dest_account_id) {
        sleep(1);
        unlockMap("write");
        //cout << "trans end" << endl;

        return 0; // must do nothing, otherwise we will get into a deadlock
    }
    // lock src and dest, in rising order to prevent deadlock
    _accounts.find(source_account_id)->second.lock("write");
    _accounts.find(dest_account_id)->second.lock("write");
    // check source has enough money
    bool enough_money = false;
    enough_money = _accounts.find(source_account_id)->second.withdrawal(amount_of_money);


    sleep(1); //actions take 1 second to perform by definition
    if (!enough_money) {
        // log action failure, free the lock.
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(source_account_id) +
            " balance is lower than " + convert_to_string(amount_of_money));
        _accounts.find(dest_account_id)->second.unlock("write");

        _accounts.find(source_account_id)->second.unlock("write");

        unlockMap("write");
        return -1;

    } else {
        // deposit to dest account. log the transfer success.
        _accounts.find(dest_account_id)->second.deposit(amount_of_money);
        unsigned int source_balance = _accounts.find(source_account_id)->second.getBalance();
        unsigned int dest_balance = _accounts.find(dest_account_id)->second.getBalance();
        log(atmID + ": Transfer " + convert_to_string(amount_of_money) + " from account " +
            convert_to_string(source_account_id) +
            " to account " + convert_to_string(dest_account_id) + " new account balance is " +
            convert_to_string(source_balance) +
            " new target account balance is " + convert_to_string(dest_balance));
    }

    // open locks
    _accounts.find(dest_account_id)->second.unlock("write");
    _accounts.find(source_account_id)->second.unlock("write");


    unlockMap("write");

    return 0;
}

/**
 * Print full bank status to standard output
 */
void bank::getStatus() {// Print full bank status to standard output.
    //  request the write lock for the accounts map
    //  request read lock for each account separately to ensure valid value
    //  we want to get the status off all the accounts at a certain point, and therefore have to
    //  first get the locks of all and not go one by one and release each immediately.

    lockMap("write");

    std::map<unsigned int, account>::iterator it;
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {
        it->second.lock("read");
    }
    pthread_mutex_lock(&balanceLock);
    // clear screen and print status as instructed
    printf("\033[2J");
    printf("\033[1;1H");
    // print status
    printf("Current Bank Status\n");
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {
        it->second.account_print();
    }
    printf(".\n.\n");
    printf("The Bank has %d $\n", _bankBalance);
    // free all of the locks.
    pthread_mutex_unlock(&balanceLock);
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {
        it->second.unlock("read");
    }

    unlockMap("write");

}

/**
 * collect random fee (2-4%) every 3 seconds
 * @return
 */
void bank::collect_fee() {
    double commission = (rand() % 3 + 2) / 100.0;
    unsigned int profit, total_profit = 0;

    lockMap("write"); // no new accounts are made while running fee collection


    std::map<unsigned int, account>::iterator it;
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {
        it->second.lock("write");

        profit = (unsigned int) round(it->second.getBalance() * commission);
        it->second.withdrawal(profit);
        total_profit += profit;
        // log commissions taken from the account.
        log("Bank: commissions of " + convert_to_string((int) (commission * 100)) + " % were charged, the bank gained "
            + convert_to_string(profit) + " $ from account " + convert_to_string(it->first));

        //   it->second.unlock("write");
    }
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {

        it->second.unlock("write");
    }

    unlockMap("write");

    // update the bank's balance.
    // only 1 thread writes to this, and 1 thread reads, so standard lock is fine.
    pthread_mutex_lock(&balanceLock);
    _bankBalance += total_profit;
    pthread_mutex_unlock(&balanceLock);

}


void bank::delete_account(unsigned int acntNum, string pass, string atmID) {
    //  check account exists

    lockMap("write");
    if (_accounts.find(acntNum) == _accounts.end()) {  // means this number is already exists
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(acntNum) +
            " does not exist");
        sleep(1);

        unlockMap("write");

        return;
    }
    std::map<unsigned int, account>::iterator it_currently_handled_account;
    it_currently_handled_account = _accounts.find(acntNum);
    int p1 = atoi(pass.c_str());
    int p2 = atoi(_accounts.find(acntNum)->second.getpass().c_str());
    if (p1 != p2) {
        // verify password
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");
        sleep(1);

        unlockMap("write");

        return;
    }

    // if we got here- all details are correct.
    // prevent others accessing the account

    it_currently_handled_account->second.lock("write");
    sleep(1);

    //  save last balance value for logging it
    unsigned int last_balance = it_currently_handled_account->second.getBalance();

    //  call account's d'tor
    it_currently_handled_account->second.~account();

    _accounts.erase(acntNum);

    // log action success and free the lock.
    log(atmID + ": Account " + convert_to_string(acntNum) + " is now closed. Balance was " +
        convert_to_string(last_balance));
    unlockMap("write");

}


/**
 * wrapper func that writes the string given to the bank's log
 * @param tolog
 */
void bank::log(std::string tolog) {
    pthread_mutex_lock(&loglock);
    _log << tolog << std::endl;
    pthread_mutex_unlock(&loglock);
}

void bank::check_balance(unsigned int acntNum, string pass, string atmID) {


    lockMap("read");


    if ((_accounts.find(acntNum)) == _accounts.end()) {//means it didn't find the id
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(acntNum) +
            " does not exist");

        sleep(1);
        unlockMap("read");


        return;
    }

    int p1 = atoi(pass.c_str());
    int p2 = atoi(_accounts.find(acntNum)->second.getpass().c_str());
//	if (!_accounts.find(acntNum)->second.check_password(pass)) {
    if (p1 != p2) {

        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");

        sleep(1);

        // success message

        unlockMap("read");

        return;
    }

    _accounts.find(acntNum)->second.lock("read");
    sleep(1);

    unsigned int bal = _accounts.find(acntNum)->second.getBalance();
    log(atmID + ": Account " + convert_to_string(acntNum) + " balance is: " + convert_to_string(bal));

    _accounts.find(acntNum)->second.unlock("read");
    unlockMap("read");


    return;
}

// 4.06 15.00 removed comments