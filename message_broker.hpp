


#ifndef IRODS_MESSAGE_QUEUE_HPP
#define IRODS_MESSAGE_QUEUE_HPP

#include <vector>
#include <iostream>
#include <sstream>

#include "zmq.hpp"


class message_broker {
public:
    class exception : public std::exception {
        // -1 - sys sock connect error
        // -2 - invalid operation
        public:
            exception(
                uint32_t           _code,
                const std::string& _msg) :
                    code_(_code),
                    msg_(_msg) {
            }

            ~exception() {}

            const char* what () const noexcept {
                return msg_.c_str();
            }

            uint32_t code () const {
                return code_;
            }

        private:
            uint32_t    code_;
            std::string msg_;
    }; // class exception

    typedef std::vector<uint8_t> data_type;

    message_broker(const std::string& _type, zmq::context_t* _zmq_ctx_ptr) :
        ctx_ptr_(_zmq_ctx_ptr), do_not_delete_ctx_ptr_(true) {
        try {
            create_socket(_type);
        }
        catch ( const zmq::error_t& _e) {
            throw exception(-2, _e.what());
        }
    }

    message_broker(const std::string& _ctx) : 
        ctx_ptr_(new zmq::context_t(1)), do_not_delete_ctx_ptr_(false) {
        try {
            create_socket(_ctx);
        }
        catch ( const zmq::error_t& _e) {
            throw exception(-2, _e.what());
        }
    }

    ~message_broker() {
        skt_ptr_->close();
        if(!do_not_delete_ctx_ptr_) {
            delete ctx_ptr_;
        }
    }

    void send(const data_type& _data) {
        try {
            zmq::message_t msg( _data.size() );
            memcpy(
                msg.data(),
                _data.data(),
                _data.size() );
            while(!skt_ptr_->send( msg ) ) {
                //TODO: need backoff
                    continue;
            }
        }
        catch ( const zmq::error_t& _e) {
            std::cerr << _e.what() << std::endl;
        }
    }

    void send(zmq::message_t& _data) {
        try {
            while(!skt_ptr_->send( _data ) ) {
                //TODO: need backoff
                    continue;
            }
        }
        catch ( const zmq::error_t& _e) {
            std::cerr << _e.what() << std::endl;
        }
    }

    void receive(data_type& _data, const int flags=0, const bool debug=false) {
        try {
            zmq::message_t msg;
            while(true) {
                int ret = skt_ptr_->recv( &msg, flags );
                if(-1 == ret && ZMQ_DONTWAIT == flags) {
                    if(debug) {
                        std::cout << "dontwait failed in receive" << std::endl; fflush(stdout);
                    }
                    if(zmq_errno() == EAGAIN) {
                        if(debug) {
                           std::cout << "dontwait with EAGAIN" << std::endl; fflush(stdout);
                        }
                        break;
                    }
                }
                else if(ret <= 0 && ZMQ_DONTWAIT != flags) {
                    int eno = zmq_errno();
                    std::cout << "read error :: ret - " << ret << "    errno - " << eno << std::endl;
                    if(EAGAIN == ret || eno == EAGAIN) {
                        throw exception(-1, "time out in receive");
                    }

                    //TODO: need backoff
                    continue;
                }
                break;
            }
        
            if(msg.size() > 0) {
                _data.resize(msg.size());
                std::memcpy(_data.data(), msg.data(), msg.size());
            }
        }
        catch ( const zmq::error_t& _e) {
            std::cerr << _e.what() << std::endl;
        }
    }

    void connect(const std::string& _conn) {
        try {
            skt_ptr_->connect(_conn);
        }
        catch(const zmq::error_t& _e) {
            throw exception(-1, _e.what());
        }
    }

    void bind(const std::string& _conn) {
        try {
            skt_ptr_->bind(_conn);
        }
        catch(const zmq::error_t& _e) {
            throw exception(-1, _e.what());
        }
    }

    void bind_to_port(size_t _port) {
        try {
            std::stringstream csstr;
            csstr << "tcp://*:" << _port;
            skt_ptr_->bind(csstr.str().c_str());
        }
        catch(const zmq::error_t& _e) {
            throw exception(-1, _e.what());
        }

    } // bind_to_open_port

    int bind_to_port_in_range(size_t _first, size_t _last) {
        for(auto port = _first; port < _last; ++port) {
            try {
                bind_to_port(port);
                return port;
            }
            catch(exception& _e) {
                continue;
            }
        } // for

        // did not find a socket in range
        std::stringstream ss;
        ss << "failed to find point in range "
           << _first << " to " << _last;
        throw exception(-1, ss.str());

    } // bind_to_open_port


private:
    void create_socket(const std::string _ctx) {
        try {
            int time_out = 1500;
            if("ZMQ_REQ" == _ctx ) {
                skt_ptr_ = std::unique_ptr<zmq::socket_t>(
                               std::make_unique<zmq::socket_t>(
                                   *ctx_ptr_, ZMQ_REQ));
            }
            else {
                skt_ptr_ = std::unique_ptr<zmq::socket_t>(
                               std::make_unique<zmq::socket_t>(
                                   *ctx_ptr_, ZMQ_REP));
            }

            skt_ptr_->setsockopt( ZMQ_RCVTIMEO, &time_out, sizeof( time_out ) );
            skt_ptr_->setsockopt( ZMQ_SNDTIMEO, &time_out, sizeof( time_out ) );
            skt_ptr_->setsockopt( ZMQ_LINGER, 0 );
        }
        catch ( const zmq::error_t& _e) {
            throw exception(-2, _e.what());
        }
    }

    zmq::context_t* ctx_ptr_;
    bool do_not_delete_ctx_ptr_;
    std::unique_ptr<zmq::socket_t> skt_ptr_;

}; // class message_broker


std::ostream& operator<<(
    std::ostream& _os,
    const message_broker::data_type& _dt) {
    std::string msg;
    msg.assign(_dt.begin(), _dt.end());
        _os << msg;
        return _os;  
} // operator<<

static const message_broker::data_type QUIT_MSG = {'q', 'u', 'i', 't'};
static const message_broker::data_type ACK_MSG = {'A', 'C', 'K'};

#endif // IRODS_MESSAGE_QUEUE_HPP



