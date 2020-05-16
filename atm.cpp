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
//*******************************************
// function name: execute_cmd
// Description: get the next command from _cmds and call the relevant bank method to execute it.
// Parameters: none
// Returns: True - there are more commands to execute, False - this was the last command
//**************************************************************************************
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

    // check first argument, and call relevant method. input is guaranteed to be valid
    if (args.front() == "O") {}
    else if (args.front() == "D") {}
    else if (args.front() == "W") {}
    else if (args.front() == "B") {}
    else // transfer command
    {}
    return !_cmds.empty();
}