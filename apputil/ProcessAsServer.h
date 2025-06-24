#pragma once

#include "ProcessWithBiPipes.h"
#include "nlohmann/json.hpp"

#include <string>
#include <map>
#include <mutex>


//#####################################################################################
class ProcessAsServer: private boost::noncopyable
{
public:
  ProcessAsServer() = delete;

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

  const std::string correlationIdTemplate = "asdfawle"; // unique token (can be anything unique)
  std::atomic_size_t counter_id{0};                     // incremental id to make correlation token unique
  const std::string messageSplitToken = "eof-json";     // unique token (separator for dedicated respond values)
  std::mutex promises_guard_mutex;
  std::map<std::string, std::promise<std::string>> promises;
  ProcessWithBiPipes bidirChildProcess;
};

