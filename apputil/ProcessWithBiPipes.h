#pragma once

#include <string>
#include <boost/process.hpp>

//#####################################################################################
class ProcessWithBiPipes
{
public:
  enum ProcessStatus{
    running,
    failure,
    finished
  };

  //#####################################################################################
  ProcessWithBiPipes(std::string const& commandLine, std::function<void(ProcessStatus status, std::string const str)> response);

  //#####################################################################################
  ~ProcessWithBiPipes();

  //#####################################################################################
  bool send(std::string const& str, std::string& error_message);

private:

  // please follow correct order of initialization
  boost::process::opstream in_pipe; // Pipe for sending data to child
  boost::process::pipe out_pipe; // Pipe for receiving data from child
  boost::process::child child_process;
  std::unique_ptr<std::thread> read_thread;
};

