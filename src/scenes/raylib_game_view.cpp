#include "raylib_game_view.hpp"
#include <boost/dll/alias.hpp>
#include <chrono>
#include <iostream>
#include <string_view>
#include <variant>
#include "gui_card_loader.hpp"
#include "gui_dice_roller.hpp"
#include "paths_to_binaries.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace {

constexpr float opts_button_width = 210;
constexpr float opts_button_height = 70;

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template <typename EnumT>
    requires meow::CountableEnum<EnumT>
EnumT draw_opts_menu(const meow::EnumArray<EnumT, std::string> &opts, raylib::Vector2 pos) {
    EnumT ret = EnumT::COUNT;
    for (std::size_t i = 0; i < opts.size(); ++i) {
        if (GuiButton(
                Rectangle{
                    pos.x, pos.y + i * opts_button_height, opts_button_width, opts_button_height
                },
                opts[i].c_str()
            )) {
            ret = static_cast<EnumT>(i);
        }
    }
    return ret;
}

}  // namespace

meow::RaylibGameView::RaylibGameView()
    : m_active_display(&m_gameplay_objects), m_on_levelup_texture("bin/imgs/levelup.png") {
    GuiLoadStyle(gui_style_path);
    GuiSetFont(LoadFont(gui_font_path));
    for (const auto &entry : std::filesystem::directory_iterator(cards_directory_path)) {
        if (entry.path().extension() == ".png") {
            load_card_img(entry.path().c_str());
            m_card_image_paths.emplace_back(entry);
        }
    }
}

void meow::RaylibGameView::on_instances_attach() {
    setup_background();
    setup_pause_menu();
    setup_hand();
    setup_active_display_selector();
    m_gameplay_objects.board.setup(m_window, &m_gameplay_objects.player_hand, m_client);
    m_gameplay_objects.text_chat.set_window(m_window);
    m_gameplay_objects.usernames_box.set_window(m_window);

    for (const auto &info : m_client->get_players_info()) {
        m_gameplay_objects.usernames_box.add_username(info.name);
    }
}

void meow::RaylibGameView::setup_background() {
    raylib::Image background_image;
    try {
        background_image.Load(gameview_background_image_path);
        background_image.Resize(m_window->GetWidth(), m_window->GetHeight());
    } catch (const raylib::RaylibException &) {
        background_image =
            raylib::Image::Color(m_window->GetWidth(), m_window->GetHeight(), background_color);
    }
    m_background.Load(background_image);

    raylib::Image blur = raylib::Image::GradientRadial(
        m_window->GetWidth(), m_window->GetHeight(), 0, raylib::Color::Blank(), {0, 0, 0, 100}
    );
    m_blur.Load(blur);
}

void meow::RaylibGameView::setup_pause_menu() {
    for (std::size_t i = 0; i < m_pause_menu_objects.button_rects.size(); ++i) {
        m_pause_menu_objects.button_rects[i].width = PauseMenuObjs::button_width;
        m_pause_menu_objects.button_rects[i].height = PauseMenuObjs::button_height;
        m_pause_menu_objects.button_rects[i].x =
            m_window->GetWidth() / 2.0f - PauseMenuObjs::button_width / 2.0f;
        m_pause_menu_objects.button_rects[i].y =
            m_window->GetHeight() / 2.0f + i * PauseMenuObjs::button_height;
    }
}

void meow::RaylibGameView::setup_active_display_selector() {
    auto &br = m_active_display_selector.button_rects;
    auto &texs = m_active_display_selector.button_texs;

    float height = 0;
    const int w = ActiveDisplaySelector::button_width, h = ActiveDisplaySelector::button_height;
    for (std::size_t i = 0; i < br.size(); ++i) {
        br[i].width = w;
        br[i].height = h;
        br[i].x = (float)(m_window->GetWidth() - GuiBoard::width) / 2 - h;
        height = br[i].y = i * h;

        try {
            texs[i] = raylib::Image(m_active_display_selector.button_texture_paths[i]).Resize(w, h);
        } catch (const raylib::RaylibException &) {
            texs[i] = raylib::Image().Color(w, h, raylib::Color(random_integer<unsigned>()));
        }
    }

    // align with board center
    height += h;
    for (std::size_t i = 0; i < br.size(); ++i) {
        br[i].y = br[i].y - height / 2 + GuiBoard::offset_top + GuiBoard::height / 2.0f;
    }
}

void meow::RaylibGameView::setup_hand() {
    m_gameplay_objects.player_hand.set_window(m_window);
    m_gameplay_objects.player_hand.set_span_borders({m_window->GetPosition(), m_window->GetSize()});
    for (int i = 0; i < 5; i++) {
        const auto random_index = random_integer<std::size_t>(0, m_card_image_paths.size() - 1);
        m_gameplay_objects.player_hand.add_card(m_card_image_paths[random_index].c_str());
    }
}

void meow::RaylibGameView::draw_pause_menu() {
    using PauseButton = PauseMenuObjs::Button;
    for (std::size_t i = 0; i < m_pause_menu_objects.button_labels.size(); ++i) {
        m_pause_menu_objects.button_pressed[i] =
            GuiButton(m_pause_menu_objects.button_rects[i], m_pause_menu_objects.button_labels[i]);
    }
    if (m_pause_menu_objects.button_pressed[PauseButton::CONTINUE]) {
        m_active_display = &m_gameplay_objects;
    }
    if (m_pause_menu_objects.button_pressed[PauseButton::BACK_TO_LOBBY]) {
        m_scene_manager->switch_scene(SceneType::MAIN_MENU);
    }
    if (m_pause_menu_objects.button_pressed[PauseButton::QUIT]) {
        m_running = false;
    }
}

