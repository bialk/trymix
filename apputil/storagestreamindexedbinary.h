#ifndef STORAGESTREAMINDEXEDBINARY_H
#define STORAGESTREAMINDEXEDBINARY_H

#include "serializerV2.h"

namespace sV2{

class StorageStreamSimpleBinary:public StorageStream{
public:
  StorageStreamSimpleBinary(StreamMedia* sm);
  ~StorageStreamSimpleBinary();
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

private:
  StreamMedia* m_streamMedia{nullptr};

  // aux data type to keep current item
  char type; // 0 - start node, 1 - end node,
  // 2 - data node string, 3 - data node int, 4 - data node float,
  // 5 - data node double, 6 - data node blob
  double ddata;
  float fdata;
  int   idata;

  // These two are synchronized.
  std::uint16_t shortlen;
  std::uint32_t biglen;

  std::vector<char> strdata;
};


class StorageStreamIndexedBinary:public StorageStream{
public:
  StorageStreamIndexedBinary(StreamMedia* sm, StreamMedia* smi);
  ~StorageStreamIndexedBinary();

  int  ReadIndex();
  int  WriteIndex();

private:
  //index support structure
  int saveindex{0};
  std::string indexfilename;
  int idxcntr;
  StreamMedia* m_streamMedia{nullptr};
  StreamMedia* m_streamMediaIndex{nullptr};

  std::map<std::string, std::uint32_t> str2int;
  std::vector<const char*> int2str;

  // aux data type to keep current item
  std::uint8_t  type; // 0 - start node, 1 - end node,
  // 2 - data node string, 3 - data node int, 4 - data node float,
  // 5 - data node double, 6 - data node binary
  double ddata;
  float fdata;
  int   idata;

  // These two are synchronized.
  std::uint16_t shortlen;
  std::uint32_t biglen;

  std::vector<char> strdata;

  void Close();

protected:
  const char* GetNodeName() override;
  int NextItem() override;
  void GetItem(int* v) override;
  void GetItem(float* v) override;
  void GetItem(double* v) override;
  void GetItem(char const** v) override;
  void GetItem(void const** v, size_t *n) override;


  void PutStartNode(const char *s) override;
  void PutEndNode(const char *s) override;

  void PutItem(int* v) override;
  void PutItem(float* v) override;
  void PutItem(double* v) override;
  void PutItem(const char* v) override;
  void PutItem(void const* v, size_t n) override;
};

}

#endif // STORAGESTREAMINDEXEDBINARY_H
