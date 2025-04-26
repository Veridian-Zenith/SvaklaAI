// This file handles the web server part of SvaklaAI
// It creates a server that can accept web connections and respond to them

#include "web_interface.h"
#include "web_template.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace ssl = boost::asio::ssl;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

// HTTP session class for handling connections
class http_session : public std::enable_shared_from_this<http_session> {
public:
  http_session(tcp::socket &&socket, ssl::context &ssl_ctx,
               std::function<void(http::request<http::string_body> &,
                                  http::response<http::string_body> &)>
                   handler)
      : stream_(std::move(socket), ssl_ctx), handler_(std::move(handler)) {}

  void run() {
    // Perform SSL handshake
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
    stream_.async_handshake(
        ssl::stream_base::server,
        beast::bind_front_handler(&http_session::on_handshake,
                                  shared_from_this()));
  }

private:
  void on_handshake(beast::error_code ec) {
    if (ec)
      return;
    do_read();
  }

  void do_read() {
    req_ = {};
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    http::async_read(
        stream_, buffer_, req_,
        beast::bind_front_handler(&http_session::on_read, shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t) {
    if (ec == http::error::end_of_stream) {
      do_close();
      return;
    }
    if (ec)
      return;

    // Handle request
    handler_(req_, res_);

    // Set response fields
    res_.version(req_.version());
    res_.set(http::field::server, "SvaklaAI");
    res_.set(http::field::content_type, "text/html");
    res_.keep_alive(req_.keep_alive());

    // Send response
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
    http::async_write(
        stream_, res_,
        beast::bind_front_handler(&http_session::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t) {
    if (ec)
      return;
    do_close();
  }

  void do_close() {
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
    stream_.async_shutdown(beast::bind_front_handler(&http_session::on_shutdown,
                                                     shared_from_this()));
  }

  void on_shutdown(beast::error_code ec) {
    if (ec == net::error::eof) {
      ec = {};
    }
  }

  beast::ssl_stream<beast::tcp_stream> stream_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  http::response<http::string_body> res_;
  std::function<void(http::request<http::string_body> &,
                     http::response<http::string_body> &)>
      handler_;
};

// WebSocket session class for handling WebSocket connections
class ws_session : public std::enable_shared_from_this<ws_session> {
public:
  ws_session(tcp::socket &&socket, ssl::context &ssl_ctx,
             std::shared_ptr<AIEngine> ai_engine,
             std::shared_ptr<ChatStorage> chat_storage)
      : ws_(std::make_unique<
            websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(
            std::move(socket), ssl_ctx)),
        buffer_(), ai_engine_(ai_engine), chat_storage_(chat_storage),
        session_id_(generate_session_id()) {}

  void run() {
    beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(30));
    ws_->next_layer().async_handshake(
        ssl::stream_base::server,
        beast::bind_front_handler(&ws_session::on_ssl_handshake,
                                  shared_from_this()));
  }

private:
  void on_ssl_handshake(beast::error_code ec) {
    if (ec)
      return;

    // Accept the websocket handshake
    ws_->async_accept(
        beast::bind_front_handler(&ws_session::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec) {
    if (ec)
      return;
    do_read();
  }

  void do_read() {
    ws_->async_read(buffer_, beast::bind_front_handler(&ws_session::on_read,
                                                       shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec)
      return;

    std::string message = beast::buffers_to_string(buffer_.data());
    buffer_.consume(bytes_transferred);

    json response;
    try {
      // Store user message
      chat_storage_->store_message(session_id_, "user", message);

      // Process with AI engine
      auto ai_response = ai_engine_->process_message(message);

      // Store AI response
      chat_storage_->store_message(session_id_, "assistant", ai_response);

      // Send response
      response = {{"type", "message"},
                  {"role", "assistant"},
                  {"content", ai_response},
                  {"session_id", session_id_}};
    } catch (const std::exception &e) {
      response = {{"type", "error"},
                  {"content", std::string("Error: ") + e.what()},
                  {"session_id", session_id_}};
    }

    ws_->async_write(
        net::buffer(response.dump()),
        beast::bind_front_handler(&ws_session::on_write, shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t) {
    if (ec)
      return;
    do_read(); // Read next message
  }

  std::string generate_session_id() {
    static std::atomic<uint64_t> counter(0);
    return std::to_string(std::time(nullptr)) + "-" + std::to_string(counter++);
  }

  std::unique_ptr<websocket::stream<beast::ssl_stream<beast::tcp_stream>>> ws_;
  beast::flat_buffer buffer_;
  std::shared_ptr<AIEngine> ai_engine_;
  std::shared_ptr<ChatStorage> chat_storage_;
  std::string session_id_;
};

WebInterface::WebInterface(boost::asio::io_context &io_context)
    : m_io_context(io_context), m_ssl_context(ssl::context::tlsv12),
      m_running(false), m_http_port(0), m_ws_port(0), m_api_port(0),
      m_ai_engine(std::make_shared<AIEngine>()),
      m_chat_storage(std::make_shared<ChatStorage>()) {}

WebInterface::~WebInterface() { stop(); }

bool WebInterface::initialize(uint16_t http_port, uint16_t ws_port,
                              uint16_t api_port) {
  m_http_port = http_port;
  m_ws_port = ws_port;
  m_api_port = api_port;

  try {
    setup_tls();
    setup_http_server(http_port);
    setup_websocket_server(ws_port);
    setup_api_server(api_port);

    m_running = true;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize web interface: " << e.what()
              << std::endl;
    return false;
  }
}

void WebInterface::stop() {
  m_running = false;

  if (m_http_acceptor)
    m_http_acceptor->close();
  if (m_ws_acceptor)
    m_ws_acceptor->close();
  if (m_api_acceptor)
    m_api_acceptor->close();
}

void WebInterface::setup_tls() {
  m_ssl_context.set_options(ssl::context::default_workarounds |
                            ssl::context::no_sslv2 | ssl::context::no_sslv3 |
                            ssl::context::single_dh_use);

  // Define certificate paths
  const std::string cert_dir = "/etc/letsencrypt/live/vz.strangled.net/";
  const std::string chain_file = cert_dir + "fullchain.pem";
  const std::string key_file = cert_dir + "privkey.pem";

  // Check if certificate files exist
  if (!std::filesystem::exists(chain_file)) {
    throw std::runtime_error("SSL certificate chain file not found: " +
                             chain_file);
  }
  if (!std::filesystem::exists(key_file)) {
    throw std::runtime_error("SSL private key file not found: " + key_file);
  }

  // Check file permissions
  if (access(chain_file.c_str(), R_OK) != 0) {
    throw std::runtime_error(
        "Cannot read SSL certificate chain file (check permissions): " +
        chain_file);
  }
  if (access(key_file.c_str(), R_OK) != 0) {
    throw std::runtime_error(
        "Cannot read SSL private key file (check permissions): " + key_file);
  }

  try {
    m_ssl_context.use_certificate_chain_file(chain_file);
    m_ssl_context.use_private_key_file(key_file, ssl::context::pem);
  } catch (const boost::system::system_error &e) {
    throw std::runtime_error("Failed to load SSL certificates: " +
                             std::string(e.what()));
  }
}

void WebInterface::setup_http_server(uint16_t port) {
  auto const address = boost::asio::ip::make_address("0.0.0.0");
  m_http_acceptor = std::make_unique<tcp::acceptor>(
      m_io_context, tcp::endpoint{address, port});

  do_http_accept();
}

void WebInterface::setup_websocket_server(uint16_t port) {
  auto const address = boost::asio::ip::make_address("0.0.0.0");
  m_ws_acceptor = std::make_unique<tcp::acceptor>(m_io_context,
                                                  tcp::endpoint{address, port});

  do_ws_accept();
}

void WebInterface::setup_api_server(uint16_t port) {
  auto const address = boost::asio::ip::make_address("0.0.0.0");
  m_api_acceptor = std::make_unique<tcp::acceptor>(
      m_io_context, tcp::endpoint{address, port});

  do_api_accept();
}

void WebInterface::do_http_accept() {
  m_http_acceptor->async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
          // Handle HTTP connection
          std::make_shared<http_session>(std::move(socket), m_ssl_context,
                                         [this](auto &req, auto &res) {
                                           handle_http_request(req.body(),
                                                               res.body());
                                         })
              ->run();
        }

        if (m_running) {
          do_http_accept();
        }
      });
}

void WebInterface::handle_http_request(const std::string &request
                                       [[maybe_unused]],
                                       std::string &response) {
  response =
      "<!DOCTYPE html>\n"
      "<html>\n"
      "<head>\n"
      "    <title>SvaklaAI Web Interface</title>\n"
      "    <meta charset=\"UTF-8\">\n"
      "    <meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1\">\n"
      "    <link "
      "href=\"https://fonts.googleapis.com/"
      "css2?family=Gamja+Flower&display=swap\" rel=\"stylesheet\">\n"
      "    <style>\n"
      "        body {\n"
      "            font-family: \"Gamja Flower\", sans-serif;\n"
      "            background-color: #111;\n"
      "            color: #FFD700;\n"
      "            margin: 0;\n"
      "            padding: 20px;\n"
      "        }\n"
      "        .container {\n"
      "            max-width: 1200px;\n"
      "            margin: 0 auto;\n"
      "        }\n"
      "        .header {\n"
      "            background: rgba(0, 0, 0, 0.8);\n"
      "            padding: 20px;\n"
      "            border-radius: 20px;\n"
      "            margin-bottom: 20px;\n"
      "        }\n"
      "        .content {\n"
      "            display: flex;\n"
      "            gap: 20px;\n"
      "        }\n"
      "        .panel {\n"
      "            flex: 1;\n"
      "            background: rgba(0, 0, 0, 0.8);\n"
      "            padding: 20px;\n"
      "            border-radius: 15px;\n"
      "            border: 1px solid #FFD700;\n"
      "        }\n"
      "        #chat-output {\n"
      "            height: 400px;\n"
      "            overflow-y: auto;\n"
      "            background: #111;\n"
      "            padding: 10px;\n"
      "            border-radius: 10px;\n"
      "            border: 1px solid #FFD700;\n"
      "            margin-bottom: 10px;\n"
      "        }\n"
      "        #chat-input {\n"
      "            width: 100%;\n"
      "            padding: 10px;\n"
      "            background: #111;\n"
      "            border: 1px solid #FFD700;\n"
      "            border-radius: 5px;\n"
      "            color: #FFD700;\n"
      "            font-family: inherit;\n"
      "            font-size: 1.1em;\n"
      "        }\n"
      "        #chat-input:focus {\n"
      "            outline: none;\n"
      "            border-color: white;\n"
      "        }\n"
      "        h1, h2 {\n"
      "            color: #FFD700;\n"
      "            margin: 0 0 20px 0;\n"
      "        }\n"
      "        .message {\n"
      "            margin: 10px 0;\n"
      "            padding: 10px;\n"
      "            border-radius: 5px;\n"
      "        }\n"
      "        .user-message {\n"
      "            background: rgba(255, 215, 0, 0.1);\n"
      "            border-left: 3px solid #FFD700;\n"
      "        }\n"
      "        .ai-message {\n"
      "            background: rgba(255, 255, 255, 0.1);\n"
      "            border-left: 3px solid white;\n"
      "        }\n"
      "    </style>\n"
      "</head>\n"
      "<body>\n"
      "    <div class=\"container\">\n"
      "        <div class=\"header\">\n"
      "            <h1>SvaklaAI Interface</h1>\n"
      "        </div>\n"
      "        <div class=\"content\">\n"
      "            <div class=\"panel\">\n"
      "                <h2>Chat Interface</h2>\n"
      "                <div id=\"chat-output\"></div>\n"
      "                <input type=\"text\" id=\"chat-input\" "
      "placeholder=\"Type your message...\">\n"
      "            </div>\n"
      "            <div class=\"panel\">\n"
      "                <h2>System Status</h2>\n"
      "                <div id=\"system-status\"></div>\n"
      "            </div>\n"
      "        </div>\n"
      "    </div>\n"
      "    <script>\n"
      "        const ws = new WebSocket('wss://' + window.location.hostname + "
      "':776');\n"
      "        const chatOutput = document.getElementById('chat-output');\n"
      "        const chatInput = document.getElementById('chat-input');\n"
      "        const systemStatus = document.getElementById('system-status');\n"
      "\n"
      "        function updateStatus(message, isError = false) {\n"
      "            const div = document.createElement('div');\n"
      "            div.className = `message ${isError ? 'ai-message' : "
      "'user-message'}`;\n"
      "            div.textContent = message;\n"
      "            systemStatus.appendChild(div);\n"
      "        }\n"
      "\n"
      "        function addMessage(role, content) {\n"
      "            const div = document.createElement('div');\n"
      "            div.className = `message ${role === 'user' ? 'user-message' "
      ": 'ai-message'}`;\n"
      "            div.innerHTML = `<strong>${role === 'user' ? 'You' : "
      "'AI'}:</strong> ${content}`;\n"
      "            chatOutput.appendChild(div);\n"
      "            chatOutput.scrollTop = chatOutput.scrollHeight;\n"
      "        }\n"
      "\n"
      "        ws.onopen = () => {\n"
      "            updateStatus('WebSocket Connected');\n"
      "            chatInput.disabled = false;\n"
      "        };\n"
      "\n"
      "        ws.onclose = () => {\n"
      "            updateStatus('WebSocket Disconnected', true);\n"
      "            chatInput.disabled = true;\n"
      "        };\n"
      "\n"
      "        ws.onerror = (error) => {\n"
      "            updateStatus('WebSocket Error: ' + error.message, true);\n"
      "            chatInput.disabled = true;\n"
      "        };\n"
      "\n"
      "        ws.onmessage = (event) => {\n"
      "            try {\n"
      "                const msg = JSON.parse(event.data);\n"
      "                if (msg.type === 'error') {\n"
      "                    updateStatus(msg.content, true);\n"
      "                } else {\n"
      "                    addMessage(msg.role, msg.content);\n"
      "                }\n"
      "            } catch (e) {\n"
      "                updateStatus('Error parsing message: ' + e.message, "
      "true);\n"
      "            }\n"
      "        };\n"
      "\n"
      "        chatInput.onkeypress = (e) => {\n"
      "            if (e.key === 'Enter' && chatInput.value) {\n"
      "                const message = chatInput.value;\n"
      "                addMessage('user', message);\n"
      "                ws.send(message);\n"
      "                chatInput.value = '';\n"
      "            }\n"
      "        };\n"
      "    </script>\n"
      "</body>\n"
      "</html>\n";
}

void WebInterface::do_api_accept() {
  if (!m_api_acceptor)
    return;

  m_api_acceptor->async_accept([this](boost::system::error_code ec,
                                      tcp::socket socket [[maybe_unused]]) {
    if (!ec) {
      // Handle API connection
      // TODO: Implement API session handling
    }

    if (m_running) {
      do_api_accept();
    }
  });
}

void WebInterface::do_ws_accept() {
  if (!m_ws_acceptor)
    return;

  m_ws_acceptor->async_accept([this](beast::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<ws_session>(std::move(socket), m_ssl_context,
                                   m_ai_engine, m_chat_storage)
          ->run();
    }

    if (m_running) {
      do_ws_accept();
    }
  });
}

// ... rest of implementation ...
