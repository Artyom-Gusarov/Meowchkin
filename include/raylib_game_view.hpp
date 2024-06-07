#ifndef RAYLIB_GAME_VIEW_HPP_
#define RAYLIB_GAME_VIEW_HPP_

#include <chrono>
#include <filesystem>
#include <limits>
#include <memory>
#include <variant>
#include "Functions.hpp"
#include "enum_array.hpp"
#include "game_view.hpp"
#include "gui_board.hpp"
#include "gui_card_span.hpp"
#include "gui_player_statistics_menu.hpp"
#include "gui_text_chat.hpp"
#include "gui_usernames_box.hpp"
#include "timed_state_machine.hpp"

namespace meow {

class RaylibGameView : public GameView {
private:
    inline static const raylib::Color background_color = raylib::Color(77, 120, 204, 255);

    struct GameplayObjs {
        GuiBoard board;
        GuiCardSpan player_hand;
        GuiUsernamesBox usernames_box;
        GuiTextChat text_chat;
        GuiPlayerStatisticsMenu stats;
    } m_gameplay_objects;

    struct PauseMenuObjs {
        static constexpr int button_width = 300;
        static constexpr int button_height = 40;

        enum class Button { CONTINUE, BACK_TO_LOBBY, QUIT, COUNT };
        EnumArray<Button, const char *> button_labels = {
            {Button::CONTINUE, "Continue"},
            {Button::BACK_TO_LOBBY, "Back to lobby"},
            {Button::QUIT, "Quit"},
        };
        EnumArray<Button, raylib::Rectangle> button_rects;
        EnumArray<Button, bool> button_pressed;
    } m_pause_menu_objects;

    std::variant<GameplayObjs *, PauseMenuObjs *> m_active_display;

    struct ActiveDisplaySelector {
        static constexpr int button_width = 80;
        static constexpr int button_height = 80;

        enum class Button { BRAWL, STATS, CHAT, PAUSE, COUNT };
        EnumArray<Button, const char *> button_texture_paths = {
            {Button::BRAWL, "bin/imgs/brawl_button_texture.png"},
            {Button::STATS, "bin/imgs/stats_button_texture.png"},
            {Button::CHAT, "bin/imgs/chat_button_texture.png"},
            {Button::PAUSE, "bin/imgs/pause_button_texture.png"},
        };
        EnumArray<Button, raylib::Texture> button_texs;
        EnumArray<Button, raylib::Rectangle> button_rects;
        EnumArray<Button, bool> button_pressed;
    } m_active_display_selector;

    /* background */
    raylib::Texture m_background;
    raylib::Texture m_blur;

    /* misc */
    std::vector<std::filesystem::path> m_card_image_paths;
    bool m_show_chat = false;
    raylib::Texture m_on_levelup_texture;
    raylib::Texture m_on_levelup_blur_texture;
    raylib::Color m_levelup_tint_color = raylib::Color::White();
    state_machine_function_args_t<raylib::Color &> m_levelup_blink_call =
        make_timed_state_machine([this](auto st, auto end, raylib::Color &tint) {
            m_on_levelup_blur_texture.Draw(0, 0, tint);
            tint.a = 255.0f -
                     255.0f * (std::chrono::steady_clock::now() - st).count() / (end - st).count();
            m_on_levelup_texture.Draw(
                (m_window->GetWidth() - m_on_levelup_texture.width) / 2,
                (m_window->GetHeight() - m_on_levelup_texture.height) / 2
            );
            // m_blur.Draw(0, 0, raylib::Color::Green());
        });
    bool m_levelup_blink = false;
    // state_machine_function_args_t<unsigned> m_dice_roll_blink_call =
    // make_timed_state_machine([this](auto, auto, unsigned res) {
    //     raylib::DrawText(std::to_string(res), 30, 400, 100, raylib::Color::Black());
    // });
    // bool m_dice_roll_blink = false;
    std::unordered_map<std::size_t, bool> m_active_turn;
    std::size_t m_player_id = -1;

protected:
    void on_instances_attach() override;

public:
    RaylibGameView();
    void draw() override;

    /* Callbacks */
    void on_card_add_on_board(
        std::size_t card_id,
        bool protogonist_sided,
        std::size_t user_id = std::numeric_limits<std::size_t>::max()
    ) override;
    void on_card_remove_from_board(std::size_t card_id) override;
    void on_turn_begin(std::size_t user_id) override;
    void on_turn_end(std::size_t user_id) override;
    void on_bonus_change(std::size_t user_id, int delta) override;
    void on_monster_bonus_change(std::size_t user_id, int delta) override;
    void on_level_change(std::size_t user_id, int delta) override;
    void on_card_receive(std::size_t user_id, std::size_t card_id) override;
    void on_card_loss(std::size_t user_id, std::size_t card_id) override;
    void on_monster_elimination(std::size_t user_id) override;  // which player killed monster
    void on_dice_roll(std::size_t user_id, unsigned res) override;

    static std::shared_ptr<Scene> make_raylib_gameview() {
        return std::make_shared<RaylibGameView>();
    }

private:
    void setup_background();
    void setup_pause_menu();
    void setup_active_display_selector();
    void setup_hand();
    void draw_pause_menu();
    void draw_active_display_selector();
};

}  // namespace meow

#endif  // RAYLIB_GAME_VIEW_HPP_
