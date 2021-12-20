#include "unx.h"

namespace unx {
    // region struct Reader

    void Reader::setPosition(std::uint32_t position) {
        stream.seekg(position, std::istream::beg);
    }

    std::uint32_t Reader::getPosition() {
        return stream.tellg();
    }

    std::vector<std::uint8_t> Reader::readBytes(std::uint32_t length) {
        std::vector<std::uint8_t> value(length);
        stream.read(reinterpret_cast<char*>(value.data()), length);

        return value;
    }

    std::uint16_t Reader::readUInt16() {
        std::uint16_t value{};
        stream.read(reinterpret_cast<char*>(&value), 2);

        return value;
    }

    std::uint32_t Reader::readUInt32() {
        std::uint32_t value{};
        stream.read(reinterpret_cast<char*>(&value), 4);

        return value;
    }

    std::string Reader::readString(std::uint32_t length) {
        std::string value(length, '\0');
        stream.read(reinterpret_cast<char*>(value.data()), length);

        return value;
    }

    Header Reader::readHeader() {
        std::string signature = readString(4);
        std::uint32_t size = readUInt32();

        return Header{
            std::move(signature),
            size,
        };
    }

    SectionInfo Reader::readSectionInfo() {
        std::string name = readString(4);
        std::uint32_t size = readUInt32();
        std::uint32_t position = getPosition();

        return SectionInfo{
            std::move(name),
            size,
            position,
        };
    }

    // void detectSection(std::uint32_t pointer, const std::vector<Section>& sections) {
    //     for (auto& section : sections) {
    //         if (pointer >= section.position && pointer <= section.position + section.size) {
    //             std::cout << section.name << std::endl;
    //             break;
    //         }
    //     }
    // }

    void Reader::readTextureSection(const SectionInfo& section, Unx& unx) {
        auto number = readUInt32();

        std::vector<std::uint32_t> positions;
        positions.reserve(number);
        for (auto i = 0; i < number; i++) {
            positions.push_back(readUInt32());
        }

        std::vector<Texture> textures;
        textures.reserve(number);
        for (auto& position : positions) {
            setPosition(position);

            unknown_map unknowns;

            // Skip 8 bytes of unknown data
            unknowns[getPosition() - position] = std::move(readBytes(8));

            auto dataPosition = readUInt32();

            textures.emplace_back(0, dataPosition, std::move(unknowns));
        }
        for (auto i = 0; i < number - 1; ++i) {
            textures[i].size = textures[i + 1].position - textures[i].position;
        }
        textures[number - 1].size = section.size - (textures[number - 1].position - section.position);

        for (auto& texture : textures) {
            setPosition(texture.position);

            texture.data.resize(texture.size);
            stream.read(reinterpret_cast<char*>(texture.data.data()), texture.size);
        }

        unx.textures = std::move(textures);
    }

    void Reader::readStringSection(const SectionInfo& section, Unx& unx) {
        auto number = readUInt32();

        std::vector<std::uint32_t> positions;
        positions.reserve(number);
        for (auto i = 0; i < number; ++i) {
            positions.push_back(readUInt32());
        }

        std::vector<String> strings;
        strings.reserve(number);
        for (auto& position : positions) {
            setPosition(position);

            auto length = readUInt32();
            auto value = readString(length);

            strings.emplace_back(position, length, std::move(value));
        }

        unx.strings = std::move(strings);
    }

    void Reader::readTextureRegionSection(const SectionInfo& section, Unx& unx) {
        auto number = readUInt32();

        std::vector<std::uint32_t> positions;
        positions.reserve(number);
        for (auto i = 0; i < number; ++i) {
            positions.push_back(readUInt32());
        }

        std::vector<TextureRegion> textureRegions;
        for (auto& position : positions) {
            setPosition(position);

            unknown_map unknowns;

            auto offsetX = readUInt16();
            auto offsetY = readUInt16();
            auto width = readUInt16();
            auto height = readUInt16();
            auto originX = readUInt16();
            auto originY = readUInt16();

            // Skip 8 bytes of unknown data
            unknowns[getPosition() - position] = std::move(readBytes(8));

            auto textureIndex = readUInt16();

            textureRegions.emplace_back(position,
                Point(offsetX, offsetY),
                Size(width, height),
                Point(originX, originY),
                textureIndex,
                &unx.textures[textureIndex],
                std::move(unknowns));
        }

        unx.textureRegions = std::move(textureRegions);
    }

