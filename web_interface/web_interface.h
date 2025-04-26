#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class WebInterface {
public:
  explicit WebInterface(boost::asio::io_context &io_context);
  ~WebInterface();

  bool initialize(uint16_t http_port, uint16_t ws_port, uint16_t api_port);
  void stop();

private:
  // HTTP Server handlers
  void setup_http_server(uint16_t port);
  void handle_http_request(const std::string &request, std::string &response);
  void do_http_accept();

  // WebSocket handlers
  void setup_websocket_server(uint16_t port);
  void handle_ws_message(const std::string &message);
  void do_ws_accept();

  // Direct API handlers
  void setup_api_server(uint16_t port);
  void handle_api_request(const std::string &request, std::string &response);
  void do_api_accept();

  // TLS/Security
  void setup_tls();
  bool verify_auth(const std::string &token);

  // Internal state
  boost::asio::io_context &m_io_context;
  ssl::context m_ssl_context;
  bool m_running;

  // Acceptors
  std::unique_ptr<tcp::acceptor> m_http_acceptor;
  std::unique_ptr<tcp::acceptor> m_ws_acceptor;
  std::unique_ptr<tcp::acceptor> m_api_acceptor;

  // Connection tracking
  std::unordered_map<std::string, std::string> m_active_sessions;

  // Ports
  uint16_t m_http_port;
  uint16_t m_ws_port;
  uint16_t m_api_port;
};
