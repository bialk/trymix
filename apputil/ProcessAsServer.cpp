#include "ProcessAsServer.h"

// #include "lib_platform/Format.h"   // IWYU pragma: keep

#include <boost/process.hpp>
#include <iostream>
#include <functional>


//#####################################################################################
ProcessWithBiPipes::ProcessWithBiPipes(std::string const& commandLine, std::function<void(ProcessStatus status, std::string str)> response)
  : in_pipe(io)
  , out_pipe(io)
  , c(commandLine, boost::process::std_in < in_pipe, boost::process::std_out > out_pipe, io)
{
  read_thread.emplace( [&, response]()
  {
    for(;;)
    {
      boost::system::error_code ec;
      boost::asio::streambuf buffer(1024);
      if( !out_pipe.is_open())
      {
        response(failure, "read pipe exitting reason: pipe is not open");
        break;
      }
      // no need to check if process is running (as it may exit but pipe still has output
      // else if( !c.running()) {
      //   response(failure, "read pipe exitting reason: child is not running");
      //   break;
      // }

      size_t received = boost::asio::read(out_pipe, buffer, boost::asio::transfer_at_least(1), ec);
      if (ec.failed())
      {
        response(failure, std::format("read pipe exitting reason: {} ({}) ", ec.message(), ec.value()));
        break;
      }
      std::string strout{(char*)buffer.data().data(), received};
      response(running, strout);
    }
  });
}

//#####################################################################################
ProcessWithBiPipes::~ProcessWithBiPipes()
{
  std::cout << "Exitting..." << std::endl;
  try
  {
    c.wait();
    in_pipe.close();
    out_pipe.close();
    if (read_thread.has_value())
      read_thread->join();
  }
  catch(const boost::system::system_error& e)
  {
    std::cout << "child process termination exception:" << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "child process termination exception: unknown" << std::endl;
  }
  std::cout << "Exit code from process:" << c.exit_code() << std::endl;
}


bool ProcessWithBiPipes::send(std::string const& str, std::string& error_message)
{
  if( !in_pipe.is_open())
  {
    error_message =  "write pipe exitting reason: pipe is not open";
    return false;
  }
  else if( !c.running())
  {
    error_message =  "write pipe exitting reason: child is not running";
    return false;
  }
  boost::system::error_code ec;
  size_t sent = boost::asio::write(in_pipe,
                                   boost::asio::buffer(str, str.size()),
                                   boost::asio::transfer_at_least(str.size()),
                                   ec);
  if (ec.failed())
  {
    error_message =  std::format("write pipe exitting reason: {} ({})",ec.message(), ec.value());
    return false;
  }
  else if (sent != str.size())
  {
    error_message =  std::format("write pipe - not all bytes are sent != message: {}!={} ",sent, str.size());
    return false;
  }
  return true;
}



//#####################################################################################
ProcessAsServer::ProcessAsServer(std::string command)
  :bidirChildProcess(command, [this](ProcessWithBiPipes::ProcessStatus status, auto const& message)
     {
        if(status == ProcessWithBiPipes::running)
          splitToChunks(message);
        else if(status == ProcessWithBiPipes::failure)
          breakAllPromises(message);
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
      if(tokpos != curpos)
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
  std::lock_guard<std::mutex> lock(mutex);
  return promises[corelationId].get_future();
}

//#####################################################################################
void ProcessAsServer::fulfillPromise(std::string const& correlationId, std::string const& value)
{
  std::lock_guard<std::mutex> lock(mutex);
  auto promiseIt = promises.find(correlationId);
  // we expect promise
  assert(promiseIt != promises.end());
  if(promiseIt == promises.end()){
    std::cout << "no promisee found: Invalid correlationId!";
    return;
  }

  promiseIt->second.set_value(value);
  promises.erase(promiseIt);
}

//#####################################################################################
void ProcessAsServer::breakAllPromises (std::string const& value)
{
  std::lock_guard<std::mutex> lock(mutex);
  for(auto& p: promises)
    p.second.set_value(value);
  promises.clear();
}


