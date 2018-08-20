#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <map>
#include <functional>
#include "help.hpp"

namespace term_app {

constexpr char enter_cmd[] = "";

constexpr char help_cmd[] = "help";
constexpr char cmd1_cmd[] = "cmd1";
constexpr char cmd2_cmd[] = "cmd2";
constexpr char exit_cmd[] = "exit";
constexpr char quit_cmd[] = "quit";

enum class cmdCode {
    enter,
    help,
    cmd1,
    cmd2,
    exit,
    quit,

    notValid,
};

const std::map<std::string, cmdCode> cmdMap {
    {enter_cmd , cmdCode::enter},

    {help_cmd , cmdCode::help},
    {cmd1_cmd , cmdCode::cmd1},
    {cmd2_cmd , cmdCode::cmd2},
    {exit_cmd , cmdCode::exit},
    {quit_cmd , cmdCode::quit}
};

struct cmdPack {
    cmdCode code;
    std::string args;
};

enum class retCode{
    OK,
    NOK,
    EXIT,
};

using callback = std::function<std::string(std::string&)>;

struct funcCbDescriptor {
    const char* fName;
    callback fCb;
};

const std::map<cmdCode, funcCbDescriptor> fCbMap {
    {cmdCode::enter, {enter_cmd, processEnter}},
    {cmdCode::help, {help_cmd, processHelp}},
    {cmdCode::cmd1, {cmd1_cmd, processCmd1}},
    {cmdCode::cmd2, {cmd2_cmd, processCmd2}},
    {cmdCode::exit, {help_cmd, processExit}},
    {cmdCode::quit, {help_cmd, processQuit}},
    {cmdCode::notValid, {help_cmd, processFailure}},


};

};

#endif
