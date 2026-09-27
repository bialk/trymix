#include "storagestreamyaml.h"
#include "base64encoder.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <utility>

namespace sV2 {

struct StorageStreamSimpleYaml::Node {
  enum Kind { Undetermined, Scalar, Mapping, Sequence } kind = Undetermined;
  std::string name;
  std::string value;
  bool binary = false;
  std::vector<std::unique_ptr<Node>> children;
};

struct StorageStreamSimpleYaml::Event {
  StreamItemType type;
  std::string name;
  std::string value;
  bool binary = false;
};

namespace {

std::string trim(const std::string& text) {
  const auto begin = text.find_first_not_of(" \t\r");
  if (begin == std::string::npos) return {};
  const auto end = text.find_last_not_of(" \t\r");
  return text.substr(begin, end - begin + 1);
}

std::string quoteYaml(const std::string& text) {
  std::string result = "\"";
  for (unsigned char c : text) {
    switch (c) {
    case '\\': result += "\\\\"; break;
    case '"': result += "\\\""; break;
    case '\n': result += "\\n"; break;
    case '\r': result += "\\r"; break;
    case '\t': result += "\\t"; break;
    default:
      if (c < 0x20) {
        char escaped[7];
        std::snprintf(escaped, sizeof(escaped), "\\u%04x", c);
        result += escaped;
      } else {
        result.push_back(static_cast<char>(c));
      }
    }
  }
  result.push_back('"');
  return result;
}

int hexDigit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

std::string unquoteYaml(const std::string& input) {
  const std::string text = trim(input);
  if (text.size() < 2 || (text.front() != '"' && text.front() != '\'')) return text;
  const char quote = text.front();
  std::string result;
  for (size_t i = 1; i + 1 < text.size(); ++i) {
    char c = text[i];
    if (quote == '\'' && c == '\'' && i + 1 < text.size() - 1 && text[i + 1] == '\'') {
      result.push_back('\'');
      ++i;
    } else if (quote == '"' && c == '\\' && i + 1 < text.size() - 1) {
      c = text[++i];
      switch (c) {
      case 'n': result.push_back('\n'); break;
      case 'r': result.push_back('\r'); break;
      case 't': result.push_back('\t'); break;
      case '"': result.push_back('"'); break;
      case '\\': result.push_back('\\'); break;
      case 'u': {
        unsigned value = 0;
        bool valid = i + 4 < text.size();
        for (int n = 0; valid && n < 4; ++n) {
          const int digit = hexDigit(text[i + 1 + n]);
          valid = digit >= 0;
          value = (value << 4) | static_cast<unsigned>(std::max(digit, 0));
        }
        if (valid && value <= 0x7f) {
          result.push_back(static_cast<char>(value));
          i += 4;
        } else {
          result += "\\u";
        }
        break;
      }
      default: result.push_back(c); break;
      }
    } else {
      result.push_back(c);
    }
  }
  return result;
}

size_t findColon(const std::string& text) {
  bool single = false, dual = false, escaped = false;
  for (size_t i = 0; i < text.size(); ++i) {
    const char c = text[i];
    if (escaped) { escaped = false; continue; }
    if (dual && c == '\\') { escaped = true; continue; }
    if (!dual && c == '\'') single = !single;
    else if (!single && c == '"') dual = !dual;
    else if (!single && !dual && c == ':') return i;
  }
  return std::string::npos;
}

struct Line { int indent; std::string text; };

std::unique_ptr<StorageStreamSimpleYaml::Node> parseBlock(
    const std::vector<Line>& lines, size_t& at, int indent, std::string name = {}) {
  auto node = std::make_unique<StorageStreamSimpleYaml::Node>();
  node->name = std::move(name);
  if (at >= lines.size() || lines[at].indent < indent) return node;
  node->kind = lines[at].text == "-" || lines[at].text.rfind("- ", 0) == 0
                 ? StorageStreamSimpleYaml::Node::Sequence
                 : StorageStreamSimpleYaml::Node::Mapping;

  while (at < lines.size() && lines[at].indent == indent) {
    const std::string text = lines[at].text;
    if (node->kind == StorageStreamSimpleYaml::Node::Sequence) {
      if (text.empty() || text[0] != '-') break;
      std::string rest = trim(text.substr(1));
      ++at;
      std::unique_ptr<StorageStreamSimpleYaml::Node> child;
      if (rest.empty() && at < lines.size() && lines[at].indent > indent)
        child = parseBlock(lines, at, lines[at].indent, "item");
      else if (findColon(rest) != std::string::npos) {
        // Compact mapping sequence item: - "key": value
        child = std::make_unique<StorageStreamSimpleYaml::Node>();
        child->name = "item";
        child->kind = StorageStreamSimpleYaml::Node::Mapping;

        const size_t colon = findColon(rest);
        auto first = std::make_unique<StorageStreamSimpleYaml::Node>();
        first->name = unquoteYaml(rest.substr(0, colon));
        std::string value = trim(rest.substr(colon + 1));
        if (value.empty() && at < lines.size() && lines[at].indent > indent + 2) {
          first = parseBlock(lines, at, lines[at].indent, first->name);
        } else if (value == "{}" || value == "[]") {
          first->kind = value == "[]" ? StorageStreamSimpleYaml::Node::Sequence
                                       : StorageStreamSimpleYaml::Node::Mapping;
        } else {
          first->kind = StorageStreamSimpleYaml::Node::Scalar;
          if (value.rfind("!!binary", 0) == 0) {
            first->binary = true;
            value = trim(value.substr(8));
          }
          first->value = unquoteYaml(value);
        }
        child->children.push_back(std::move(first));

        if (at < lines.size() && lines[at].indent == indent + 2) {
          auto remainder = parseBlock(lines, at, indent + 2);
          for (auto& item : remainder->children)
            child->children.push_back(std::move(item));
        }
      }
      else {
        child = std::make_unique<StorageStreamSimpleYaml::Node>();
        child->name = "item";
        child->kind = StorageStreamSimpleYaml::Node::Scalar;
        if (rest.rfind("!!binary", 0) == 0) {
          child->binary = true;
          rest = trim(rest.substr(8));
        }
        child->value = unquoteYaml(rest);
      }
      node->children.push_back(std::move(child));
      continue;
    }

    const size_t colon = findColon(text);
    if (colon == std::string::npos) { ++at; continue; }
    const std::string childName = unquoteYaml(text.substr(0, colon));
    std::string rest = trim(text.substr(colon + 1));
    ++at;
    std::unique_ptr<StorageStreamSimpleYaml::Node> child;
    if (rest.empty() && at < lines.size() && lines[at].indent > indent) {
      child = parseBlock(lines, at, lines[at].indent, childName);
    } else if (rest == "{}" || rest == "[]") {
      child = std::make_unique<StorageStreamSimpleYaml::Node>();
      child->name = childName;
      child->kind = rest == "[]" ? StorageStreamSimpleYaml::Node::Sequence
                                  : StorageStreamSimpleYaml::Node::Mapping;
    } else {
      child = std::make_unique<StorageStreamSimpleYaml::Node>();
      child->name = childName;
      child->kind = StorageStreamSimpleYaml::Node::Scalar;
      if (rest.rfind("!!binary", 0) == 0) {
        child->binary = true;
        rest = trim(rest.substr(8));
      }
      child->value = unquoteYaml(rest);
    }
    node->children.push_back(std::move(child));
  }
  return node;
}

void emitYaml(const StorageStreamSimpleYaml::Node& node, int indent, std::string& out,
              bool sequenceItem = false) {
  const std::string pad(static_cast<size_t>(indent), ' ');
  if (sequenceItem) {
    out += pad + "-";
    if (node.kind == StorageStreamSimpleYaml::Node::Scalar) {
      out += node.binary ? " !!binary " : " ";
      out += node.binary ? node.value : quoteYaml(node.value);
      out += '\n';
      return;
    }
    if (node.kind == StorageStreamSimpleYaml::Node::Mapping && !node.children.empty()) {
      const auto& first = *node.children.front();
      out += " " + quoteYaml(first.name) + ":";
      if (first.kind == StorageStreamSimpleYaml::Node::Scalar) {
        out += first.binary ? " !!binary " : " ";
        out += first.binary ? first.value : quoteYaml(first.value);
        out += '\n';
      } else if (first.children.empty()) {
        out += first.kind == StorageStreamSimpleYaml::Node::Sequence ? " []\n" : " {}\n";
      } else {
        out += '\n';
        for (const auto& child : first.children)
          emitYaml(*child, indent + 4, out,
                   first.kind == StorageStreamSimpleYaml::Node::Sequence);
      }
      for (size_t i = 1; i < node.children.size(); ++i)
        emitYaml(*node.children[i], indent + 2, out);
      return;
    }
    out += '\n';
  } else if (!node.name.empty()) {
    out += pad + quoteYaml(node.name) + ":";
    if (node.kind == StorageStreamSimpleYaml::Node::Scalar) {
      out += node.binary ? " !!binary " : " ";
      out += node.binary ? node.value : quoteYaml(node.value);
      out += '\n';
      return;
    }
    if (node.children.empty()) {
      out += node.kind == StorageStreamSimpleYaml::Node::Sequence ? " []\n" : " {}\n";
      return;
    }
    out += '\n';
  }
  const int childIndent = indent + (node.name.empty() && !sequenceItem ? 0 : 2);
  for (const auto& child : node.children)
    emitYaml(*child, childIndent, out, node.kind == StorageStreamSimpleYaml::Node::Sequence);
}

void appendEvents(const StorageStreamSimpleYaml::Node& node,
                  std::vector<StorageStreamSimpleYaml::Event>& events) {
  if (!node.name.empty())
    events.push_back({StorageStreamFormatter::StartNode, node.name, {}, false});
  if (node.kind == StorageStreamSimpleYaml::Node::Scalar)
    events.push_back({StorageStreamFormatter::StringType, {}, node.value, node.binary});
  else
    for (const auto& child : node.children) appendEvents(*child, events);
  if (!node.name.empty())
    events.push_back({StorageStreamFormatter::EndNode, node.name, {}, false});
}

} // namespace

StorageStreamSimpleYaml::StorageStreamSimpleYaml(StreamMedia* sm) : m_streamMedia(sm) {}
StorageStreamSimpleYaml::~StorageStreamSimpleYaml() { flushOutput(); }

const char* StorageStreamSimpleYaml::GetNodeName() { return m_nodeName.c_str(); }

void StorageStreamSimpleYaml::loadInput() {
  if (m_root) return;
  m_readMode = true;
  std::string input;
  char buffer[4096];
  for (;;) {
    const size_t count = m_streamMedia->read(buffer, sizeof(buffer));
    if (count == 0) break;
    input.append(buffer, count);
  }
  std::vector<Line> lines;
  std::istringstream stream(input);
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    size_t first = line.find_first_not_of(' ');
    if (first == std::string::npos) continue;
    std::string content = line.substr(first);
    if (content.empty() || content[0] == '#' || content == "---" || content == "...") continue;
    lines.push_back({static_cast<int>(first), std::move(content)});
  }
  size_t at = 0;
  m_root = lines.empty() ? std::make_unique<Node>() : parseBlock(lines, at, lines.front().indent);
  for (const auto& child : m_root->children) appendEvents(*child, m_events);
}

