#include <iostream>

#include "server.hpp"

int main(int argc, char *argv[]) {
  meow::Server serv;

  std::cout << "слушаю в порту " << serv.get_port() << '\n';

  serv.start_listening(std::stoi(argv[1]));

  std::cout << "поймал этих:\n";
  for (std::size_t id : serv.get_clients_id()) {
    std::cout << id << "\n";
  }

  meow::Action actforoneplayer(123, serv.get_clients_id()[0], 0);
  serv.send_action(serv.get_clients_id()[0], actforoneplayer);

  meow::Action actforall(666, 4, 0);
  serv.send_action_to_all_clients(actforall);

  meow::Feedback feed(190, false);
  serv.send_feedback(serv.get_clients_id()[0], feed);

  std::cout << "пытаюсь получать действия клиентов\n";
  for (int i = 0; i < 5; ++i) {
    auto gettedact = serv.receive_action();
    if (gettedact == std::nullopt) {
      std::cout << "упс\n";
    } else {
      std::cout << gettedact.value().to_json().dump(5) << '\n';
    }
    sleep(10);
  }
  std::cout << "устал\n";
}