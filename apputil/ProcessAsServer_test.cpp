#include "processAsServer.h"

#ifdef off
void test_ProcessAsServer();
test_ProcessAsServer();
#endif

#include <QResource>
#include <QByteArray>

#include <fstream>
#include <filesystem>
#include <iostream>

//#####################################################################################
void test_ProcessAsServer()
{

  QResource python_sample_data(":/samples/apputil/ProcessAsServer_test_sample.py");
  std::string python_sample = QByteArray((char*)python_sample_data.data(),python_sample_data.size()).toStdString();

  enum test_type{
    normal,
    with_crash_before_exit,
    with_crash_after_exit
  };

  auto temp_path = std::filesystem::temp_directory_path();
  auto test_python_sample = (temp_path / "ProcessAsServer_test_sample.py").generic_string();
  std::ofstream file(test_python_sample);
  file << python_sample << std::endl;

  auto test = [&](test_type tt){
#ifdef __linux__
    ProcessAsServer pp(std::format(R"==(python3 -u {} )==", test_python_sample));
#elif defined _WIN32
    ProcessAsServer pp(std::format(R"==(python -u {} )==", test_python_sample));
#endif

    std::vector<std::future<std::string>> futuresArray;
    futuresArray.reserve(10);

    auto j = nlohmann::json::object();
    j["command"]="command_a";
    futuresArray.emplace_back(pp.runCommand(j));

    if(tt==with_crash_before_exit)
      (
      j = nlohmann::json::object(),
          j["command"]="crash",
          futuresArray.emplace_back(pp.runCommand(j))
          );

    j = nlohmann::json::object();
    j["command"]="exit";
    futuresArray.emplace_back(pp.runCommand(j));

    if(tt==with_crash_after_exit)
      (
      j = nlohmann::json::object(),
          j["command"]="crash",
          futuresArray.emplace_back(pp.runCommand(j))
          );

    for(auto& future: futuresArray){
      future.wait();
      std::cout << "future out:" << future.get() << std::endl;
    }
  };

  test(normal);
  test(with_crash_before_exit);
  test(with_crash_after_exit);
}

