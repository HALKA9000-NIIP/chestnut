#pragma once

#include "command.hpp"



Command::Command() {}

void Command::runCommand(std::string cmd, Options opt) {
    options = opt;
    typedef std::map<std::string, void(*)()> cmdprocs;
    cmdprocs procs;

    if(options.exists("-limit")) {
        std::string input = options.get("-limit");

        for(int i = 0; i < input.length() - 1; i++)
            if(input[0] == '0') input.erase(input.begin());
            else break;

        try {
            int limit = std::stoi(input);

            if(limit <= 0)
                Console::error("cerr3681", "invalid display limit", { { "input", input } }, true);

            Console::displayCountLimit = limit;
        } catch(std::invalid_argument) {
            Console::error("cerr3681", "invalid display limit", { { "input", input } }, true);
        }
    }

    procs.insert(std::make_pair("ches", Command::c_ches));
    procs.insert(std::make_pair("cmp", Command::c_cmp));
    procs.insert(std::make_pair("run", Command::c_run));
    procs.insert(std::make_pair("set", Command::c_set));

    cmdprocs::iterator it = procs.find(cmd);
    if(it != procs.end())
        it->second();
    else
        Console::error("cerr1064", "unknown command", { { "command", cmd } }, true);
}
