#include "chat_storage.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>

using json = nlohmann::json;

ChatStorage::ChatStorage(const std::string &path)
    : dbPath(path), env(nullptr), initialized(false) {}

ChatStorage::~ChatStorage() {
  if (initialized) {
    closeDatabase();
  }
}

bool ChatStorage::initialize() {
  if (initialized)
    return true;

  try {
    if (!fs::exists(dbPath)) {
      fs::create_directories(dbPath);
    }

    if (mdb_env_create(&env) != 0) {
      throw std::runtime_error("Failed to create LMDB environment");
    }

    // Set max DB size to 10GB
    if (mdb_env_set_mapsize(env, 10ULL * 1024 * 1024 * 1024) != 0) {
      throw std::runtime_error("Failed to set database size");
    }

    if (mdb_env_open(env, dbPath.c_str(), 0, 0644) != 0) {
      throw std::runtime_error("Failed to open database");
    }

    MDB_txn *txn;
    if (mdb_txn_begin(env, nullptr, 0, &txn) != 0) {
      throw std::runtime_error("Failed to begin transaction");
    }

    if (mdb_dbi_open(txn, nullptr, 0, &dbi) != 0) {
      mdb_txn_abort(txn);
      throw std::runtime_error("Failed to open database");
    }

    if (mdb_txn_commit(txn) != 0) {
      throw std::runtime_error("Failed to commit transaction");
    }

    initialized = true;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize chat storage: " << e.what() << std::endl;
    if (env) {
      mdb_env_close(env);
      env = nullptr;
    }
    return false;
  }
}

bool ChatStorage::storeMessage(const ChatMessage &message) {
  if (!initialized)
    return false;

  try {
    // Convert message to JSON
    json j = {
        {"id", message.id},
        {"content", message.content},
        {"sender", message.sender},
        {"timestamp", std::chrono::system_clock::to_time_t(message.timestamp)},
        {"tags", message.tags}};

    std::string value = j.dump();

    MDB_txn *txn;
    if (mdb_txn_begin(env, nullptr, 0, &txn) != 0) {
      throw std::runtime_error("Failed to begin transaction");
    }

    MDB_val key, data;
    key.mv_size = message.id.size();
    key.mv_data = (void *)message.id.c_str();
    data.mv_size = value.size();
    data.mv_data = (void *)value.c_str();

    if (mdb_put(txn, dbi, &key, &data, 0) != 0) {
      mdb_txn_abort(txn);
      throw std::runtime_error("Failed to store message");
    }

    if (mdb_txn_commit(txn) != 0) {
      throw std::runtime_error("Failed to commit transaction");
    }

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to store message: " << e.what() << std::endl;
    return false;
  }
}

ChatMessage ChatStorage::retrieveMessage(const std::string &id) {
  ChatMessage message;
  if (!initialized)
    return message;

  try {
    MDB_txn *txn;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) {
      throw std::runtime_error("Failed to begin transaction");
    }

    MDB_val key, data;
    key.mv_size = id.size();
    key.mv_data = (void *)id.c_str();

    if (mdb_get(txn, dbi, &key, &data) == 0) {
      std::string value(static_cast<char *>(data.mv_data), data.mv_size);
      json j = json::parse(value);

      message.id = j["id"];
      message.content = j["content"];
      message.sender = j["sender"];
      message.timestamp =
          std::chrono::system_clock::from_time_t(j["timestamp"]);
      message.tags = j["tags"].get<std::vector<std::string>>();
    }

    mdb_txn_abort(txn);
  } catch (const std::exception &e) {
    std::cerr << "Failed to retrieve message: " << e.what() << std::endl;
  }

  return message;
}

std::vector<ChatMessage>
ChatStorage::retrieveConversation(const std::string &conversationId) {
  std::vector<ChatMessage> messages;
  if (!initialized)
    return messages;

  try {
    MDB_txn *txn;
    if (mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn) != 0) {
      throw std::runtime_error("Failed to begin transaction");
    }

    MDB_cursor *cursor;
    if (mdb_cursor_open(txn, dbi, &cursor) != 0) {
      mdb_txn_abort(txn);
      throw std::runtime_error("Failed to open cursor");
    }

    MDB_val key, data;
    while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0) {
      std::string value(static_cast<char *>(data.mv_data), data.mv_size);
      json j = json::parse(value);

      if (j["id"].get<std::string>().starts_with(conversationId)) {
        ChatMessage msg;
        msg.id = j["id"];
        msg.content = j["content"];
        msg.sender = j["sender"];
        msg.timestamp = std::chrono::system_clock::from_time_t(j["timestamp"]);
        msg.tags = j["tags"].get<std::vector<std::string>>();
        messages.push_back(msg);
      }
    }

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
  } catch (const std::exception &e) {
    std::cerr << "Failed to retrieve conversation: " << e.what() << std::endl;
  }

  return messages;
}

bool ChatStorage::clearMemory(bool keepLogs) {
  if (!initialized)
    return false;

  try {
    if (!keepLogs) {
      MDB_txn *txn;
      if (mdb_txn_begin(env, nullptr, 0, &txn) != 0) {
        throw std::runtime_error("Failed to begin transaction");
      }

      if (mdb_drop(txn, dbi, 0) != 0) {
        mdb_txn_abort(txn);
        throw std::runtime_error("Failed to clear database");
      }

      if (mdb_txn_commit(txn) != 0) {
        throw std::runtime_error("Failed to commit transaction");
      }
    }
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to clear memory: " << e.what() << std::endl;
    return false;
  }
}

void ChatStorage::compactStorage() {
  if (!initialized)
    return;

  try {
    if (mdb_env_copy2(env, (dbPath + ".compact").c_str(), MDB_CP_COMPACT) ==
        0) {
      closeDatabase();
      fs::remove_all(dbPath);
      fs::rename(dbPath + ".compact", dbPath);
      initialize();
    }
  } catch (const std::exception &e) {
    std::cerr << "Failed to compact storage: " << e.what() << std::endl;
  }
}

std::string ChatStorage::generateMessageId() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 15);

  std::stringstream ss;
  ss << std::hex;

  for (int i = 0; i < 16; ++i) {
    ss << dis(gen);
  }

  return ss.str();
}

void ChatStorage::closeDatabase() {
  if (initialized) {
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
    initialized = false;
  }
}

void ChatStorage::store_message(const std::string &session_id,
                                const std::string &role,
                                const std::string &content) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Create session if it doesn't exist
  if (chat_history_.find(session_id) == chat_history_.end()) {
    chat_history_[session_id] = std::vector<Message>();
  }

  // Add message to history
  chat_history_[session_id].push_back({role, content, std::time(nullptr)});

  // Persist to disk
  save_to_disk(session_id);
}

std::vector<Message> ChatStorage::get_history(const std::string &session_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  return chat_history_[session_id];
}

void ChatStorage::save_to_disk(const std::string &session_id) {
  // Create chat directory if it doesn't exist
  std::filesystem::create_directories("chat_history");

  std::string filename = "chat_history/" + session_id + ".json";
  std::ofstream file(filename);

  json j;
  j["session_id"] = session_id;
  j["messages"] = chat_history_[session_id];

  file << j.dump(2);
}
