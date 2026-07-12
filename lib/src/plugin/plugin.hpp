#pragma once
#include <sol/sol.hpp>
#include <types.hpp>

#include "../detection/idetector.hpp"

class CPlugin : public IDetector {
    public:
        CPlugin( std::filesystem::path path );

        std::string_view name( ) const override { return m_name; }
        std::expected<std::vector<Game>, SMError> find( ) override;

    private:
        // TODO: use a shared sol::state with per-plugin sandboxed environments instead of one VM per plugin
        sol::state m_lua;
        bool m_show_parent_path = false;
        std::string m_name; // plugin filename, surfaced through name() for logging
};