StorageStreamFormatter::StreamItemType StorageStreamSimpleYaml::NextItem() {
  loadInput();
  if (m_eventOffset >= m_events.size()) return EndNode;
  const Event& event = m_events[m_eventOffset++];
  m_nodeName = event.name;
  m_value = event.value;
  m_valueIsBinary = event.binary;
  return event.type;
}

void StorageStreamSimpleYaml::GetItem(int* v) { *v = std::atoi(m_value.c_str()); }
void StorageStreamSimpleYaml::GetItem(float* v) { *v = std::strtof(m_value.c_str(), nullptr); }
void StorageStreamSimpleYaml::GetItem(double* v) { *v = std::strtod(m_value.c_str(), nullptr); }
void StorageStreamSimpleYaml::GetItem(char const** v) { *v = m_value.c_str(); }
void StorageStreamSimpleYaml::GetItem(void const** v, size_t* n) {
  m_binaryValue.assign(m_value.begin(), m_value.end());
  if (m_valueIsBinary) {
    m_binaryValue.push_back('\0');
    char* begin = m_binaryValue.data();
    char* end = decodeBase64InPlace(begin, begin + m_value.size());
    m_binaryValue.resize(static_cast<size_t>(end - begin));
  }
  *v = m_binaryValue.empty() ? nullptr : m_binaryValue.data();
  *n = m_binaryValue.size();
}

