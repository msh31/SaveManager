#pragma once

struct ImageData {
        GLuint texture_id = 0;
        int texture_width = 0;
        int texture_height = 0;
        bool is_background = false;
};

class CImageManager {
    public:
        ImageData load_from_disk( const fs::path& path, const std::string& name );
        ImageData load_from_memory( void* data, int length, const std::string& name );

        static CImageManager& get( );
        ImageData get_image( const std::string& img_name );

        // delete copy & move constructors since there must only be 1 instance
        CImageManager( const CImageManager& ) = delete;
        CImageManager& operator=( const CImageManager& ) = delete;
        CImageManager( CImageManager&& ) = delete;
        CImageManager& operator=( CImageManager&& ) = delete;

    private:
        CImageManager( ) = default;
        ImageData upload( const unsigned char* pixels, int width, int height, const std::string& name );

        std::unordered_map<std::string, ImageData> m_images = { };
};
