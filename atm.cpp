/*
 *  "atm.cpp"
 *  Implementation of the atm class
 *  used for maintaining the atms in the system.
 *  the atm's job is to parse the input commands, and send the instruction over to the bank.
 */

#include <vector>
#include <iostream>
#include "atm.h"

//********************************************
// function name: atm
// Description: atm's constructor implementation.
//              Open the cmd_file, and adds the lines one at a time to _cmds <list>
// Parameters: id - atmID, cmd_file - path to txt file containing the commands, owner_bank - reference to the bank object
// Returns: void
//**************************************************************************************
atm::atm(std::string id, std::string cmd_file, bank &owner_bank) : _id(id), _owner_bank(owner_bank) {
    std::ifstream in_file(cmd_file.c_str());
    std::string line;
    while (std::getline(in_file, line)) {
        _cmds.push_back(line);
    }
    in_file.close();
}


// TODO : this function isn't done
/**
 *  Description: get the next command from _cmds and call the relevant bank method to execute it.
 *  has to execute only one command at a time
 *
 * @return True - there are more commands to execute, False - this was the last command
 */
bool atm::execute_cmd() {
    if (_cmds.empty()) {   // should only enter this if _cmds was empty to begin with.
        return false;
    }

    //TODO I have copied most of it from stackoverflow, needs to be adjusted and checked

    //  split the command into args with " " as the delimiter
    std::istringstream streamer(_cmds.front()); // copy next command
    _cmds.pop_front(); // pop it out of the stack since we are handling it now.
    std::vector <std::string> args;
    std::string s("");
    while (std::getline(streamer, s, ' ')) // push the split arguments into args.
        args.push_back(s);

    //TODO fill commands after conditions
    // the bank supposed to have these methodes, but only checking and sending them to be executed by account class
    int accountNum = atoi(args[1]);
    string password = args[2];
    // check first argument, and call relevant method. input is guaranteed to be valid
    if (args[0] == "O") {
        cout << "blaaa" << endl;
        int initial_amount = atoi(args[3]);

        _owner_bank.create_account(accountNum, initial_amount, password, this->_id);

    } else if (args[0] == "D") {
        int amount = atoi(args[3]);
        _owner_bank.deposit(accountNum, password, amount, this->_id);

    } else if (args[0] == "W") {
        int amount = atoi(args[3]);
        _owner_bank.withdrawal(accountNum, password, amount, this->_id);
    } else if (args[0] == "B") {
        _owner_bank.check_balance(accountNum, password, this->_id);

    } else if (args[0] == "Q") {

        _owner_bank.delete_account(accountNum, password, this->_id);


    }
    else // transfer command
    {
        int target_account = atoi(args[3]);
        int amount = atoi(args[4]);
        _owner_bank.transfer_money(accountNum, password, target_account, amount, this->_id);

    }
    //	return !_cmds.empty();
}

}