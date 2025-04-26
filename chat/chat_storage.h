#pragma once

#include <chrono>
#include <filesystem>
#include <lmdb.h>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Message {
  std::string role;
  std::string content;
  std::time_t timestamp;
};

struct ChatMessage {
  std::string id;
  std::string content;
  std::string sender;
  std::chrono::system_clock::time_point timestamp;
  std::vector<std::string> tags;
};

class ChatStorage {
public:
  ChatStorage(const std::string &path = "chat_db");
  ~ChatStorage();

  bool initialize();
  bool storeMessage(const ChatMessage &message);
  ChatMessage retrieveMessage(const std::string &id);
  std::vector<ChatMessage>
  retrieveConversation(const std::string &conversationId);
  bool clearMemory(bool keepLogs = false);
  void compactStorage();

  // New methods
  void store_message(const std::string &session_id, const std::string &role,
                     const std::string &content);
  std::vector<Message> get_history(const std::string &session_id);
  void save_to_disk(const std::string &session_id);

  static std::string generateMessageId();

private:
  void closeDatabase();

  std::string dbPath;
  MDB_env *env;
  MDB_dbi dbi;
  bool initialized;

  std::map<std::string, std::vector<Message>> chat_history_;
  std::mutex mutex_;
};
