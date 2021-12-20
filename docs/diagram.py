# Uses https://github.com/tabatkins/railroad-diagrams to generate SVG diagrams of the Unx file format

from railroad import Stack, Sequence, Group as _Group, Skip, Diagram, Start, Terminal, NonTerminal, OneOrMore, Choice, \
    Comment

import railroad

railroad.VS = 16
railroad.AR = 8
railroad.STROKE_ODD_PIXEL_LENGTH = False
railroad.INTERNAL_ALIGNMENT = 'left'
railroad.CHAR_WIDTH = 9
railroad.COMMENT_CHAR_WIDTH = 8

cssFile = open("style.css", "r")
css = cssFile.read()
cssFile.close()


def DiagramWithCssToSvg(path, *items, **kwargs):
    kwargs["css"] = css
    kwargs["INTERNAL_ALIGNMENT"] = "left"

    diagram = Diagram(*items, **kwargs).format(8)

    svgFile = open("svg/" + path, "w")
    diagram.writeSvg(svgFile.write)
    svgFile.close()


def Group(item, label):
    return _Group(Sequence(Skip(), item), label)


if __name__ == '__main__':
    DiagramWithCssToSvg(
        "unx.svg",
        Start("simple", "Unx"),
        Stack(
            Group(
                Sequence(
                    Terminal("FORM"),
                    NonTerminal("uint32 size"),
                ),
                "byte header[8]"
            ),
            OneOrMore(
                Group(
                    Sequence(
                        Group(
                            Sequence(
                                Choice(2, "\"STRG\"", "\"TEXT\"", "\"TPAG\"", "\"SPRT\"", Comment("...")),
                                NonTerminal("uint32 size"),
                            ),
                            "byte info[8]",
                        ),
                        Group(
                            Choice(
                                2,
                                NonTerminal("StringSection"),
                                NonTerminal("TextureSection"),
                                NonTerminal("TextureRegionSection"),
                                NonTerminal("SpriteSection"),
                                Comment("...")
                            ),
                            "byte data[info.size]"
                        )
                    ),
                    "byte section[8 + section.info.size]"
                )
            )
        )
    )

    DiagramWithCssToSvg(
        "string-section.svg",
        Start("simple", "StringSection"),
        Group(
            Stack(
                Group(
                    Sequence(
                        Terminal("\"STRG\""),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Sequence(
                        NonTerminal("uint32 number"),
                        NonTerminal("String* (uint32) strings[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte stringSection[8 + stringSection.info.size]"
        )
    )

    DiagramWithCssToSvg(
        "string.svg",
        Start("simple", "String"),
        Group(
            Sequence(
                NonTerminal("uint32 length"),
                NonTerminal("char value[length]"),
                Terminal("\"\\0\""),
            ),
            "byte string[8 + string.length]"
        ),
    )

    DiagramWithCssToSvg(
        "texture-section.svg",
        Start("simple", "TextureSection"),
        Group(
            Stack(
                Group(
                    Sequence(
                        Terminal("\"TXTR\""),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Sequence(
                        NonTerminal("uint32 number"),
                        NonTerminal("Texture* (uint32) textures[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte textureSection[8 + textureSection.info.size]"
        )

    )

    DiagramWithCssToSvg(
        "texture.svg",
        Start("simple", "Texture"),
        Group(
            Sequence(
                NonTerminal("byte unknown[8]"),
                NonTerminal("TextureData* (uint32) textureData"),
            ),
            "byte texture[12]"
        ),
    )

    DiagramWithCssToSvg(
        "texture-data.svg",
        Start("simple", "TextureData"),
        Group(
            NonTerminal("Png"),
            "byte textureData[number of bytes to the next texture/end of the section]",
        )

    )

    DiagramWithCssToSvg(
        "texture-region-section.svg",
        Start("simple", "TextureRegionSection"),
        Group(
            Stack(
                Group(
                    Sequence(
                        Terminal("\"TPAG\""),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Stack(
                        NonTerminal("uint32 number"),
                        NonTerminal("TextureRegion* (uint32) textures[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte textureRegionSection[8 + textureRegionSection.info.size]"
        )
    )

    DiagramWithCssToSvg(
        "texture-region.svg",
        Start("simple", "TextureRegion"),
        Group(
            Stack(
                Group(
                    Sequence(
                        NonTerminal("uint16 x"),
                        NonTerminal("uint16 y"),
                    ),
                    "byte position[4]"
                ),
                Group(
                    Sequence(
                        NonTerminal("uint16 width"),
                        NonTerminal("uint16 height"),
                    ),
                    "byte size[4]"
                ),
                Group(
                    Sequence(
                        NonTerminal("uint16 x"),
                        NonTerminal("uint16 y"),
                    ),
                    "byte padding[4]"
                ),
                NonTerminal("byte unknown[8]"),
                NonTerminal("uint16 textureIndex"),
            ),
            "byte textureRegion[22]"
        ),
    )

    DiagramWithCssToSvg(
        "sprite-section.svg",
        Start("simple", "SpriteSection"),
        Group(
            Stack(
                Group(
                    Sequence(
                        Terminal("\"SPRT\""),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Sequence(
                        NonTerminal("uint32 number"),
                        NonTerminal("Sprite* (uint32) sprites[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte spriteSection[8 + spriteSection.info.size]"
        )
    )

    DiagramWithCssToSvg(
        "sprite.svg",
        Start("simple", "Sprite"),
        Group(
            Stack(
                NonTerminal("String.value* (uint32) name"),
                Group(
                    Sequence(
                        NonTerminal("uint32 width"),
                        NonTerminal("uint32 height")
                    ),
                    "byte size[8]"
                ),
                NonTerminal("byte unknown[36]"),
                Group(
                    Sequence(
                        NonTerminal("uint32 x"),
                        NonTerminal("uint32 y")
                    ),
                    "byte origin[8]"
                ),
                NonTerminal("byte unknown[20]"),
                Group(
                    Stack(
                        NonTerminal("uint32 frameCount"),
                        NonTerminal("TextureRegion* (uint32) frames[frameCount]")
                    ),
                    "byte frames[4 + 4 * frameCount]"
                ),
                NonTerminal("byte unknown[?]"),
            ),
            "byte sprite[80 + 4 * frameCount + ?]"
        )
    )
