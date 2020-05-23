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

/**
 * turn our input object into string.
 * @param our input
 * @return string value
 */
std::string to_string(T value) {
    //create an output string stream
    std::ostringstream os;

    //throw the value into the string stream
    os << value;

    //convert the string stream into a string and return
    return os.str();
}


/**
 * Checks if the given bank account id is valid
 * @param account_id
 * @return bool value
 */
bool bank::is_account_exists(unsigned int account_id) {
    lockMap("read");
    if (_accounts.find(account_id)) == _accounts.end()) //means it didn't find the id
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
void bank::lockMap(std::string rw) {

    if (rw == "read") // requested lock is read lock, perform reader lock as in the algorithm
    {
        pthread_mutex_lock(&accountsReadlock);
        if (++_accountsReaders == 1)
            pthread_mutex_lock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);
    } else if (rw == "write")  // requested lock is write lock, perform reader lock as in the algorithm
        pthread_mutex_lock(&accountsWritelock);
}

/**
 * // Wrapper function for implementing readers/writers mutual exclusion on the accounts map in the bank.
 * @param rw
 */
void bank::unlockMap(std::string rw) {
    //we assume that we are calling this func after one of the locks has been called, and that we are still dealing with the same flag (read or write)
    if (rw == "read") {   // need to free the read lock
        pthread_mutex_lock(&accountsReadlock);
        if (--_accountsReaders == 0)
            pthread_mutex_unlock(&accountsWritelock);
        pthread_mutex_unlock(&accountsReadlock);
    } else if (rw == "write")  // need to free the write lock
        pthread_mutex_unlock(&accountsWritelock);
}


/**
 * Create new account after validating for duplications
 * @param acntNum
 * @param initBalance
 * @param pass
 * @param atmID
 */
void bank::create_account(unsigned int acntNum, int initBalance, std::string pass, std::string atmID) {
    // verify valid 4 digits password
    std::string::const_iterator it = pass.begin();
    while (it != pass.end() && std::isdigit(*it)) ++it;
    if (it != pass.end || pass.length() != 4)
        return;  //  we are not expecting this case as defined by instructions.

    // password is valid
    lockMap("write");
    if (_account.find(acntNum) != _account.end())  // means this number is already exists
    {
        log("Error " + atmID + ": Your transaction failed - account with the same id exists");
    } else {
        _account.insert(std::pair < unsigned
        intm
        account > (acntNum, account(acntNum, initBalance, pass)));
        sleep(1); //actions take 1 second to perform by definition
        // success message
        log(atmID + ": New account id is " + to_string(acntNum) + " with password " + pass + " and initial balance " +
            to_string(initBalance));
    }
    unlockMap("write");
}


/**
 * Deposite to account as requested by an ATM
 * @param acntNum
 * @param pass
 * @param amount
 * @param atmID - requesting atm id
 */
