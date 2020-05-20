/*
 *  "account.cpp"
 *  Implementation of the account class
 *  used for maintaining the accounts in the bank's system.
 *  NOT PROTECTED - protections will be done outside of the class, using the object's private locks.
 */
#include "account.h"

/**
 * add money to the account.
 * working under the assumption that write-lock is done
 * @param amount_of_money
 */
void account::deposit(unsigned int amount_of_money) {
    _balance += amount_of_money;
}

/**
 * if there is enough money in the account- reduces it by the amount given
 * @param amount_of_money
 * @return true if valid, false if not enough balance.
 */
bool account::withdrawal(unsigned int amount_of_money) {  // if there's not enough balance - return false
    if (_balance >= amount_of_money) {
        _balance -= amount_of_money;
        return true;
    } else return false;
}

unsigned int account::getBalance() {
    return _balance;
}

bool account::check_password(unsigned int password) {
    return (password == _password);
}

/** Prints the account status to stdout
 *  assumes read lock has been done
 */
void account::account_print() {
    std::cout << "Account " << _account_id << ": Balance - " << _balance << " $ , Account Password - " << _password
              << std::endl;
}

void account::lock(std::string rw); // Wrapper function for managing Readers/Writers mutual exclusions
void account::unlock(std::string rw); // Wrapper function for managing Readers/Writers mutual exclusions

