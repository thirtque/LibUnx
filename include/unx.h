#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace unx {
    using unknown_map = std::map<std::uint32_t, std::vector<std::uint8_t>>;

    struct Size {
            std::uint32_t width;
            std::uint32_t height;

            explicit Size(std::uint32_t width, std::uint32_t height):
                width(width),
                height(height) {}
    };

    struct Point {
            std::uint32_t x;
            std::uint32_t y;

            explicit Point(std::uint32_t x, std::uint32_t y):
                x(x),
                y(y) {}
    };

    struct Header {
            std::string signature;
            std::uint32_t size;

            explicit Header(std::string&& signature, std::uint32_t size):
                signature(std::move(signature)),
                size(size) {}
    };

    struct SectionInfo {
            std::string name;
            std::uint32_t size;
            std::uint32_t position;

            SectionInfo() = default;

            explicit SectionInfo(std::string&& name, std::uint32_t size, std::uint32_t position):
                name(std::move(name)),
                size(size),
                position(position) {}
    };

    struct String {
            std::uint32_t position;
            std::uint32_t length;
            std::string value;

            explicit String(std::uint32_t position, std::uint32_t length, std::string&& value):
                position(position),
                length(length),
                value(std::move(value)) {}
    };

    struct Texture {
            std::uint32_t size;
            std::uint32_t position;
            std::vector<uint8_t> data;

            unknown_map unknowns;

            explicit Texture(std::uint32_t size, std::uint32_t position, unknown_map&& unknowns):
                size(size),
                position(position),
                data(0),
                unknowns(unknowns) {}
    };

    struct TextureRegion {
            std::uint32_t position;
            Point offset;
            Size size;
            Point origin;
            std::uint32_t textureIndex;
            Texture* texture;

            unknown_map unknowns;

            explicit TextureRegion(
                std::uint32_t position, Point offset, Size size, Point origin, std::uint32_t textureIndex, Texture* texture, unknown_map&& unknowns):
                position(position),
                offset(offset),
                size(size),
                origin(origin),
                textureIndex(textureIndex),
                texture(texture),
                unknowns(std::move(unknowns)) {}
    };

    struct Sprite {
            String* name;
            Size size;
            Point origin;
            std::vector<TextureRegion*> frames;

            unknown_map unknowns;

            explicit Sprite(
                String* name, Size size, Point origin, std::vector<TextureRegion*>&& frames, unknown_map&& unknowns):
                name(name),
                size(size),
                origin(origin),
                frames(std::move(frames)),
                unknowns(std::move(unknowns)) {}
    };

    struct Unx {
            std::vector<String> strings;
            std::vector<Texture> textures;
            std::vector<TextureRegion> textureRegions;
            std::vector<Sprite> sprites;
    };

    class Reader {
        private:
            const std::string stringSectionName = "STRG";
            const std::string textureSectionName = "TXTR";
            const std::string textureRegionSectionName = "TPAG";
            const std::string spriteSectionName = "SPRT";

            std::istream& stream;

        public:
            explicit Reader(std::istream& stream):
                stream(stream) {}

            Reader(const Reader&) = default;
            Reader(Reader&&) noexcept = default;

            virtual ~Reader() = default;

            void setPosition(std::uint32_t position);
            std::uint32_t getPosition();

            std::vector<std::uint8_t> readBytes(std::uint32_t length);
            std::uint16_t readUInt16();
            std::uint32_t readUInt32();
            std::string readString(std::uint32_t length);

            Header readHeader();
            SectionInfo readSectionInfo();
            void readSection(const SectionInfo& sectionInfo, Unx& unx);
            void readTextureSection(const SectionInfo& section, Unx& unx);
            void readStringSection(const SectionInfo& section, Unx& unx);
            void readTextureRegionSection(const SectionInfo& section, Unx& unx);
            void readBackgroundSection(const SectionInfo& section, Unx& unx);
            void readSpriteSection(const SectionInfo& section, Unx& unx);

            Unx read();

            Reader& operator=(const Reader&) = delete;
            Reader& operator=(Reader&&) = delete;
    };

    using section_reader = void (Reader::*)(const SectionInfo&, Unx&);
} // namespace unx
