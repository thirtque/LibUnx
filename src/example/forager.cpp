#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "unx.h"

int main() {
    std::ios_base::sync_with_stdio(false);

    std::string home = std::getenv("HOME");
    std::string path = home + "/.steam/steam/steamapps/common/Forager/assets/game.unx";

    std::array<std::uint8_t, 4096> streamBuffer{};
    std::fstream stream;
    stream.rdbuf()->pubsetbuf(reinterpret_cast<char*>(streamBuffer.data()), streamBuffer.size());
    stream.open(path, std::fstream::binary | std::fstream::in);

    stream.seekg(0, std::fstream::beg);

    auto reader = unx::Reader(stream);
    auto unx = reader.read();

    stream.close();

    std::filesystem::create_directories("sections");
    std::ofstream f("sections/strg.txt", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
    for (auto& string : unx.strings) {
        f << string.value << "\n";
    }
    f.close();

    std::filesystem::create_directories("sections/txtr");
    auto i = 0;
    for (auto texture : unx.textures) {
        std::ofstream textureStream("sections/txtr/" + std::to_string(i++) + ".png",
            std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
        textureStream.write(reinterpret_cast<const char*>(texture.data.data()), texture.size);
        textureStream.close();
    }

    for (auto& sprite : unx.sprites) {
        std::cout << sprite.name->value << "\n\tsize.width: " << sprite.size.width
                  << "\n\tsize.height: " << sprite.size.height << "\n\torigin.x: " << sprite.origin.x
                  << "\n\torigin.y: " << sprite.origin.y << "\n\tframeCount: " << sprite.frames.size() << std::endl;
    }
}