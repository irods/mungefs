/*
 * ** 27-12-2015
 * **
 * ** The author disclaims copyright to this source code.  In place of
 * ** a legal notice, here is a blessing:
 * **
 * **    May you do good and not evil.
 * **    May you find forgiveness for yourself and forgive others.
 * **    May you share freely, never taking more than you give.
 * **
 */

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <regex>
#include <thread>
#include <vector>
#include <fstream>

#include <cerrno>
#include <sys/types.h>
#include <csignal>

#include <fuse.h>

#include "message_broker.hpp"
#include "mungefs_ctl.hpp"

static std::ofstream err_log;

void print_ctl(const mungefs_ctl& _ctl) {
    err_log << std::endl << std::endl
            << __FUNCTION__ << std::endl 
            << "random: " << _ctl.random << std::endl
            << "err_no: " << _ctl.err_no << std::endl
            << "probability: " << _ctl.probability << std::endl
            << "regexp: " << _ctl.regexp << std::endl
            << "kill_caller: " << _ctl.kill_caller << std::endl
            << "delay_us: " << _ctl.delay_us << std::endl
            << "auto_delay: " << _ctl.auto_delay << std::endl
            << "corrupt_data: " << _ctl.corrupt_data << std::endl
            << "corrupt_size: " << _ctl.corrupt_size << std::endl;
    err_log << "operation: "<< std::endl;
    err_log << "     ";
    for(auto m : _ctl.operations) {
        err_log << m << ", ";
    }
    err_log << std::endl;
}

class server_handler {
    public:
    struct fault_descriptor {
        bool        random;       // error code must be randomized
        int         err_no;       // error code to return
        int32_t     probability;  // 0 < probability < 100, rnd error injection
        std::string regexp;       // regular expression on filename
        bool        kill_caller;  // Must we kill the caller
        int32_t     delay_us;     // operation delay in us
        bool        auto_delay;   // must auto delay like an SSD
        bool        corrupt_data; // corrupt read or write data
        bool        corrupt_size; // corrupt the size reported in a stat
        fault_descriptor() :
            random{false},
            err_no{0},
            probability{0},
            regexp{""},
            kill_caller{false},
            delay_us{0},
            auto_delay{false},
            corrupt_data{false},
            corrupt_size{false} {
        }
        fault_descriptor( const fault_descriptor& _rhs) :
            random{_rhs.random},
            err_no{_rhs.err_no},
            probability{_rhs.probability},
            regexp{_rhs.regexp},
            kill_caller{_rhs.kill_caller},
            delay_us{_rhs.delay_us},
            auto_delay{_rhs.auto_delay},
            corrupt_data{_rhs.corrupt_data},
            corrupt_size{_rhs.corrupt_size} {
        }
        fault_descriptor& operator=( const fault_descriptor& _rhs) {
            if(this == &_rhs) {
                return *this;
            }

            random = _rhs.random;
            err_no = _rhs.err_no;
            probability = _rhs.probability;
            regexp = _rhs.regexp;
            kill_caller = _rhs.kill_caller;
            delay_us = _rhs.delay_us;
            auto_delay = _rhs.auto_delay;
            corrupt_data = _rhs.corrupt_data;
            corrupt_size = _rhs.corrupt_size;
            return *this;
        }
    };

    void get_operations(std::vector<std::string> & _return) {
        for (auto op: valid_operations_) {
            _return.push_back(op);
        }
    }

    void clear_all_faults() {
        std::lock_guard<std::mutex> lk(fault_mutex_);
        fault_map_.clear();
    }

    void clear_fault(const std::string& _operation) {
        std::lock_guard<std::mutex> lk(fault_mutex_);
        fault_map_.erase(_operation);
    }

    void set_fault(
        const std::vector<std::string>& _operations,
        const bool                      _random,
        const int                       _err_no,
        const int32_t                   _probability,
        const std::string&              _regexp,
        const bool                      _kill_caller,
        int32_t                         _delay_us,
        const bool                      _auto_delay,
        const bool                      _corrupt_data,
        const bool                      _corrupt_size) {

        fault_descriptor descr;
        descr.random       = _random;
        descr.err_no       = _err_no;
        descr.probability  = _probability;
        descr.regexp       = _regexp;
        descr.kill_caller  = _kill_caller;
        descr.delay_us     = _delay_us;
        descr.auto_delay   = _auto_delay;
        descr.corrupt_data = _corrupt_data;
        descr.corrupt_size = _corrupt_size;

        std::lock_guard<std::mutex> lk(fault_mutex_);
        for (auto op: _operations) {
            if (is_valid_method(op)) {
                fault_map_[op] = descr;
            }
        } // for

    } // set_fault

