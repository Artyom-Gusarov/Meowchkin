#ifndef GUI_BOARD_HPP
#define GUI_BOARD_HPP

#include <memory>
#include "client.hpp"
#include "gui_card_span.hpp"
#include "gui_card_span_dropdown_menu.hpp"
#include "raylib-cpp.hpp"

namespace meow {

class GuiBoard {
    friend class RaylibGameView;

private:
    GuiCardSpan *m_player_hand = nullptr;
    raylib::Window *m_window = nullptr;
    network::Client *m_client = nullptr;

    raylib::Rectangle m_rect;
    raylib::Texture m_texture;
    raylib::Rectangle m_drop_card_rect;
    GuiCardSpan m_kitten_cards;
    GuiCardSpan m_opponent_cards;

public:
    static constexpr int width = 1200;
    static constexpr int height = 700;
    static constexpr int offset_top = 60;

    GuiBoard()
        : m_kitten_cards(std::make_unique<BrawlCardsDDM>(&m_kitten_cards)),
          m_opponent_cards(std::make_unique<BrawlCardsDDM>(&m_opponent_cards)) {
    }

    void setup(raylib::Window *window, GuiCardSpan *hand, network::Client *client);
    void draw(float frame_time);
    void add_card(std::size_t card_id);

    void remove_card(std::string_view card_filename) {
        m_kitten_cards.remove_card(card_filename);
    }
};

}  // namespace meow

#endif  // GUI_BOARD_HPP
