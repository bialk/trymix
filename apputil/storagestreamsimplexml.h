#ifndef STORAGESTREAMSIMPLEXML_H
#define STORAGESTREAMSIMPLEXML_H

#include "serializerV2.h"

namespace sV2 {

class StorageStreamSimpleXML: public StorageStreamFormatter{
public:
  StorageStreamSimpleXML(StreamMedia* sm);
  ~StorageStreamSimpleXML();

protected:

  const char* GetNodeName() override;
  StreamItemType NextItem() override;
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
private:

  bool nextLine(char** begin, char** end);

  bool readMore();

  StreamMedia* m_streamMedia{nullptr};

  int tabsz;
  std::string indent;
  char* strbegin;
  char* strend;

  std::vector<char> buf;
  size_t bufBeginOffset;
  size_t bufEndOffset;
};

} // end of namespase sV2

#endif //STORAGESTREAMSIMPLEXML_H
