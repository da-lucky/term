#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <array>
#include <algorithm>
#include "commonDefs.hpp"
#include "inputInterpreter.hpp"
#include "commands.hpp"


using namespace term_app;

namespace {

constexpr std::size_t MAX_BUF_SIZE = 256;

void prompt(int sock) {
    static const std::string defaultPrompt(std::string(APP_NAME) + std::string(":"));
    write(sock, defaultPrompt.c_str(), defaultPrompt.size());
}

cmdPack defineCmd(std::array<char,MAX_BUF_SIZE>& input) {

    cmdPack cp {};  
    std::string cmd {};

    cp.args = input.data();
    auto space = cp.args.find_first_of(' ');
    
    if(space != std::string::npos) {
        cmd = cp.args.substr(0, space);
        cp.args = cp.args.substr(space, std::string::npos);
    } else {
        cmd.swap(cp.args);
    }

    auto it = cmdMap.find(cmd.c_str());

    cp.code = (it != cmdMap.end()) ? it->second : cmdCode::notValid; // ???

    std::cout << "cmd received is :" << cmd << "\targs:" << cp.args << " op code " << static_cast<int>(cp.code)  << "\n";

    // TEMP
    if(!std::strcmp(exit_cmd, input.data())) {
        cp.code = cmdCode::exit;
    }
    //

    return cp;
}


cmdPack handleInput(std::array<char,MAX_BUF_SIZE>& input, std::size_t readBytes) {
           
    if(readBytes > 1 && std::equal(&input[readBytes-2], &input[readBytes], &CR_LF[0])) {
        input[readBytes-2] = '\0';
    }

    cmdPack cp = defineCmd(input);

    return cp;
}

retCode executeCmd(cmdPack& cp) {

    // TEMP
    if(cmdCode::exit == cp.code) {
        return retCode::EXIT;
    }
    //

    return retCode::OK;
}

}

namespace session {

void handler(int sock, bool& isActiveFlag) {

    std::array<char,MAX_BUF_SIZE> recvBuff;
    
    prompt(sock);

    while(isActiveFlag) {
        
        auto retCode = recv(sock, recvBuff.data(), recvBuff.size(), 0);
        
        if(retCode != -1) {

            auto cmd = handleInput(recvBuff, retCode);

            if( executeCmd(cmd) != retCode::OK ) {
                break;
            }
        } else {
            std::cerr << formErrnoString("recv returned -1:");
            break;
        }

        prompt(sock);
    }

    isActiveFlag = false;
    close(sock);
    return;    
}

}