void StorageStreamSimpleYaml::PutStartNode(const char* s) {
  auto node = std::make_unique<Node>();
  node->name = s;
  node->kind = std::strcmp(s, "vector") == 0 ? Node::Sequence : Node::Undetermined;
  Node* raw = node.get();
  if (m_writeStack.empty()) {
    m_root = std::move(node);
  } else if (node->kind == Node::Sequence) {
    // "vector" is an internal serializer node. Turn its parent into YAML sequence.
    m_writeStack.back()->kind = Node::Sequence;
    raw = m_writeStack.back();
  } else {
    Node* parent = m_writeStack.back();
    if (parent->kind == Node::Undetermined) parent->kind = Node::Mapping;
    parent->children.push_back(std::move(node));
  }
  m_writeStack.push_back(raw);
}

void StorageStreamSimpleYaml::PutEndNode(const char* s) {
  if (m_writeStack.empty()) return;
  if (std::strcmp(s, "vector") == 0 && m_writeStack.size() >= 2 &&
      m_writeStack.back() == m_writeStack[m_writeStack.size() - 2]) {
    m_writeStack.pop_back();
    return;
  }
  if (m_writeStack.back()->kind == Node::Undetermined) m_writeStack.back()->kind = Node::Mapping;
  m_writeStack.pop_back();
  if (m_writeStack.empty()) flushOutput();
}

