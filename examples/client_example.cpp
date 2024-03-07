#include <iostream>

#include "client.hpp"

int main(int argc, char *argv[]) {
  meow::Client me;

  std::string idplusport = argv[1];

  me.set_my_name(argv[2]); //сначала задаём имя, потом подключаемся
  me.connect(idplusport);

  std::cout << "Mоё айди это " << me.get_my_id() << "\n";
  std::cout << "Mоё имя это " << me.get_my_name() << "\n";

  std::cout << "Играю с ними:\n";
  for (meow::PlayerInfo& player : me.get_players_info()) {
    std::cout << player.to_json().dump(4) << '\n';
  }

  me.send_action(meow::Action(52, 812, me.get_my_id()));
  me.send_action(meow::Action(99, 912, me.get_my_id()));

  std::cout << "пытаюсь получать действия сервера\n";
  for (int i = 0; i < 5; ++i) {
    auto gettedact = me.receive_action();
    if (gettedact == std::nullopt) {
      std::cout << "упс\n";
    } else {
      std::cout << gettedact.value().to_json().dump(5) << '\n';
    }
    sleep(10);
  }
  std::cout << "устал\n";

  std::cout << "пытаюсь получать фидбеки сервера\n";
  for (int i = 0; i < 5; ++i) {
    auto getted_feed = me.receive_feedback();
    if (getted_feed == std::nullopt) {
      std::cout << "упс\n";
    } else {
      std::cout << getted_feed.value().to_json().dump(4) << '\n';
    }
    sleep(10);
  }
  std::cout << "устал\n";
}