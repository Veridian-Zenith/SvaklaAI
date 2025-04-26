#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class WebScraper {
public:
  struct ScrapingResult {
    std::string url;
    std::string content;
    std::vector<std::string> metadata;
    bool success;
    std::string error;
  };

  WebScraper();
  ~WebScraper();

  bool initialize();
  ScrapingResult scrapeURL(const std::string &url);
  bool setupFirefoxInstance(bool headless = true);
  void setUserAgent(const std::string &agent);
  void setProxy(const std::string &proxy);
  void addCustomHeader(const std::string &name, const std::string &value);

  // Callback for when permission is needed
  using PermissionCallback =
      std::function<bool(const std::string &url, const std::string &reason)>;
  void setPermissionCallback(PermissionCallback callback);

private:
  bool initialized = false;
  std::string userAgent;
  std::optional<std::string> proxySettings;
  std::vector<std::pair<std::string, std::string>> customHeaders;
  PermissionCallback permissionCallback;

  void *firefoxInstance; // Will be properly typed when Firefox automation is
                         // implemented
};