void bank::deposit(unsigned int acntNum, std::string pass, unsigned int amount, std::string atmID) {
    // TODO prevent someone delete my account after I search for it
    // TODO locMap(write)
    lockMap("write");
    if (!is_account_exists(acntNum)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // verify password
    if (!_account.find(acntNum)->check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // if we got here- all details are correct.
    //we request the lock
    _accounts.find(acntNum)->lock("write");
    _accounts.find(acntNum)->deposit(amount);

    sleep(1); //actions take 1 second to perform by definition
    // success message
    unlockMap("write");
    // log action success and free the lock.
    unsigned int newbalance = _accounts.find(acntNum)->getBalance();
    log(atmID + ": Account " + to_string(acntNum) + " new balance is " + to_string(newbalance) + " after " +
        to_string(amnt) + " $ was deposited");
    _accounts.find(acntNum)->second.unlock("write");

}


/**
 *  withdrawal from account as requested by an ATM
 * @param amount_of_money
 */
void bank::withdrawal(unsigned int acntNum, std::string pass, unsigned int amount, std::string atmID) {
    if (!is_account_exists(acntNum)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // verify password
    if (!_account.find(acntNum)->check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // if we got here- all details are correct.
    //we request the lock
    _account.find(acntNum)->lock("write");

    // make sure it has enough money in the account
    bool enough_money = _accounts.find(acntNum)->deposit(amount);
    sleep(1); //actions take 1 second to perform by definition
    if (!enough_money) {
        // log action failure, free the lock.
        log("Error " + atmID + ": Your transaction failed - account id " + to_string(acntNum) +
            " balance is lower than " + to_string(amnt));
        _accounts.find(acntNum)->second.unlock("write");
        return;
    }
    // success message
    // log action success and free the lock.
    unsigned int newbalance = _accounts.find(acntNum)->getBalance();
    log(atmID + ": Account " + to_string(acntNum) + " new balance is " + to_string(newbalance) + " after " +
        to_string(amnt) + " $ was withdrew");
    _accounts.find(acntNum)->second.unlock("write");
}

/**
 *  transfer money between accounts
 * @param account_id1
 * @param id1_pass
 * @param account_id2
 * @param id2_pass
 * @param amount_of_money
 * @return
 */
int transfer_money(unsigned int source_account_id, unsigned int source_account__pass, unsigned int dest_account_id,
                   unsigned int amount_of_money) {
    // check source account exists
    if (!is_account_exists(source_account_id)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // check source account password
    if (!_account.find(source_account_id)->check_password(source_account__pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }

    // check dest account exists
    if (!is_account_exists(dest_account_id)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // dest account password isn't checked- as instructed

    // check source!=dest
    if (srcAcnt == dstAcnt)
        return; // must do nothing, otherwise we will get into a deadlock

    // lock src and dest, in rising order to prevent deadlock
    _accounts.find(std::min(source_account_id, dest_account_id))->second.lock("write");
    _accounts.find(std::max(source_account_id, dest_account_id))->second.lock("write");

    // check source has enough money
    bool enough_money = _accounts.find(source_account_id)->deposit(amount_of_money);
    sleep(1); //actions take 1 second to perform by definition
    if (!enough_money) {
        // log action failure, free the lock.
        log("Error " + atmID + ": Your transaction failed - account id " + to_string(acntNum) +
            " balance is lower than " + to_string(amnt));
        _accounts.find(acntNum)->second.unlock("write");
        return;
    } else {
        // deposit to dest account. log the transfer success.
        _accounts.find(dstAcnt)->second.deposit(amount_of_money);
        unsigned int source_balance = _accounts.find(source_account_id)->second.getBalance();
        unsigned int dest_balance = _accounts.find(dest_account_id)->second.getBalance();
        log(atmID + ": Transfer " + to_string(amount_of_money) + " from account " + to_string(source_account_id) +
            " to account " + to_string(dest_account_id) + " new account balance is " + to_string(source_balance) +
            " new target account balance is " + to_string(dest_balance));
    }

    // open locks
    _accounts.find(source_account_id)->second.unlock("write");
    _accounts.find(dest_account_id)->second.unlock("write");

}

/**
 * Print full bank status to standard output
 */
void bank::getStatus() {// Print full bank status to standard output.
    //  request the read lock for the accounts map
    //  request read lock for each account separately to ensure valid value
    //  we want to get the status off all the accounts at a certain point, and therefore have to
    //  first get the locks of all and not go one by one and release each immediately.

    lockMap("read")
    // TODO should we prevent writing to map as well to avoid new accounts to be made while getting status?
    // TODO LockMap("write");
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
        it->second.print();
    }
    printf(".\n.\n");
    printf("The Bank has %d $\n", _bankBalance);
    // free all of the locks.
    pthread_mutex_unlock(&balanceLock);
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {
        it->second.unlock("read");
    }
    unlockMap("read");
    // TODO unlockMap("write);??

}

/**
 * collect random fee (2-4%) every 3 seconds
 * @return
 */
int bank::collect_fee() {
    double commission = (rand() % 3 + 2) / 100.0;
    unsigned int profit, total_profit = 0;
    lockMap("read"); // no new accounts are made while running fee collection
    // TODO should we prevent writing to map as well to avoid new accounts to be made while getting status?
    // TODO LockMap("write");
    std::map<unsigned int, account>::iterator it;
    for (it = _accounts.begin(); it != _accounts.end(); ++it) {
        it->second.lock("write");
        if (!it->second.isVip()) {   // account is not VIP, take commissions
            profit = (unsigned int) round(it->second.getBalance() * commission);
            it->second.withdraw(profit);
            total_profit += profit;
            // log commissions taken from the account.
            log("Bank: commissions of " + to_string((int) (commission * 100)) + " % were charged, the bank gained "
                + to_string(profit) + " $ from account " + to_string(it->first));
        }
        it->second.unlock("write");
    }
    unlockMap("read");
    //TODO unlock write


    // update the bank's balance.
    // only 1 thread writes to this, and 1 thread reads, so standard lock is fine.
    pthread_mutex_lock(&balanceLock);
    _bankBalance += total_profit;
    pthread_mutex_unlock(&balanceLock);

}


void bank::delete_account(unsigned int acntNum, std::string pass, std::string atmID) {
    //  check account exists
    if (!is_account_exists(acntNum)) {
        // log the error and return
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    std::map<unsigned int, account>::iterator it_currently_handled_account;
    it_currently_handled_account = _account.find(acntNum)

    // verify password
    if (!it_currently_handled_account->check_password(pass)) {
        // log bad pass, and return.
        log("Error " + atmID + ": Your transaction failed - password for account id " + to_string(acntNum) +
            " is incorrect");
        return;
    }
    // if we got here- all details are correct.
    // prevent others accessing the account

    // TODO lockMap("write");???
    //  prevent any further actions on the account
    it_currently_handled_account->lock("write");

    //  we have waited as late as possible with blocking the map reading, but now it's time.
    lockMap("read");

    // wait until no one reads the account
    //  TODO make sure this  means only one thread, which is this check function is reading from the account
    while (it_currently_handled_account.check_num_of_readers() != 1);
    //  get the lock
    it_currently_handled_account->lock("read");

    //  save last balance value for logging it
    unsigned int last_balance = it_currently_handled_account.getBalance()

    //  call account's d'tor
    it_currently_handled_account.~account();

    //  TODO:  make sure we have cleaned all memory, threads and 2 locks
    unlockMap("read")
    // TODO unlockMap("write");???

    // log action success and free the lock.
    log(atmID + ": Account " + to_string(acntNum) + " is now closed. Balance was " + to_string(last_balance));
}






