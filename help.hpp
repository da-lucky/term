#ifndef help_h
#define help_h

using namespace term_app;
std::string processEnter(std::string& arg) {

    return std::string {};
};

std::string processHelp(std::string& arg) {
    std::string replyToSend {"Help output"};

    return replyToSend;
};

// TODO: move to separate files
std::string processCmd1(std::string& arg) {
    std::string replyToSend {"Cmd1 output"};

    return replyToSend;
};

std::string processCmd2(std::string& arg) {
    std::string replyToSend {"Cmd2 output"};

    return replyToSend;
};

std::string processExit(std::string& arg) {
    std::string replyToSend {"Exit output"};

    return replyToSend;
};

std::string processQuit( std::string& arg) {
    std::string replyToSend {"Quit output"};

    return replyToSend;
};

std::string processFailure(std::string& arg) {
    std::string replyToSend {"Unknown command:"};

    return replyToSend.append(arg);
};


//

#endif /* help_h */
