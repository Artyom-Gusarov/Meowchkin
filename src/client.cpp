#include "client.hpp"

#include <boost/asio.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;

namespace meow {
Client::Client() : io_context() {}

Client::~Client() = default;

void Client::accept_info_about_players() {
  std::string buf;
  std::size_t count_of_players;
  std::getline(connection, buf);
  count_of_players = std::stoul(buf);
  players_info.reserve(count_of_players);
  std::getline(connection, buf);
  my_id = std::stoul(buf);

  for (std::size_t i = 0; i < count_of_players; ++i) {
    std::getline(connection, buf);
    auto json = json::parse(buf);
    players_info[i] = PlayerInfo(json);
  }
}

void Client::connect(const std::string& host) {
  size_t delimiter_pos = host.find(':');
  std::string ip = host.substr(0, delimiter_pos);
  std::string port = host.substr(delimiter_pos + 1);

  tcp::socket s(io_context);
  boost::asio::connect(s, tcp::resolver(io_context).resolve(ip, port));
  connection = tcp::iostream(std::move(s));

  std::thread([this]() mutable {
    while (connection) {
      std::string received_msg;
      if (!std::getline(connection, received_msg)) {
        break;
      }
      auto json = json::parse(received_msg);
      std::unique_lock l(mtx);
      if (json.contains("validness")) {
        received_feedbacks.emplace(json);
      } else {
        received_actions.emplace(json);
      }
    }
  }).detach();

  connection << my_name + "\n" << std::flush;

  accept_info_about_players();
}

void Client::disconnect() { connection.close(); }

[[nodiscard]] std::size_t Client::get_my_id() const { return my_id; }

[[nodiscard]] std::string Client::get_my_name() const { return my_name; }

void Client::set_my_name(const std::string& name) { my_name = name; }

[[nodiscard]] std::vector<PlayerInfo> Client::get_players_info() const {
  return players_info;
}

void Client::send_action(const Action& action) {
  auto json = action.to_json();
  std::string message_to_send;
  message_to_send = json.dump();
  message_to_send += "\n";
  connection << message_to_send << std::flush;
}

std::optional<Action> Client::receive_action() {
  std::unique_lock l(mtx);
  if (received_actions.empty()) {
    return std::nullopt;
  }
  Action action = received_actions.front();
  received_actions.pop();
  return action;
}

std::optional<Feedback> Client::get_feedback() {
  std::unique_lock l(mtx);
  if (received_feedbacks.empty()) {
    return std::nullopt;
  }
  Feedback feedback = received_feedbacks.front();
  received_feedbacks.pop();
  return feedback;
}
}  // namespace meow