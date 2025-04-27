#ifndef ETHICS_H
#define ETHICS_H

#include <string>

void initializeAuth();
bool authenticateUser(const std::string& username, const std::string& password);

#endif // ETHICS_H
