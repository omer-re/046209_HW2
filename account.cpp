/*
 *  "account.cpp"
 *  Implementation of the account class
 *  used for maintaining the accounts in the bank's system.
 *  NOT PROTECTED - protections will be done outside of the class, using the object's private locks.
 */
#include "account.h"

using namespace std;

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

bool account::check_password(string password) {
	if (_password == password) {
		return true;
	}
	return false;
}

/** Prints the account status to stdout
 *  assumes read lock has been done
 */
void account::account_print() {
    std::cout << "Account " << _account_id << ": Balance - " << _balance << " $ , Account Password - " << _password
              << std::endl;
}

void account::lock(std::string rw) { // Wrapper function for managing Readers/Writers mutual exclusions
	if (rw == "read") { // requested lock is read lock, perform reader lock as in the algorithm

		pthread_mutex_lock(&readlock);
		_num_of_Readers++;
		if (_num_of_Readers == 1) {
			pthread_mutex_lock(&writelock);
		}
		pthread_mutex_unlock(&readlock);
		// need to free the write lock

	}// requested lock is write lock, perform write lock as in the algorithm

	else if (rw == "write") {

		pthread_mutex_lock(&writelock);

	}
}

void account::unlock(std::string rw) { // Wrapper function for managing Readers/Writers mutual exclusions
// It is assumed that unlock is called after lock was called, and with the same rw
	if (rw == "read") { // need to free the read lock

		pthread_mutex_lock(&readlock);
		_num_of_Readers--;
		if (_num_of_Readers == 0) {
			pthread_mutex_unlock(&writelock);
		}
		pthread_mutex_unlock(&readlock);
	}// need to free the write lock
	else if (rw == "write") {
		pthread_mutex_unlock(&writelock);
	}
}


int account::check_num_of_readers() {
    return _num_of_Readers;
}

string account::getpass()
{
	return _password;
}

// 4.06 15.00 removed comments