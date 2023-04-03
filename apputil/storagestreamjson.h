#ifndef STORAGESTREAMJSON_H
#define STORAGESTREAMJSON_H


#include "serializerV2.h"

#include <stack>
#include <functional>

namespace sV2{

  class StorageStreamSimpleJson: public StorageStream{
  public:
     StorageStreamSimpleJson(StreamMedia* sm);
     ~StorageStreamSimpleJson();

  protected:
     const char* GetNodeName() override;
     int NextItem() override;
     void GetItem(int* v) override;
     void GetItem(float* v) override;
     void GetItem(double* v) override;
     void GetItem(char const** v) override;
     void GetItem(void const** v, size_t* n) override;


     void PutStartNode(const char *s) override;
     void PutEndNode(const char *s) override;

     void PutItem(int* v) override;
     void PutItem(float* v) override;
     void PutItem(double* v) override;
     void PutItem(const char* v) override;
     void PutItem(void const* v, size_t n) override;

  public:

     enum ParserState{
       InString = 0,
       InStringSlash,
       InToken,
       InSpace,
     };

     ParserState pState = InSpace;

     std::string whitespace = " \t\n\r";
     std::string specialsymbol = "{}[]:,";

     std::deque<std::string> found_tokens;
     std::string token;

     char buffer[1024];
     size_t bufferCounter=0;
     size_t position = 0;

     std::vector<std::function<void(char c)>> l1gram;
     std::string nextTocken();
     std::function<char()> readSymbol;
     std::string nodename;
     std::string value;
     std::pair<char,char> symbols{' ',' '};
     char prevType=0;
     //std::stack<std::string> tokens_stack;
     std::deque<int> found_types;


  private:


    static char* decodeBase64InPlace(char* begin, char* end);

    static std::string toBase64(char const* begin, char const* end);

    StreamMedia* m_streamMedia{nullptr};

    int tabsz;
    std::string indent;
    char* strbegin;
    char* strend;

    struct Context{
      int itemCount = 0;
      std::string nodeName;
      bool isVector = false;
    };

    std::stack<Context> context;

    static char const m_base64EncodeChars[];
    static signed char const m_base64DecodeChars[];

    std::vector<char> buf;
    size_t bufBeginOff;
    size_t bufEndOff;
  };

}

#endif // STORAGESTREAMJSON_H
