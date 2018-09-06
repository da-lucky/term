#include "SessionManager.hpp"

namespace {

using namespace session;

struct SessionThreadDescriptor {
    bool isActive {false};
    int socket {0};
    std::thread t {};
};

class SessionManager {
    std::array<SessionThreadDescriptor, term_app::MAX_NUM_SESSIONS_ALLOWED> m_threads {};

    const std::string ERROR_NO_MORE_CONNECTIONS_AVAILABLE {"no more connections available...\n"};

public:

    SessionManager() {
        for(auto& e: m_threads) {
            e.isActive = false;
        }
    }

    ~SessionManager() {
        for(auto& e: m_threads) {
            if(e.t.joinable()) {
                e.t.join();
            }
            e.isActive = false;
            close(e.socket);
        }        
    }
    
    session::SessionStatus enableSession(int socket) {
    
        auto td = std::find_if_not(m_threads.begin(), m_threads.end(), [](auto& e) { return e.isActive; });
    
        SessionStatus retCode = SessionStatus::OK;
        if(m_threads.end() != td) {
            if(td->t.joinable()) {
               td->t.join(); 
            }
    
            try {
                td->t = std::thread(handler, socket, std::ref(td->isActive));
                td->isActive = true;
                td->socket = socket;
    
            } catch(std::exception& e) {
                std::cerr << "exception during thread creation: " << e.what() << "\n";
                td->isActive = false;
                retCode = SessionStatus::NOK;
            }
        } else {            
            send(socket, ERROR_NO_MORE_CONNECTIONS_AVAILABLE.data(), ERROR_NO_MORE_CONNECTIONS_AVAILABLE.size(), 0);
            retCode = SessionStatus::NOK;
        }
    
        return retCode;
    }
};    


class SessionMngrSingleton {

    static std::unique_ptr<SessionManager> m_instance;

public:

    static SessionManager& getInstance() {
        if(! m_instance) {
            m_instance.reset(new SessionManager());
        }
        return *m_instance;
    }
};

std::unique_ptr<SessionManager> SessionMngrSingleton::m_instance = {nullptr};

}

namespace term_app {

SessionStatus startSession(int socket) {
    return SessionMngrSingleton::getInstance().enableSession(socket);
}

}