void meow::RaylibGameView::draw_active_display_selector() {
    auto &br = m_active_display_selector.button_rects;
    auto &texs = m_active_display_selector.button_texs;
    auto &pressed = m_active_display_selector.button_pressed;

    float was = guiAlpha;
    GuiSetAlpha(0.3f);
    for (std::size_t i = 0; i < br.size(); ++i) {
        br[i].DrawRoundedLines(0.1, 4, 1, raylib::Color::RayWhite());
        br[i].DrawRounded(0.1, 4, raylib::Color(0xFFFFFF88));
        texs[i].Draw(br[i].GetPosition());
        pressed[i] = GuiButton(br[i], "");
    }
    GuiSetAlpha(was);

    using Button = ActiveDisplaySelector::Button;
    if (pressed[Button::BRAWL]) {
        m_active_display = &m_gameplay_objects;
        m_show_chat = false;
    } else if (pressed[Button::PAUSE]) {
        m_active_display = &m_pause_menu_objects;
    } else if (pressed[Button::CHAT]) {
        m_show_chat = !m_show_chat;
    }
}

void meow::RaylibGameView::draw() {
    Overload active_display_visitor = {
        [this](GameplayObjs *objs) {
            objs->board.draw(m_window->GetFrameTime());
            objs->player_hand.draw_cards(m_window->GetFrameTime());
            objs->usernames_box.draw();
            if (m_show_chat) {
                objs->text_chat.draw(*m_client);
            }
            objs->stats.draw();  // TODO: move to separate object
            draw_active_display_selector();
            GuiCardSpan::draw_inspected_card(m_window->GetWidth(), m_window->GetHeight());

            if (GuiButton({0, 0, 40, 40}, "-") && objs->player_hand.card_count() > 0) {
                objs->player_hand.remove_card();
            }
            if (GuiButton({40, 0, 40, 40}, "+") && objs->player_hand.card_count() < 10) {
                const auto random_index =
                    random_integer<std::size_t>(0, m_card_image_paths.size() - 1);
                objs->player_hand.add_card(m_card_image_paths[random_index].c_str());
            }
            if (objs->player_hand.somethind_inspected()) {
                m_blur.Draw();
            }

            enum class bebra_enum { bebra1, bebra2, COUNT };
            EnumArray<bebra_enum, std::string> bebra_arr{
                {bebra_enum::bebra1, "bbbbeeebra1"}, {bebra_enum::bebra2, "bbbbeeebra2"}
            };
            float was = guiAlpha;
            GuiSetAlpha(0.85);
            if (auto x = draw_opts_menu(
                    bebra_arr, {m_window->GetWidth() - opts_button_width,
                                m_window->GetHeight() - opts_button_height * bebra_arr.size()}
                );
                x != bebra_enum::COUNT) {
                std::cout << bebra_arr[x] << '\n';
            }
            GuiSetAlpha(was);
        },
        [this](PauseMenuObjs *objs) {
            m_blur.Draw();
            draw_pause_menu();
        },
        [](auto...) { throw std::runtime_error("unhandled type(s) in active display!"); }
    };

    // FIXME
    if (auto action = m_client->receive_action(); action) {
        if (random_integer(0, 1) == 0) {
            m_gameplay_objects.board.m_kitten_cards.add_card(action->card_filename);
        } else {
            m_gameplay_objects.board.m_opponent_cards.add_card(action->card_filename);
        }
    }

    if (auto chat_message = m_client->receive_chat_message(); chat_message) {
        std::cout << "received from " << chat_message->sender_player << ": "
                  << chat_message->message << '\n';
        m_gameplay_objects.text_chat.receive(*chat_message);
    }

    m_background.Draw();
    std::visit(active_display_visitor, m_active_display);

    if (IsKeyPressed(KEY_SPACE)) {
        on_levelup();
    }
    if (IsKeyPressed(KEY_ENTER)) {
        on_turn_begin();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (!std::holds_alternative<PauseMenuObjs *>(m_active_display)) {
            m_active_display = &m_pause_menu_objects;
        } else {
            m_active_display = &m_gameplay_objects;
        }
    }
    m_levelup_blink_call(std::chrono::milliseconds(1000), m_levelup_blink);
    m_levelup_blink = false;
}

/* Callbacks */
void meow::RaylibGameView::on_card_add(std::string_view card_filename) {
    m_gameplay_objects.board.add_card(card_filename);
}

void meow::RaylibGameView::on_card_remove(std::string_view card_filename) {
    m_gameplay_objects.board.remove_card(card_filename);
}

void meow::RaylibGameView::on_turn_begin() {
}

void meow::RaylibGameView::on_turn_end() {
}

void meow::RaylibGameView::on_levelup() {
    // ++m_gameplay_objects.stats.menu_elements[GuiPlayerStatisticsMenu::StatisticKind::LEVEL].value;
    // m_gameplay_objects.stats.blink = true;
    // m_gameplay_objects.stats.blink_color = raylib::Color(0, 200, 0, 180);
    m_levelup_blink = true;
}

void meow::RaylibGameView::on_card_receive() {
}

void meow::RaylibGameView::on_item_equip() {
}

void meow::RaylibGameView::on_item_loss() {
}

void meow::RaylibGameView::on_monster_elimination() {
}

void meow::RaylibGameView::on_being_cursed() {
}

BOOST_DLL_ALIAS(meow::RaylibGameView::make_raylib_gameview, make_gameview)