    void set_all_fault(
        const bool         _random,
        const int32_t      _err_no,
        const int32_t      _probability,
        const std::string& _regexp,
        const bool         _kill_caller,
        int32_t            _delay_us,
        const bool         _auto_delay,
        const bool         _corrupt_data,
        const bool         _corrupt_size) {
        
        std::vector<std::string> operations(
           valid_operations_.begin(),
           valid_operations_.end());

        set_fault(
            operations,
            _random,
            _err_no,
            _probability,
            _regexp,
            _kill_caller,
            _delay_us,
            _auto_delay,
            _corrupt_data,
            _corrupt_size);
    } // set_all_fault

    server_handler() {
        valid_operations_.insert("getattr");
        valid_operations_.insert("readlink");
        valid_operations_.insert("mknod");
        valid_operations_.insert("mkdir");
        valid_operations_.insert("unlink");
        valid_operations_.insert("rmdir");
        valid_operations_.insert("symlink");
        valid_operations_.insert("rename");
        valid_operations_.insert("link");
        valid_operations_.insert("chmod");
        valid_operations_.insert("chown");
        valid_operations_.insert("truncate");
        valid_operations_.insert("open");
        valid_operations_.insert("read");
        valid_operations_.insert("write");
        valid_operations_.insert("statfs");
        valid_operations_.insert("flush");
        valid_operations_.insert("release");
        valid_operations_.insert("fsync");
        valid_operations_.insert("setxattr");
        valid_operations_.insert("getxattr");
        valid_operations_.insert("listxattr");
        valid_operations_.insert("removexattr");
        valid_operations_.insert("opendir");
        valid_operations_.insert("readdir");
        valid_operations_.insert("releasedir");
        valid_operations_.insert("fsyncdir");
        valid_operations_.insert("access");
        valid_operations_.insert("create");
        valid_operations_.insert("ftruncate");
        valid_operations_.insert("fgetattr");
        valid_operations_.insert("lock");
        valid_operations_.insert("bmap");
        valid_operations_.insert("ioctl");
        valid_operations_.insert("poll");
        valid_operations_.insert("flock");
        valid_operations_.insert("fallocate");
    }

    bool is_valid_method(const std::string& _operation) const {
        return valid_operations_.count(_operation);
    }

    bool check_for_fault(const std::string& _m) {
        std::lock_guard<std::mutex> lk(fault_mutex_);
        return fault_map_.find(_m) != fault_map_.end(); 
    }

    const fault_descriptor get_fault(const std::string& _m) {
        std::lock_guard<std::mutex> lk(fault_mutex_);
        return fault_map_[_m]; 
    }

    typedef message_broker::data_type data_t;
    void process_message(
        data_t& _msg) {
        auto in = avro::memoryInputStream(
                      &_msg[0],
                      _msg.size());
        auto dec = avro::binaryDecoder();
        dec->init( *in );
        mungefs_ctl ctl;
        avro::decode(*dec, ctl);

        set_fault(
            ctl.operations,
            ctl.random,
            ctl.err_no,
            ctl.probability,
            ctl.regexp,
            ctl.kill_caller,
            ctl.delay_us,
            ctl.auto_delay,
            ctl.corrupt_data,
            ctl.corrupt_size);
    } // process_message

private:
    std::set<std::string>                   valid_operations_;
    std::map<std::string, fault_descriptor> fault_map_;
    std::mutex                              fault_mutex_;
}; // class server_handler

static server_handler static_server_instance;

// return a random err_no
static int get_random_err_no() {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(E2BIG, EXFULL);
    return dist(rd);
} // get_random_err_no