void StorageStreamSimpleYaml::putScalar(std::string value, bool binary) {
  if (m_writeStack.empty()) return;
  Node* node = m_writeStack.back();
  node->kind = Node::Scalar;
  node->value = std::move(value);
  node->binary = binary;
}
void StorageStreamSimpleYaml::PutItem(int* v) { putScalar(std::to_string(*v)); }
void StorageStreamSimpleYaml::PutItem(float* v) {
  char value[64]; std::snprintf(value, sizeof(value), "%.9g", static_cast<double>(*v)); putScalar(value);
}
void StorageStreamSimpleYaml::PutItem(double* v) {
  char value[64]; std::snprintf(value, sizeof(value), "%.17g", *v); putScalar(value);
}
void StorageStreamSimpleYaml::PutItem(const char* v) { putScalar(v ? v : ""); }
void StorageStreamSimpleYaml::PutItem(void const* v, size_t n) {
  const char* begin = static_cast<const char*>(v);
  putScalar(toBase64(begin, begin + n), true);
}

void StorageStreamSimpleYaml::flushOutput() {
  if (m_outputFlushed || m_readMode || !m_root || !m_streamMedia) return;
  std::string output = "---\n";
  emitYaml(*m_root, 0, output);
  output += "...\n";
  m_streamMedia->write(output.data(), output.size());
  m_outputFlushed = true;
}

} // namespace sV2
