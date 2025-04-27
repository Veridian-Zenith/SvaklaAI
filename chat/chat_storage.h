#ifndef CHAT_STORAGE_H
#define CHAT_STORAGE_H

#include <string>

void initializeChatStorage();
void saveChat(const std::string& chatName, const std::string& chatContent);
std::string loadChat(const std::string& chatName);

#endif // CHAT_STORAGE_H