// return true if random number is not in the probability
static bool check_for_random_fault(int _probability) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(1, 100000);

    if (!_probability) {
        return false;
    }

    if (dist(rd) > _probability) {
        return true;
    }
    
    return false;
} // check_for_random_fault

// return an err_no if we must proceed to error injection
static int evaluate_fault_for_operation_impl(
    const std::string&                _path,
    const std::string&                _operation,
    server_handler::fault_descriptor& _descr) {
    int err_no = 0;

    try {
        if (!static_server_instance.check_for_fault(_operation)) {
            return 0;
        }

        _descr = static_server_instance.get_fault(_operation);
        // randomly skip the fault evaluation
        if(check_for_random_fault(_descr.probability)) {
            return 0;
        }

        if(!_descr.regexp.empty()) {
            std::regex r(_descr.regexp);
            if (!std::regex_match(_path, r)) {
                return 0;
            }
        }

        if(_descr.err_no) {
            err_no = _descr.err_no;
        }
        else if(_descr.random) {
            err_no = get_random_err_no();
        }

        uint32_t delay = 0;
        if (_descr.delay_us) {
            delay = _descr.delay_us;
        }

        if (_descr.auto_delay) {
            // FIXME currently a no-op?
            delay = 0;
        }

        if (delay) {
            std::this_thread::sleep_for(
                std::chrono::microseconds(delay));
        }

        if (_descr.kill_caller) {
            static struct fuse_context *context;
            context = fuse_get_context();
            kill(context->pid, SIGKILL);
            return 0;
        }
    }
    catch(const std::out_of_range& _e) {
        err_log << __FUNCTION__ 
                << " operation is not in the fault map ["
                << _operation << "]" << std::endl; 
    }

    return -err_no;

} // evaluate_fault_for_operation_impl

int evaluate_fault_for_operation(
    const std::string& _path,
    const std::string& _operation) {

    server_handler::fault_descriptor fd;
    return evaluate_fault_for_operation_impl(
               _path,
               _operation,
               fd);
} // evaluate_fault_for_operation

int evaluate_fault_for_operation(
    const std::string& _path,
    const std::string& _operation,
    bool&              _corrupt_flag) {
    _corrupt_flag = false;

    server_handler::fault_descriptor fd;
    int err = evaluate_fault_for_operation_impl(
                 _path,
                 _operation,
                 fd);
    if(err) {
        return err;
    }

    if(fd.corrupt_data &&
       ("write" == _operation || 
        "read"  == _operation)) {
        _corrupt_flag = true;
    }

    if(fd.corrupt_size && 
       "getattr"  == _operation) {
        _corrupt_flag = true;
    }

    return 0;
} // evaluate_fault_for_operation

void server_thread_executor() {
    try {
        typedef message_broker::data_type data_t;
        message_broker bro("ZMQ_REP");
        bro.bind("tcp://*:9000");

        bool exit_flg = false;
        while(!exit_flg) {
            data_t msg;
            bro.receive(msg, ZMQ_DONTWAIT, true);
            if(msg.size() > 0) {
                if(QUIT_MSG == msg) {
                    bro.send(ACK_MSG);
                    break;
                } // if quit

                static_server_instance.process_message(msg);
                bro.send(ACK_MSG);

            } // if msg
        } // while
    }
    catch( const message_broker::exception& _e) {
        err_log << __FUNCTION__ << " [" << _e.what() << "]" << std::endl;
    }
} // server_thread_executor

static std::unique_ptr<std::thread> static_server_thread;
void start_server_thread() {
    err_log.open("/tmp/mungefs_log.txt", std::fstream::in|std::fstream::out|std::fstream::app);
    static_server_thread = std::make_unique<std::thread>(server_thread_executor);
} // start_server_thread

void stop_server_thread() {
    // TODO - send a message to the zmq listener to exit
    try {
        typedef message_broker::data_type data_t;
        message_broker bro("ZMQ_REQ");
        bro.connect("tcp://localhost:9000");
        bro.send(QUIT_MSG);
        
        data_t rcv_msg;
        bro.receive(rcv_msg);
    }
    catch( const message_broker::exception& _e) {
        err_log << __FUNCTION__ << " [" << _e.what() << "]" << std::endl;
    }
    
    static_server_thread->join();
    err_log.close();

} // stop_server_thread



