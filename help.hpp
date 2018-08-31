#ifndef help_h
#define help_h

using namespace term_app;
std::string processEmptyInput(const std::string& arg) {

    return std::string {};
};

std::string processHelp(const std::string& arg) {
    std::string replyToSend {"Help output"};

    return replyToSend;
};

// TODO: move to separate files
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
    std::string replyToSend {"Unknown command:"};

    if(IAC == *arg.begin()) {
        for(auto c:arg) {
            replyToSend.append(std::to_string(static_cast<uint8_t>(c))); replyToSend.push_back(' ');
        }
    }
    return replyToSend.append(arg);
};


//

#endif /* help_h */
