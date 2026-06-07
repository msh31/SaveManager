#pragma once
#include <types.hpp>

class CConfig;
class CLudusaviParser;

enum class GameKeyKind { INVALID, STEAM_APPID, UBISOFT_ID, MINECRAFT, NAME, PATH };
struct GameKey {
        GameKeyKind kind;
        std::string value;
        // spaceship operator,
        auto operator<=>( const GameKey& ) const = default;
};

namespace Detection {
    GameKey get_game_identity_key( const Game& game );

    void add_game(
        std::expected<std::vector<Game>, DetectionError> result, const std::string& platform,
        std::vector<Game>& games );
    void find_saves( CConfig& config, std::vector<Game>& games );

    // should not be called externally
    void find_saves_ludusavi( CConfig& config, std::vector<Game>& games, std::shared_ptr<CLudusaviParser>& parser );
}; // namespace Detection
