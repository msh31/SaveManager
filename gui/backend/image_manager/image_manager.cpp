#include "image_manager.hpp"

#include <logger.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

ImageData CImageManager::load_from_disk( const fs::path& path, const std::string& name ) {
    ImageData img;
    int channels = 0; // discarded

    FILE* fp = nullptr;
#if defined( _WIN32)
    fp = _wfopen( path.c_str( ), L"rb" );
#else
    fp = fopen( path.c_str( ), "rb" );
#endif

    if ( !fp ) {
        SPDLOG_ERROR( "[CImageManager]: failed to open file for loading: {}", path.filename( ).string() );
        return { };
    }

    auto loaded_image = stbi_load_from_file( fp, &img.texture_width, &img.texture_height, &channels, 4 );

    if ( loaded_image == nullptr ) {
        SPDLOG_ERROR( "[ImageManager]: Failed to load image from disk: {}", path.string( ) );
        fclose( fp );
        return { };
    }

    fclose( fp );
    return upload( loaded_image, img.texture_width, img.texture_height, name );
}

ImageData CImageManager::load_from_memory( void* data, int length, const std::string& name ) {
    ImageData img;
    int channels = 0;

    auto loaded_image = stbi_load_from_memory(
        static_cast<const unsigned char*>( data ), length, &img.texture_width, &img.texture_height, &channels, 4 );

    if ( loaded_image == nullptr ) {
        SPDLOG_ERROR( "[ImageManager]: Failed to load image from memory: {}", name );
        return { };
    }

    return upload( loaded_image, img.texture_width, img.texture_height, name );
}

ImageData CImageManager::upload( const unsigned char* pixels, int width, int height, const std::string& name ) {
    ImageData img;
    img.texture_width = width;
    img.texture_height = height;

    if ( auto it = m_images.find( name ); it != m_images.end( ) ) {
        glDeleteTextures( 1, &it->second.texture_id );
        it->second.texture_id = 0;
    }

    glGenTextures( 1, &img.texture_id );
    glBindTexture( GL_TEXTURE_2D, img.texture_id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // upload
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, img.texture_width, img.texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );

    stbi_image_free( (void*)pixels );
    m_images[name] = img;
    return img;
}

ImageData CImageManager::get_image( const std::string& img_name ) {
    if ( auto it = m_images.find( img_name ); it != m_images.end( ) ) {
        return it->second;
    }
    return { };
}

CImageManager& CImageManager::get( ) {
    static CImageManager instance;
    return instance;
}
