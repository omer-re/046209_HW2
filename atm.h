//
// Created by os on 14.5.2020.
//
/*
 *  "atm.h"
 *  Declaration of the atm class
 *  used for maintaining the atms in the system.
 *  the atm's job is to parse the input commands, and send the instruction over to the bank.
 *  A reference to the bank object is kept in the class in order to send commands to it for execution.
 */

#ifndef HW2_ATM_H
#define HW2_ATM_H

#include <string>
#include <list>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <stdlib.h>
#include "bank.h"


//********************************************
// Class name:  atm
// Description: used for maintaining the atms in the system.
//              the atm's job is to parse the input commands, and send the instruction over to the bank.
//              A reference to the bank object is kept in the class in order to send commands to it for execution.
// Parameters:  _id - ATM number, _cmds - list of the commands to execute, sorted
//              _owner_bank - reference to the system's bank object
//**************************************************************************************
class atm {
private:
    std::string _id;
    std::list <std::string> _cmds;
    bank &_owner_bank;

public:
    // C'tor
    atm(std::string id, std::string cmd_file, bank &owner_bank); // atm's constructor, need to initialize _cmds
    bool execute_cmd(); // get the next command from _cmds and call the relevant bank method to execute it.

};




#endif //HW2_ATM_H
