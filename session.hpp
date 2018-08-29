#include <iostream>
#include <array>
#include <thread>
#include <memory>
#include <algorithm>

namespace session {

void handler(int sock, bool& isActiveFlag);

enum class SessionStatus {
    OK,
    NOK
};



}

