#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <map>
#include <boost/process.hpp>

//#####################################################################################
class ProcessWithBiPipes
{
public:
  enum ProcessStatus{
    running,
    failure
  };

  //#####################################################################################
  ProcessWithBiPipes(std::string const& commandLine, std::function<void(ProcessStatus status, std::string str)> response);

  //#####################################################################################
  ~ProcessWithBiPipes();

  //#####################################################################################
  bool send(std::string const& str, std::string& error_message);

private:

  // please follow correct order of initialization
  boost::asio::io_context io;
  boost::process::async_pipe in_pipe;  // Pipe for sending data to child
  boost::process::async_pipe out_pipe; // Pipe for receiving data from child
  boost::process::child c;
  std::optional<std::thread> read_thread;
};


//#####################################################################################
class ProcessAsServer
{
public:
  //#####################################################################################
  ProcessAsServer(std::string command);

  //#####################################################################################
  ~ProcessAsServer() = default;

  //#####################################################################################
  std::future<std::string> runCommand(nlohmann::json& j);

private:

  //#####################################################################################
  std::future<std::string> makeNewPromise(std::string const& corelationId);

  //#####################################################################################
  void fulfillPromise(std::string const& corelationId, std::string const& value);

  //#####################################################################################
  void breakAllPromises(std::string const& errorString);

  //#####################################################################################
  void splitToChunks(std::string const& string_in);

  //#####################################################################################
  void processChunk(std::string_view const& chunk);

  const std::string correlationIdTemplate = "asdfawle";
  const std::string messageSplitToken = "eof-json";
  std::atomic_size_t counter_id{0};
  std::mutex mutex;
  std::map<std::string, std::promise<std::string>> promises;
  ProcessWithBiPipes bidirChildProcess;
};

