#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "../slim/core/base.h"


u8* readImageComponents(char *filename, ImageInfo &info) {
    if (info.flags.flip) stbi_set_flip_vertically_on_load(true);
    int width = 0, height = 0, comp = 0;
    u8 *data = stbi_load(filename, &width, &height, &comp, 0);
    if (data) {
        info.flags.alpha = comp == 4;
        info.updateDimensions(width, height);
    }
    return data;
}

RawImage loadRawImage(char *filename) {
    RawImage raw_image;
    raw_image.content = readImageComponents(filename, raw_image);
    return raw_image;
}

void freeRawImage(RawImage &raw_image) {
    stbi_image_free(raw_image.content);
    raw_image.content = nullptr;
}