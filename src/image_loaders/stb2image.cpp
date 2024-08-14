#include "stb_image_loader.h"
#include "../slim/core/image.h"
#include "../slim/serialization/image.h"
#include "../slim/platforms/win32_base.h"


int main(int argc, char *argv[]) {
    ImageInfo info{};

    char* bitmap_file_path = argv[1];
    char* image_file_path = argv[2];

    bool byte_color = false;
    bool raw = false;
    for (u8 i = 3; i < (u8)argc; i++)
        if (     argv[i][0] == '-' && argv[i][1] == 'f') info.flags.flip = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'c') info.flags.channel = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'l') info.flags.linear = true;
        else if (argv[i][0] == '-' && argv[i][1] == 't') info.flags.tile = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'n') info.flags.normal = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'b') byte_color = true;
        else if (argv[i][0] == '-' && argv[i][1] == 'r') raw = true;
        else return 0;

    u8* components = readImageComponents(bitmap_file_path, info);
    if (raw) {
        RawImage image;
        *((ImageInfo*)(&image)) = info;
        image.content = components;
        save(image, image_file_path);
    } else if (byte_color) {
        ByteColorImage image;
        *((ImageInfo*)(&image)) = info;
        image.content = new ByteColor[image.size];
        componentsToByteColors(components, image, image.content);
        save(image, image_file_path);
    } else if (info.flags.channel) {
        FloatImage image;
        *((ImageInfo*)(&image)) = info;
        image.content = new f32[image.size * (info.flags.alpha ? 4 : 3)];
        componentsToChannels(components, image, image.content);
        save(image, image_file_path);
    } else {
        PixelImage image;
        *((ImageInfo*)(&image)) = info;
        image.content = new Pixel[image.size];
        componentsToPixels(components, image, image.content);
        save(image, image_file_path);
    }

    return 0;
}