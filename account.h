/*
 * "account.h"
 * Declaration of the account class
 * used for maintaining the accounts in the bank's system.
 * NOT PROTECTED - protections will be done outside of the class, using the object's private locks.
 */

#ifndef HW2_ACCOUNT_H
#define HW2_ACCOUNT_H

#include <string>
#include <string.h>

#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <semaphore.h>	

using namespace std;
//********************************************
// Class name:  account
// Description: used for maintaining the accounts in the bank's system.
// Parameters:  _account_id - account number,  _balance - account balance
//              _password - account password,
//              (_num_of_Readers,readlock,writelock) - used for readers/writers lock implementation
//**************************************************************************************

class account {
private:
	const unsigned int _account_id;
	unsigned int _balance;
	int _num_of_Readers;  // used for readers-writers implementation
     string _password;

	pthread_mutex_t readlock, writelock; // for readers/writers lock implementation


public:
	// C'tor + init the object's mutexs
	account(unsigned int acntNum, int initBalance, std::string pass, string atmId) :
		_account_id(acntNum), _balance(initBalance), _password(pass)
	{
		
		if (pthread_mutex_init(&readlock, NULL) ||
			pthread_mutex_init(&writelock, NULL)) {   // init allocates memory, need to make sure sys call didnt fail
			perror("system call error:");
			exit(1);
		}
		_num_of_Readers = 0;
	}

	//D'tor + destroy locks
	~account() {
		pthread_mutex_destroy(&readlock);
		pthread_mutex_destroy(&writelock);
		
	}
	
	void deposit(unsigned int amount_of_money);
	bool withdrawal(unsigned int amount_of_money);  // if there's not enough balance - return false
	unsigned int getBalance();

    bool check_password(string password);
	void lock(std::string rw); // Wrapper function for managing Readers/Writers mutual exclusions
	void unlock(std::string rw); // Wrapper function for managing Readers/Writers mutual exclusions
	void account_print();

	int check_num_of_readers();


	string getpass();
};



#endif //HW2_ACCOUNT_H
// 4.06 15.00 removed comments