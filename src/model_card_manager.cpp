#include "model_card_manager.hpp"
#include "paths_to_binaries.hpp"
#include "model_card.hpp"

#include <nlohmann/json.hpp>
#include <cassert>
#include <fstream>
#include <memory>
#include <vector>

namespace meow {
CardManager::CardManager() {
    std::map<std::string, model::CardType> str_to_type{
        {"SPELL", model::CardType::SPELL},
        {"MONSTER", model::CardType::MONSTER},
        {"RACE", model::CardType::RACE},
        {"CLASS", model::CardType::CLASS},
        {"ITEM", model::CardType::ITEM}};

    std::ifstream f(json_info_path);
    ::nlohmann::json data = ::nlohmann::json::parse(f);
    f.close();

    std::size_t counter = 0;
    for (auto &card : data["cards"]) {
        std::string image = card["image"].get<std::string>();
        std::size_t card_id = counter++;
        model::CardType type = str_to_type[card["type"].get<std::string>()];
        bool openable = card["openable"].get<bool>();
        std::vector<model::Command> verification =
            model::Command::parse(card["verification"].get<std::vector<std::string>>());

        model::CardInfo info(image, card_id, type, openable, verification);

        switch (type) {
            case model::CardType::SPELL: {
                std::vector<model::Command> action =
                    model::Command::parse(card["action"].get<std::vector<std::string>>());
                std::vector<model::Command> unwind =
                    model::Command::parse(card["unwind"].get<std::vector<std::string>>());
                bool storable = card["storable"].get<bool>();
                bool is_one_time = card["is_one_time"].get<bool>();

                cards_instances.emplace_back(std::make_unique<model::SpellCardInfo>(
                    std::move(info), storable, is_one_time, action, unwind
                ));
            } break;
            default:
                break;
        }
    }
}

std::unique_ptr<model::Card> CardManager::get_card(std::size_t card_id) const {
    switch (cards_instances.at(card_id)->type) {
        case model::CardType::SPELL: {
            return std::make_unique<model::SpellCard>(cards_instances.at(card_id).get());
        } break;
        default:
            break;
    }
    assert(false);
    return nullptr;
}
}  // namespace meow