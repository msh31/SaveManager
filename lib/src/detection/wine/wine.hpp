#pragma once
#include "../detector_context.hpp"
#include "../idetector.hpp"

class CWinePrefixDetector : public IDetector {
    public:
        CWinePrefixDetector(
            fs::path prefix, DetectorContext ctx, std::vector<WineScanHook> prefix_hooks,
            std::vector<WineScanHook> user_hooks )
            : m_path( std::move( prefix ) ), m_ctx( ctx ), m_prefix_hooks( std::move( prefix_hooks ) ),
              m_user_hooks( std::move( user_hooks ) ) {};

        std::expected<std::vector<Game>, SMError> find( ) override;

        std::string_view name( ) const override;

    private:
        fs::path m_path = { }; // must be set
        DetectorContext m_ctx;
        std::vector<WineScanHook> m_prefix_hooks; // per drive_c, e.g. Program Files
        std::vector<WineScanHook> m_user_hooks;   // per user under drive_c/users
};
