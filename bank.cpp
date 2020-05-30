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
#include <sstream>
#include <string.h>

using namespace std;
ofstream _log;

/**
 * turn our input object into string.
 * @param our input
 * @return string value
 */

string convert_to_string(int value) {
    //create an output string stream
    ostringstream os;

    //throw the value into the string stream
    os << value;

    //convert the string stream into a string and return
    return os.str();

    //pthread_mutex_t logMutex;
}

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
        cout << " lock r" << endl;  // TODO: remove

        pthread_mutex_lock(&accountsReadlock);
        if (++_accountsReaders == 1)  //TODO: should it be "while"?
            pthread_mutex_lock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);
    }
        // write
    else {  // requested lock is write lock, perform reader lock as in the algorithm
        pthread_mutex_lock(&accountsReadlock);
        if (++_accountsReaders == 1)   //TODO: should it be "while"?
            pthread_mutex_lock(&accountsWritelock)
        pthread_mutex_unlock(&accountsReadlock);  // TODO: not sure why it's here

        cout << " lock w" << endl; // TODO: remove

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
        cout << " unlock r" << endl;

        pthread_mutex_lock(&accountsReadlock);
        if (--_accountsReaders == 0)
            pthread_mutex_unlock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);
    } else {  // need to free the write lock
        cout << "unlock w" << endl;
        pthread_mutex_unlock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);

    }
}


/**
 * Create new account after validating for duplications
 * @param acntNum
 * @param initBalance
 * @param pass
 * @param atmID
 */
void bank::create_account(unsigned int acntNum, unsigned int initBalance, std::string pass, std::string atmID) {
    // verify valid 4 digits password
    std::string::const_iterator it = pass.begin();
    while (it != pass.end() && std::isdigit(*it)) ++it;
    if (it != pass.end() || pass.length() != 4)
        return;  //  we are not expecting this case as defined by instructions.

    // password is valid
    lockMap("read");
    lockMap("write");
    unlockMap("read");
/**
	if (pthread_mutex_lock(&logMutex) != 0) {
		perror("");
		exit(1);
	}**/
    if (_accounts.find(acntNum) != _accounts.end())  // means this number is already exists
    {
        log("Error " + atmID + ": Your transaction failed - account with the same id exists");
    } else {
        _accounts.insert(pair<unsigned int, account>(acntNum, account(acntNum, initBalance, pass, atmID)));
        sleep(1); //actions take 1 second to perform by definition
        // success message
        log(atmID + ": New account id is " + convert_to_string(acntNum) + " with password " + pass +
            " and initial balance " +
            convert_to_string(initBalance));
    }

    unlockMap("write");
    /**if (pthread_mutex_unlock(&logMutex) != 0) {
        perror("");
        exit(1);
}**/
}

/**
 * Deposite to account as requested by an ATM
 * @param acntNum
 * @param pass
 * @param amount
 * @param atmID - requesting atm id
 */
void bank::deposit(unsigned int acntNum, string pass, unsigned int amount, string atmID) {
    // TODO prevent someone delete my account after I search for it
    cout << "deposit start" << endl;  //TODO remove
    lockMap("read");
    lockMap("write");
    unlockMap("read");    //pthread_mutex_lock(&logMutex);
    if (!is_account_exists(acntNum)) {  //TODO: maybe remove?
        // log the error and return
        //if((_accounts.find(acntNum)) == _accounts.end()) //means it didn't find the id
        log("Error " + atmID + ": Your transaction failed - account id " + to_string(acntNum) +
            " does not exist");

        sleep(1); //actions take 1 second to perform by definition
        // success message
        cout << "deposit end" << endl;  // TODO remove
        unlockMap("write");
        return;
    }
    // verify password
    if (!_accounts.find(acntNum)->second.check_password(pass)) {
        // log bad pass, and return.
        int account_num = acntNum;

        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(account_num) +
            " is incorrect");

        sleep(1); //actions take 1 second to perform by definition
        // success message
        cout << "deposit end" << endl;  // TODO remove
        unlockMap("write");
        return;
    }
    // if we got here- all details are correct.
    //we request the lock
    _accounts.find(acntNum)->second.lock("write");
    _accounts.find(acntNum)->second.deposit(amount);

    sleep(1); //actions take 1 second to perform by definition
    // success message
    cout << "deposit end" << endl;  // TODO remove

    //pthread_mutex_unlock(&logMutex);

    // log action success and free the lock.
    unsigned int newbalance = _accounts.find(acntNum)->second.getBalance();

    log(atmID + ": Account " + convert_to_string(acntNum) + " new balance is " + convert_to_string(newbalance) +
        " after " +
        convert_to_string(amount) + " $ was deposited");
    _accounts.find(acntNum)->second.unlock("write");
    unlockMap("write");

}


