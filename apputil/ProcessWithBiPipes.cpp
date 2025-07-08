#include "ProcessWithBiPipes.h"
#include <boost/process/v1/windows.hpp>

//#####################################################################################
ProcessWithBiPipes::ProcessWithBiPipes(std::string const& commandLine,
                                      std::function<void(ProcessStatus status, std::string str)> response)
  : child_process(commandLine,
      boost::process::std_in < in_pipe,
      (boost::process::std_out & boost::process::std_err) > out_pipe,
                  boost::process::windows::hide)
{
  //#####################################################################################
  read_thread = std::make_unique<std::thread>([&, response]()
  {
    auto read_pipe = [&]()
    {
      char strout[10024];
      size_t received = out_pipe.read(strout, sizeof(strout)-1); // reserve for zero (assuming data is textual)
      if(received != 0){
        strout[received] = 0; // terminating string with zero
        response(running, strout);
      }
    };

    for(;;)
    {
      if( !out_pipe.is_open())
      {
        response(failure, "read pipe error: pipe is not open");
      }

      try
      {
        read_pipe();
      }
      catch (const std::ios_base::failure& e)
      {
        response(failure,std::format("read pipe error - Stream error: {}", e.what()));
      } catch (const boost::system::system_error& e)
      {
        response(failure, std::format("read pipe error - Boost system error: {}", e.what()));
      } catch (const std::exception& e)
      {
        response(failure, std::format("read pipe error - General error: {}", e.what()));
      }
      catch (...)
      {
        response(failure, "Unknown exeptions in thread");
      }

      // exit if process is not running
      if( !child_process.running() )
      {
        read_pipe();
        response(finished,std::format("{}", child_process.exit_code()));
        break;
      }
    }
  });
}

//#####################################################################################
ProcessWithBiPipes::~ProcessWithBiPipes()
{
  // std::cout() << "Exitting..." << std::endl;
  std::error_code ec;
  child_process.wait(ec);
  // std::cout() << "child process wait: " << ec.message() << std::endl;
  read_thread->join();
  in_pipe.close();
  out_pipe.close();
  // std::cout() << "Exit code from process:" << child_process.exit_code() << std::endl;
}

//#####################################################################################
bool ProcessWithBiPipes::send(std::string const& str, std::string& error_message)
{
  if( !in_pipe.good())
  {
    error_message =  "write pipe exitting reason: pipe is not open";
    return false;
  }
  else if( !child_process.running())
  {
    error_message =  "write pipe exitting reason: child is not running";
    return false;
  }
  in_pipe << str;
  in_pipe.flush();
  if(in_pipe.fail())
  {
    error_message =  "write stream failure";
    return false;
  }
  return true;
}

