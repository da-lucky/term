#ifndef help_h
#define help_h

#include "commonDefs.hpp"

using namespace term_app;

std::string processHelp(const std::string& arg) {
    std::string replyToSend {"Help output"};

    return replyToSend;
};

// TODO: move to separate files
std::string processEmptyInput(const std::string& arg) {

    return std::string {};
};

std::string processCmd1(const std::string& arg) {
    std::string replyToSend {"Cmd1 output"};

    return replyToSend;
};

std::string processCmd2(const std::string& arg) {
    std::string replyToSend {"Cmd2 output"};

    return replyToSend;
};

std::string processExit(const std::string& arg) {
    std::string replyToSend {"Exit output"};

    return replyToSend;
};

std::string processQuit(const std::string& arg) {
    std::string replyToSend {"Quit output"};

    return replyToSend;
};

std::string processFailure(const std::string& arg) {
    std::string replyToSend {"\tUnknown command:"};

    return replyToSend.append(arg);
};


//

#endif /* help_h */