/**
 *  withdrawal from account as requested by an ATM
 * @param amount_of_money
 */
void bank::withdrawal(unsigned int acntNum, string pass, unsigned int amount, string atmID) {
    cout << "withdrawal start" << endl;  //TODO remove
    lockMap("read");
    lockMap("write");
    unlockMap("read");

    if (!is_account_exists(acntNum)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(acntNum) +
            " does not exist");

        cout << "withdrawal end" << endl;  // TODO remove
        sleep(1); //actions take 1 second to perform by definition
        // success message
        unlockMap("write");
        return;
    }
    // verify password
    if (!_accounts.find(acntNum)->second.check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");
        cout << "withdrawal end" << endl;  // TODO remove
        sleep(1); //actions take 1 second to perform by definition
        // success message
        unlockMap("write");
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
        cout << "withdrawal end" << endl;  // TODO remove
        unlockMap("write");
        return;
    }
    // success message
    // log action success and free the lock.
    unsigned int newbalance = _accounts.find(acntNum)->second.getBalance();
    log(atmID + ": Account " + convert_to_string(acntNum) + " new balance is " + convert_to_string(newbalance) +
        " after " +
        convert_to_string(amount) + " $ was withdrew");
    _accounts.find(acntNum)->second.unlock("write");
    cout << "withdrawal end" << endl;  // TODO remove
    unlockMap("write");
}

/**
 *  transfer money between accounts
 * @param source_account_id
 * @param source_account__pass
 * @param dest_account_id
 * @param amount_of_money
 * @return
 */
int bank::transfer_money(unsigned int source_account_id, string source_account_pass, unsigned int dest_account_id,
                         unsigned int amount_of_money, string atmID) {
    cout << "trans start" << endl;  // TODO remove
    lockMap("read");
    lockMap("write");
    unlockMap("read");
    // check source account exists
    if (!is_account_exists(source_account_id)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed  – account id " + convert_to_string(source_account_id) +
            " does not exist");
        return -1;
    }
    // check source account password
    if (!_accounts.find(source_account_id)->second.check_password(source_account_pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " +
            convert_to_string(source_account_id) +
            " is incorrect");
        cout << "trans end" << endl;  // TODO remove

        unlockMap("write");
        return -1;
    }

    // check dest account exists
    if (!is_account_exists(dest_account_id)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed – account id " + convert_to_string(dest_account_id) +
            " does not exist");
        cout << "trans end" << endl;  // TODO remove

        unlockMap("write");
        return -1;
    }
    // dest account password isn't checked- as instructed

    // check source!=dest
    if (source_account_id == dest_account_id) {
        unlockMap("write");
        return 0; // must do nothing, otherwise we will get into a deadlock
    }

    // lock src and dest, in rising order to prevent deadlock
    // _accounts.find(std::min(source_account_id, dest_account_id))->second.lock("write");
    // _accounts.find(std::max(source_account_id, dest_account_id))->second.lock("write");
    _accounts.find(source_account_id)->second.lock("write");
    _accounts.find(dest_account_id)->second.lock("write");
    // check source has enough money
    // initialize to false and check
    bool enough_money = false;
    enough_money = _accounts.find(source_account_id)->second.withdrawal(amount_of_money);


    sleep(1); //actions take 1 second to perform by definition
    if (!enough_money) {
        // log action failure, free the lock.
        log("Error " + atmID + ": Your transaction failed - account id " + convert_to_string(source_account_id) +
            " balance is lower than " + convert_to_string(amount_of_money));
        _accounts.find(dest_account_id)->second.unlock("write");
        _accounts.find(source_account_id)->second.unlock("write");
        cout << "trans end" << endl;  // TODO remove

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

    cout << "trans end" << endl; // TODO remove

    unlockMap("write");
    return 0;
}

