#ifndef STORAGESTREAMYAML_H
#define STORAGESTREAMYAML_H

#include "serializerV2.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace sV2 {

// A small YAML formatter for the data model used by Serializer.  It supports
// mappings, sequences, quoted/plain scalars and YAML's !!binary tag.
class StorageStreamSimpleYaml : public StorageStreamFormatter {
public:
  // Public only so the implementation's parser/emitter helpers can operate on
  // these internal representations; they are not part of the formatter API.
  struct Node;
  struct Event;

  explicit StorageStreamSimpleYaml(StreamMedia* sm);
  ~StorageStreamSimpleYaml() override;

protected:
  const char* GetNodeName() override;
  StreamItemType NextItem() override;
  void GetItem(int* v) override;
  void GetItem(float* v) override;
  void GetItem(double* v) override;
  void GetItem(char const** v) override;
  void GetItem(void const** v, size_t* n) override;

  void PutStartNode(const char* s) override;
  void PutEndNode(const char* s) override;
  void PutItem(int* v) override;
  void PutItem(float* v) override;
  void PutItem(double* v) override;
  void PutItem(const char* v) override;
  void PutItem(void const* v, size_t n) override;

private:
  void putScalar(std::string value, bool binary = false);
  void flushOutput();
  void loadInput();

  StreamMedia* m_streamMedia;
  std::unique_ptr<Node> m_root;
  std::vector<Node*> m_writeStack;
  bool m_outputFlushed = false;
  bool m_readMode = false;

  std::vector<Event> m_events;
  size_t m_eventOffset = 0;
  std::string m_nodeName;
  std::string m_value;
  bool m_valueIsBinary = false;
  std::vector<char> m_binaryValue;
};

} // namespace sV2

#endif // STORAGESTREAMYAML_H
