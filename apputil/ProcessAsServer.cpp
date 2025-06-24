#include "ProcessAsServer.h"

#include <iostream>
#include <functional>
#include <format>

//#####################################################################################
ProcessAsServer::ProcessAsServer(std::string command)
  : bidirChildProcess(command, [this](ProcessWithBiPipes::ProcessStatus status, std::string const message)
     {
        if(status == ProcessWithBiPipes::running)
          splitToChunks(message);
        else if(status == ProcessWithBiPipes::failure)
          breakAllPromises(message);
        else if(status == ProcessWithBiPipes::finished){
           breakAllPromises(std::string(std::format("{{\"exit_code\":\"{}\"}}", message)));
        }
     })
{}

//#####################################################################################
void ProcessAsServer::splitToChunks(std::string const& message)
{
  size_t curpos = 0;
  for(;;)
  {
    auto tokpos = message.find(messageSplitToken, curpos);
    if(tokpos != std::string::npos)
    {
      if(tokpos != curpos)
        // chunk: [curpos, tokpos]
        processChunk({message.data() + curpos, tokpos-curpos});

      curpos=tokpos+messageSplitToken.size();
    }
    else
    {
      if(message.size() > curpos)
        // chunk: (curpos, std::sring::size)
        processChunk({message.data() + curpos, message.size()-curpos});

      break;
    }
  }
}

//#####################################################################################
void ProcessAsServer::processChunk(std::string_view const& messageChunk)
{
  try
  {
    nlohmann::json j = nlohmann::json::parse(messageChunk);
    // std::cout << "No exceptoin: Valid JSON!" << std::endl;
    if(j.contains("correlationId"))
      fulfillPromise(j["correlationId"].get<std::string>(), std::string(messageChunk));
    else
      std::cout << messageChunk << std::endl;
  }
  catch (nlohmann::json::parse_error& /*e*/)
  {
    // if not json - just outputting message from process to the log
    std::cout << messageChunk << std::endl;
  }
}

//#####################################################################################
std::future<std::string> ProcessAsServer::runCommand(nlohmann::json& j)
{
  // generating new correlation id to match command with response
  auto id = counter_id++;
  std::string newCorrelationId = correlationIdTemplate + std::to_string(id);
  auto future = makeNewPromise(newCorrelationId);
  j["correlationId"]=newCorrelationId;
  std::string errorMessage;
  if(!bidirChildProcess.send(j.dump()+"\n", errorMessage))
  {
    fulfillPromise(newCorrelationId, std::format("falure: {}", errorMessage));
  }

  return future;
}

//#####################################################################################
std::future<std::string> ProcessAsServer::makeNewPromise(std::string const& corelationId)
{
  std::lock_guard<std::mutex> lock(promises_guard_mutex);
  return promises[corelationId].get_future();
}

//#####################################################################################
void ProcessAsServer::fulfillPromise(std::string const& correlationId, std::string const& value)
{
  std::lock_guard<std::mutex> lock(promises_guard_mutex);
  auto promiseIt = promises.find(correlationId);
  // we expect promise
  assert(promiseIt != promises.end());
  if(promiseIt == promises.end()){
    std::cout << "no promise found: Invalid correlationId!" << std::endl;
    return;
  }

  promiseIt->second.set_value(value);
  promises.erase(promiseIt);
}

//#####################################################################################
void ProcessAsServer::breakAllPromises (std::string const& value)
{
  std::lock_guard<std::mutex> lock(promises_guard_mutex);
  for(auto& p: promises)
    p.second.set_value(value);
  promises.clear();
}