/**
 * Print full bank status to standard output
 */
void bank::getStatus() {// Print full bank status to standard output.
    //  request the read lock for the accounts map
    //  request read lock for each account separately to ensure valid value
    //  we want to get the status off all the accounts at a certain point, and therefore have to
    //  first get the locks of all and not go one by one and release each immediately.
    cout << "getstatus start" << endl;  // TODO remove

    lockMap("read");;
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
    cout << "getstatus end" << endl; // TODO remove
    unlockMap("write");
    unlockMap("read");

}

/**
 * collect random fee (2-4%) every 3 seconds
 * @return
 */
void bank::collect_fee() {
    double commission = (rand() % 3 + 2) / 100.0;
    unsigned int profit, total_profit = 0;

    cout << "collect start" << endl;  // TODO remove

    lockMap("read");
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

    }
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {

        it->second.unlock("write");
    }
    unlockMap("write");
    unlockMap("read");
    cout << "collect end" << endl;  // TODO remove


    // update the bank's balance.
    // only 1 thread writes to this, and 1 thread reads, so standard lock is fine.
    pthread_mutex_lock(&balanceLock);
    _bankBalance += total_profit;
    pthread_mutex_unlock(&balanceLock);

}


void bank::delete_account(unsigned int acntNum, string pass, string atmID) {
    lockMap("read");
    lockMap("write");
    cout << "delete start" << endl;  //TODO remove

    //  check account exists
    if (!is_account_exists(acntNum)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");
        unlockMap("write");
        unlockMap("read");
        return;
    }
    std::map<unsigned int, account>::iterator it_currently_handled_account;
    it_currently_handled_account = _accounts.find(acntNum);

    // verify password
    if (!it_currently_handled_account->second.check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + convert_to_string(acntNum) +
            " is incorrect");
        unlockMap("write");
        unlockMap("read");
        return;
    }
    // if we got here- all details are correct.
    // prevent others accessing the account

    // TODO lockMap("write");???
    //  prevent any further actions on the account
    it_currently_handled_account->second.lock("write");

    //  we have waited as late as possible with blocking the map reading, but now it's time.
    //lockMap("read");

    // wait until no one reads the account
    //  TODO make sure this  means only one thread, which is this check function is reading from the account
    while (it_currently_handled_account->second.check_num_of_readers() != 1);
    //  get the lock
    it_currently_handled_account->second.lock("read");

    //  save last balance value for logging it
    unsigned int last_balance = it_currently_handled_account->second.getBalance();

    //  call account's d'tor
    it_currently_handled_account->second.~account();

    //  TODO:  make sure we have cleaned all memory, threads and 2 locks
    cout << "delete end" << endl;  //TODO remove
    unlockMap("write");
    unlockMap("read");

    // log action success and free the lock.
    log(atmID + ": Account " + convert_to_string(acntNum) + " is now closed. Balance was " +
        convert_to_string(last_balance));
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
    cout << "checkBalnce start" << endl;  //TODO remove
    lockMap("read");
    lockMap("write");  // no changes in the middle of checking
    unlockMap("read");
    if (!is_account_exists(acntNum)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - account id " + to_string(acntNum) +
            " does not exist");

        sleep(1); //actions take 1 second to perform by definition
        // success message
        unlockMap("write");
        return;
    }
    if (!_accounts.find(acntNum)->second.check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        unlockMap("write");
        return;
    }
    unsigned int bal = _accounts.find(acntNum)->second.getBalance();
    log(atmID + ": Account " + to_string(acntNum) + " balance is: " + to_string(bal));
    _accounts.find(acntNum)->second.unlock("read");

    unlockMap("write");
    return;


}



