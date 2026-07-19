#pragma once
#include <detection/game.hpp>
#include <sm_error.hpp>

class IDetector {
    public:
        virtual ~IDetector( ) = default;

        virtual std::string_view name( ) const = 0;

        [[nodiscard]] virtual std::expected<std::vector<Game>, SMError> find( ) = 0;
};
