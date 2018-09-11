#ifndef TELNET_CONTROL_COMMANDS_HPP_
#define TELNET_CONTROL_COMMANDS_HPP_

#include "commonDefs.hpp"

using namespace term_app;
using namespace TELNET::CMD_CODE;
using namespace TELNET::OPTS_CODE;

namespace term_app {

std::string processIAC(const std::string& arg) {

    std::string replyToSend {};

    if(arg == TELNET::DO_SGA) {
        replyToSend = TELNET::WILL_ECHO;
    }

    if(arg == TELNET::DO_ECHO) {
//        replyToSend = {IAC, WILL, NAWS};
    }

    return replyToSend;
};


}

#endif