    void Reader::readSpriteSection(const SectionInfo& section, Unx& unx) {
        auto number = readUInt32();

        std::vector<std::uint32_t> positions;
        positions.reserve(number);
        for (auto i = 0; i < number; ++i) {
            positions.push_back(readUInt32());
        }

        std::vector<Sprite> sprites;
        for (auto& position : positions) {
            setPosition(position);

            unknown_map unknowns;

            String* name = nullptr;
            auto nameStringPosition = readUInt32() - 4;
            for (auto& string : unx.strings) {
                if (string.position == nameStringPosition) {
                    name = &string;
                    break;
                }
            }
            if (name == nullptr) {
                throw std::runtime_error("Unable to find string for sprite name");
            }

            // Skip 72 bytes of unknown data
            unknowns[getPosition() - position] = std::move(readBytes(72));

            auto frameCount = readUInt32();
            std::vector<TextureRegion*> frames;
            for (auto j = 0; j < frameCount; j++) {
                auto frameTextureRegionPosition = readUInt32();

                TextureRegion* frameTextureRegion = nullptr;
                for (auto& textureRegion : unx.textureRegions) {
                    if (textureRegion.position == frameTextureRegionPosition) {
                        frameTextureRegion = &textureRegion;
                        break;
                    }
                }
                if (frameTextureRegion == nullptr) {
                    throw std::runtime_error("Unable to find texture region for sprite frame");
                }
                frames.push_back(frameTextureRegion);
            }

            sprites.emplace_back(name, std::move(frames), std::move(unknowns));
        }

        unx.sprites = std::move(sprites);
    }

    // void Reader::readBackgroundSection(const SectionInfo& section, Unx& unx) {
    //     auto number = readUInt32();
    //
    //     std::vector<std::uint32_t> positions;
    //     positions.reserve(number);
    //     for (auto i = 0; i < number; i++) {
    //         positions.push_back(readUInt32());
    //     }
    //
    //     for (auto& position : positions) {
    //         setPosition(position);
    //
    //         auto unknown1 = readUInt32();
    //         auto unknown2 = readUInt32();
    //         auto unknown3 = readUInt32();
    //         auto unknown4 = readUInt32();
    //         auto unknown5 = readUInt32();
    //         auto unknown6 = readUInt32();
    //         auto unknown7 = readUInt32();
    //         auto unknown8 = readUInt32();
    //         auto unknown9 = readUInt32();
    //         auto unknown10 = readUInt32();
    //         auto unknown11 = readUInt32();
    //         auto unknown12 = readUInt32();
    //         auto unknown13 = readUInt32();
    //         auto unknown14 = readUInt32();
    //         auto unknown15 = readUInt32();
    //         auto unknown16 = readUInt32();
    //         auto unknown17 = readUInt32();
    //         auto unknown18 = readUInt32();
    //         auto unknown19 = readUInt32();
    //         auto unknown20 = readUInt32();
    //         auto unknown21 = readUInt32();
    //         auto unknown22 = readUInt32();
    //         auto unknown23 = readUInt32();
    //         auto unknown24 = readUInt32();
    //         auto unknown25 = readUInt32();
    //         auto unknown26 = readUInt32();
    //         auto unknown27 = readUInt32();
    //         auto unknown28 = readUInt32();
    //         auto unknown29 = readUInt32();
    //     }
    // }

    void Reader::readSection(const SectionInfo& sectionInfo, Unx& unx) {
        static const std::map<const std::string, const section_reader> readers = {
            // {"BGND", &readBackgroundSection},

            {stringSectionName, &Reader::readStringSection},
            {textureSectionName, &Reader::readTextureSection},
            {textureRegionSectionName, &Reader::readTextureRegionSection},
            {spriteSectionName, &Reader::readSpriteSection},
        };
        auto reader = readers.find(sectionInfo.name);
        if (reader != readers.end()) {
            setPosition(sectionInfo.position);
            (*this.*(reader->second))(sectionInfo, unx);
        }
    }

    Unx Reader::read() {
        auto header = readHeader();

        std::map<std::string, SectionInfo> sectionInfos;
        while (getPosition() < 8 + header.size) {
            auto sectionInfo = readSectionInfo();
            setPosition(sectionInfo.position + sectionInfo.size);

            sectionInfos.emplace(sectionInfo.name, std::move(sectionInfo));
        }

        Unx unx;
        readSection(sectionInfos[stringSectionName], unx);
        readSection(sectionInfos[textureSectionName], unx);
        readSection(sectionInfos[textureRegionSectionName], unx);
        readSection(sectionInfos[spriteSectionName], unx);
        return unx;
    }

    // endregion struct Reader
} // namespace unx