#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "textures.hpp"
#include "core/helpers/paths.hpp"
#include "core/logger/logger.hpp"

#include <glad/gl.h>
#include <stb_image.h>
#include <core/network/network.hpp>

GLuint Textures::upload_image_to_gpu(const ImageData& data) {

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.pixels.data());

    return image_texture;
}

Textures::ImageData Textures::load_image(const std::string& appid) {
    if(!fs::exists(paths::cache_dir())) {
        fs::create_directory(paths::cache_dir());
    }

    Network::download_game_image(appid); //this silently fails, but its fine for now.

    auto filename = paths::cache_dir() / (appid + ".jpg");
    FILE* f = fopen(filename.string().c_str(), "rb");
    if (f == NULL) {
        get_logger().error("Failed to open: " + appid + ".jpg during loading");
        return {};
    }

    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);

    if (file_size == -1) {
        get_logger().error("Failed to seek through: " + appid + ".jpg");
        fclose(f);
        return {};
    }

    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);

    ImageData image_data;
    if(file_size > 0) {
        int width = 0, height = 0;
        unsigned char* pixels = stbi_load_from_memory((const unsigned char*)file_data, file_size, &width, &height, nullptr, 4);
        if (pixels == NULL) {
            IM_FREE(file_data);
            return {};
        }

        image_data.pixels.assign(pixels, pixels + (width * height * 4));
        image_data.width = width;
        image_data.height = height;
        image_data.appid = appid;

        stbi_image_free(pixels);
        IM_FREE(file_data);
    } else {
        IM_FREE(file_data);
        return {};
    }

    return image_data;
}
