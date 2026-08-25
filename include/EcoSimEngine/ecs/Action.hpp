#pragma once

#include <string>
#include <string_view>

#include "EcoSimEngine/math/Vec2.hpp"

enum class ActionName {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SELECT,
    BACK,
    PAUSE,
    QUIT_AND_SAVE,
    LEFT_CLICK,
    MIDDLE_CLICK,
    RIGHT_CLICK,
    MOUSE_MOVE,
    TOGGLE_FOLLOW,
    TOGGLE_TEXTURE,
    TOGGLE_COLLISION,
    TOGGLE_GRID,
    NONE
};

enum class ActionType {
    START,
    END,
    NONE
};

class Action {
    ActionName m_name{ ActionName::NONE };
    ActionType m_type{ ActionType::NONE };
    Vec2f m_pos{};

    [[nodiscard]] static constexpr std::string_view nameString(ActionName name) noexcept {
        switch (name) {
        case ActionName::UP: return "UP";
        case ActionName::DOWN: return "DOWN";
        case ActionName::LEFT: return "LEFT";
        case ActionName::RIGHT: return "RIGHT";
        case ActionName::SELECT: return "SELECT";
        case ActionName::BACK: return "BACK";
        case ActionName::PAUSE: return "PAUSE";
        case ActionName::QUIT_AND_SAVE: return "QUIT_AND_SAVE";
        case ActionName::LEFT_CLICK: return "LEFT_CLICK";
        case ActionName::MIDDLE_CLICK: return "MIDDLE_CLICK";
        case ActionName::RIGHT_CLICK: return "RIGHT_CLICK";
        case ActionName::MOUSE_MOVE: return "MOUSE_MOVE";
        case ActionName::TOGGLE_FOLLOW: return "TOGGLE_FOLLOW";
        case ActionName::TOGGLE_TEXTURE: return "TOGGLE_TEXTURE";
        case ActionName::TOGGLE_COLLISION: return "TOGGLE_COLLISION";
        case ActionName::TOGGLE_GRID: return "TOGGLE_GRID";
        case ActionName::NONE: return "NONE";
        }
        return "UNKNOWN";
    }

    [[nodiscard]] static constexpr std::string_view typeString(ActionType type) noexcept {
        switch (type) {
        case ActionType::START: return "START";
        case ActionType::END: return "END";
        case ActionType::NONE: return "NONE";
        }
        return "UNKNOWN";
    }

public:
    Action() = default;

    Action(ActionName name, ActionType type, Vec2f pos)
        : m_name{ name }, m_type{ type }, m_pos{ pos } {
    }

    Action(ActionName name, ActionType type)
        : Action{ name, type, Vec2f{} } {
    }

    Action(ActionName name, Vec2f pos)
        : Action{ name, ActionType::NONE, pos } {
    }

    [[nodiscard]] ActionName name() const noexcept { return m_name; }
    [[nodiscard]] ActionType type() const noexcept { return m_type; }
    [[nodiscard]] const Vec2f& pos() const noexcept { return m_pos; }

    [[nodiscard]] std::string toString() const {
        return std::string{nameString(m_name)} + " " +
               std::string{typeString(m_type)} + " " +
               std::to_string(static_cast<int>(m_pos.x)) + " " +
               std::to_string(static_cast<int>(m_pos.y));
    }
};
