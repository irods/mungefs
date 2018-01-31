
#include <iostream>

#include "message_broker.hpp"
#include "mungefs_ctl.hpp"

#include "boost/program_options.hpp"
#include "boost/any.hpp"
#include "boost/algorithm/string.hpp"



void print_ctl(const mungefs_ctl& _ctl) {
    std::cout << std::endl << std::endl
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
    std::cout << "operation: "<< std::endl;
    std::cout << "     ";
    for(auto m : _ctl.operations) {
        std::cout << m << ", ";
    }
    std::cout << std::endl;
}









template <typename T>
int usage(T& _os) {
    _os << "--help : show command usage" << std::endl;
    _os << "--operations : list of operations to apply a fault" << std::endl;
    _os << "--random : randomize error injection" << std::endl;
    _os << "--err_no : error number to force" << std::endl;
    _os << "--probability : 0-100 probability of random error to inject" << std::endl;
    _os << "--regexp : regexp matching operations" << std::endl;
    _os << "--kill_caller : kill the calling process" << std::endl;
    _os << "--delay_us : delay a method by a given number of microsecods"<< std::endl;
    _os << "--auto_delay : set delay to simulate ssd" << std::endl;
    _os << "--corrupt_data : corrupt read or write data" << std::endl;
    _os << "--corrupt_size : report an invalid file size" << std::endl;
    return 1;
}

int parse_program_options(
    int          _argc,
    char*        _argv[],
    mungefs_ctl& _ctl_out ) {
    namespace po = boost::program_options;

    po::options_description opt_desc( "options" );
    opt_desc.add_options()
    ( "help,h", "show command usage" )
    ( "operations", po::value<std::string>(), "list of operations to apply a a fault")
    ( "random", "randomize error injection" )
    ( "err_no", po::value<int>(), "error number to force" )
    ( "probability", po::value<long>(), "0-100 probability of random error to inject" )
    ( "regexp", po::value<std::string>(), "regexp matching operations" )
    ( "kill_caller", "kill the calling process" )
    ( "delay_us", po::value<long>(), "delay a method by a given number of microsecods")
    ( "auto_delay", "set delay to simulate ssd" )
    ( "corrupt_data", "corrupt read or write data" )
    ( "corrupt_size", "report an invalid file size" );

    po::variables_map vm;
    try {
        po::store(
            po::command_line_parser(
                _argc, _argv ).options(
                opt_desc ).run(), vm );
        po::notify( vm );
    }
    catch(const po::error& _e ) {
        std::cerr << std::endl
                  << "Error: "
                  << _e.what()
                  << std::endl
                  << std::endl;
        return usage(std::cerr);
    }

    if(vm.count("operations")) {
        try {
            boost::split(
                _ctl_out.operations,
                vm[ "operations" ].as<std::string>(),
                boost::is_any_of( ", " ),
                boost::token_compress_on );
        }
        catch ( const boost::bad_function_call& ) {
            std::cerr << "boost threw bad_function_call on split." << std::endl;
            return usage(std::cerr);
        }
    }
    else {
        return usage(std::cerr);
    }

    _ctl_out.random       = (vm.count("random") > 0);
    _ctl_out.kill_caller  = (vm.count("kill_caller") > 0);
    _ctl_out.auto_delay   = (vm.count("auto_delay") > 0);
    _ctl_out.corrupt_data = (vm.count("corrupt_data") > 0);
    _ctl_out.corrupt_size = (vm.count("corrupt_size") > 0);

    if(vm.count("err_no")) {
        _ctl_out.err_no = vm["err_no"].as<int>();
    }

    if(vm.count("probability")) {
        _ctl_out.probability = vm["probability"].as<long>();
    }

    if(vm.count("regexp")) {
        _ctl_out.regexp = vm["regexp"].as<std::string>();
    }

    if(vm.count("delay_us")) {
        _ctl_out.delay_us = vm["delay_us"].as<long>();
    }

    return 0;
} // parse_program_options

int main(
    int   _argc,
    char* _argv[]) {
    typedef message_broker::data_type data_t;
   
    try { 
        mungefs_ctl ctl;
        int err = parse_program_options(_argc, _argv, ctl);
        if(err) {
            return err;
        }

        print_ctl(ctl);

        auto out = avro::memoryOutputStream();
        auto enc = avro::binaryEncoder();
        enc->init( *out );
        avro::encode( *enc, ctl );
        auto data = avro::snapshot( *out );

        message_broker bro("ZMQ_REQ");
        bro.connect("tcp://localhost:9000");
        bro.send(*data);

        data_t msg;
        bro.receive(msg);
        
        std::cout << msg << std::endl;
    }
    catch( const message_broker::exception& _e) {
        std::cerr << _e.what() << std::endl;
        return 1;
    }

    return 0;
}



