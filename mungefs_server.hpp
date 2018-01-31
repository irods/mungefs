
#ifndef MUNGEFS_SERVER_HPP
#define MUNGEFS_SERVER_HPP

#include <string>
int evaluate_fault_for_operation(
    const std::string& _path,
    const std::string& _operation);
int evaluate_fault_for_operation(
    const std::string& _path,
    const std::string& _operation,
    bool&              _corrupt_flag);
void start_server_thread();
void stop_server_thread();

#endif // MUNGEFS_SERVER_HPP


