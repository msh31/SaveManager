#pragma once
#include <array>

namespace utils {
    inline constexpr std::array<std::string_view, 12> IMAGE_EXTENSIONS = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".psd", ".gif", ".hdr", ".pic", ".pnm", ".ppm", ".pgm" };

    inline bool is_image_file( const fs::path& path ) {
        auto ext = path.extension( ).string( );
        std::transform( ext.begin( ), ext.end( ), ext.begin( ), []( unsigned char c ) { return std::tolower( c ); } );

        return std::find( IMAGE_EXTENSIONS.begin( ), IMAGE_EXTENSIONS.end( ), ext ) != IMAGE_EXTENSIONS.end( );
    }
} // namespace utils
