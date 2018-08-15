#include <iostream>
#include <array>
#include <thread>
#include <memory>
#include <algorithm>

namespace session {

void prompt();

void provider(int sock, bool& isActiveFlag);

struct SessionThreadDescriptor {
    bool isActive {false};
    int socket {0};
    std::thread t {};
};

enum class SessionStatus {
    OK,
    NOK
};

class SessionManager {
    std::array<SessionThreadDescriptor, term_app::MAX_NUM_SESSIONS_ALLOWED> m_threads {};

public:

    SessionManager() {
        for(auto& e: m_threads) {
            e.isActive = false;
        }
    }

    ~SessionManager() {
        std::cout << "SessionManager d-tor\n";
        for(auto& e: m_threads) {
            std::cout << "SessionManager threads joining\n";
            if(e.t.joinable()) {
                e.t.join();
            }
            e.isActive = false;
            close(e.socket);
        }        
    }
    
    SessionStatus enableSession(int socket) {

        auto td = std::find_if_not(m_threads.begin(), m_threads.end(), [](auto& e) { return e.isActive; });

        SessionStatus retCode = SessionStatus::OK;
        if(m_threads.end() != td) {
            if(td->t.joinable()) {
               td->t.join(); 
            }

            try {
                td->t = std::thread(provider, socket, std::ref(td->isActive));
                td->isActive = true;
                td->socket = socket;

            } catch(std::exception& e) {
                std::cerr << "exception during thread creation: " << e.what() << "\n";
                td->isActive = false;
                retCode = SessionStatus::NOK;
            }
        } else {
            std::cerr << "no free threads..." << "\n";
            retCode = SessionStatus::NOK;
        }

        return retCode;
    }
};    

}

