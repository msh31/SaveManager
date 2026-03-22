#pragma once

namespace Textures {
    struct ImageData {
        std::string appid;
        std::vector<unsigned char> pixels;
        int width;
        int height;
    };

    ImageData load_image(const std::string& appid);
    GLuint upload_image_to_gpu(const ImageData& data);
};

