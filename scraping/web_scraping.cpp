#include "web_scraping.h"
#include <chrono>
#include <curl/curl.h>
#include <dbus/dbus.h>
#include <iostream>
#include <thread>

// Callback function for cURL
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

WebScraper::WebScraper() : initialized(false), firefoxInstance(nullptr) {
  userAgent = "SvaklaAI/1.0 (Linux x86_64) Firefox/123.0";
}

WebScraper::~WebScraper() {
  if (firefoxInstance) {
    // TODO: Implement proper Firefox cleanup
    firefoxInstance = nullptr;
  }
}

bool WebScraper::initialize() {
  curl_global_init(CURL_GLOBAL_ALL);
  initialized = true;
  return true;
}

WebScraper::ScrapingResult WebScraper::scrapeURL(const std::string &url) {
  ScrapingResult result;
  result.url = url;
  result.success = false;

  if (!initialized) {
    result.error = "WebScraper not initialized";
    return result;
  }

  // Ask for permission if callback is set
  if (permissionCallback &&
      !permissionCallback(url, "Requesting permission to scrape URL")) {
    result.error = "Permission denied by user";
    return result;
  }

  CURL *curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.content);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // Apply proxy settings if set
    if (proxySettings) {
      curl_easy_setopt(curl, CURLOPT_PROXY, proxySettings->c_str());
    }

    // Apply custom headers
    struct curl_slist *headers = nullptr;
    for (const auto &[name, value] : customHeaders) {
      headers = curl_slist_append(headers, (name + ": " + value).c_str());
    }
    if (headers) {
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      result.error = curl_easy_strerror(res);
    } else {
      result.success = true;

      // Extract basic metadata
      char *contentType;
      curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
      if (contentType) {
        result.metadata.push_back("Content-Type: " + std::string(contentType));
      }

      double elapsed;
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
      result.metadata.push_back("Request-Time: " + std::to_string(elapsed) +
                                "s");
    }

    if (headers) {
      curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
  } else {
    result.error = "Failed to initialize cURL";
  }

  return result;
}

bool WebScraper::setupFirefoxInstance(bool headless) {
  if (!initialized)
    return false;

  // Connect to D-Bus session bus
  DBusError error;
  dbus_error_init(&error);

  DBusConnection *connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
  if (dbus_error_is_set(&error)) {
    std::cerr << "D-Bus Connection Error: " << error.message << std::endl;
    dbus_error_free(&error);
    return false;
  }

  // Store connection for future use
  if (connection) {
    // TODO: Use connection for Firefox automation
    dbus_connection_unref(connection);
  }

  // For now, just launch Firefox in headless mode if requested
  if (headless) {
    system("firefox --headless");
  } else {
    system("firefox");
  }

  return true;
}

void WebScraper::setUserAgent(const std::string &agent) { userAgent = agent; }

void WebScraper::setProxy(const std::string &proxy) { proxySettings = proxy; }

void WebScraper::addCustomHeader(const std::string &name,
                                 const std::string &value) {
  customHeaders.emplace_back(name, value);
}

void WebScraper::setPermissionCallback(PermissionCallback callback) {
  permissionCallback = callback;
}
