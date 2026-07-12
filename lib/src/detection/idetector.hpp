#pragma once
#include <types.hpp>

class IDetector {
    public:
        virtual ~IDetector( ) = default;

        virtual std::string_view name( ) const = 0;

        [[nodiscard]] virtual std::expected<std::vector<Game>, SMError> find( ) = 0;
};
