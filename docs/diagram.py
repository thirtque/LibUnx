# Uses https://github.com/tabatkins/railroad-diagrams to generate SVG diagrams of the Unx file format

from railroad import NonTerminal, Diagram as DefaultDiagram, Sequence, Group, Terminal, OneOrMore, Comment, Choice, \
    Stack, Start

cssFile = open("style.css", "r")
css = cssFile.read()
cssFile.close()


def Diagram(path, *items, **kwargs):
    kwargs["css"] = css

    diagram = DefaultDiagram(*items, **kwargs)

    svg = open("svg/" + path, "w")
    diagram.writeSvg(svg.write)
    svg.close()


if __name__ == '__main__':
    Diagram(
        "unx.svg",
        Start("simple", "Unx"),
        Group(
            Stack(
                Terminal("FORM"),
                NonTerminal("uint32 size"),
            ),
            "byte header[8]"
        ),
        OneOrMore(
            Group(
                Sequence(
                    Group(
                        Stack(
                            Choice(2, "STRG", "TEXT", "TPAG", "SPRT", Comment("...")),
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

    Diagram(
        "string-section.svg",
        Start("simple", "StringSection"),
        Group(
            Sequence(
                Group(
                    Stack(
                        Terminal("STRG"),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Stack(
                        NonTerminal("uint32 number"),
                        NonTerminal("String* (uint32) strings[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte stringSection[8 + stringSection.info.size]"
        )
    )

    Diagram(
        "string.svg",
        Start("simple", "String"),
        Group(
            Sequence(
                NonTerminal("uint32 length"),
                NonTerminal("char value[length]"),
                Terminal("\\0"),
            ),
            "byte string[8 + string.length]"
        ),
    )

    Diagram(
        "texture-section.svg",
        Start("simple", "TextureSection"),
        Group(
            Sequence(
                Group(
                    Stack(
                        Terminal("TXTR"),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Stack(
                        NonTerminal("uint32 number"),
                        NonTerminal("Texture* (uint32) textures[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte textureSection[8 + textureSection.info.size]"
        )

    )

    Diagram(
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

    Diagram(
        "texture-data.svg",
        Start("simple", "TextureData"),
        Group(
            NonTerminal("Png"),
            "byte textureData[number of bytes to the next texture/end of the section]",
        )

    )

    Diagram(
        "texture-region-section.svg",
        Start("simple", "TextureRegionSection"),
        Group(
            Sequence(
                Group(
                    Stack(
                        Terminal("TPAG"),
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

    Diagram(
        "texture-region.svg",
        Start("simple", "TextureRegion"),
        Group(
            Sequence(
                Stack(
                    NonTerminal("uint16 offsetX"),
                    NonTerminal("uint16 offsetY"),
                ),
                Stack(
                    NonTerminal("uint16 width"),
                    NonTerminal("uint16 height"),
                ),
                Stack(
                    NonTerminal("uint16 originX"),
                    NonTerminal("uint16 originY"),
                ),
                NonTerminal("byte unknown[8]"),
                NonTerminal("uint16 textureIndex"),
            ),
            "byte textureRegion[22]"
        ),
    )

    Diagram(
        "sprite-section.svg",
        Start("simple", "SpriteSection"),
        Group(
            Sequence(
                Group(
                    Stack(
                        Terminal("SPRT"),
                        NonTerminal("uint32 size"),
                    ),
                    "byte info[8]"
                ),
                Group(
                    Stack(
                        NonTerminal("uint32 number"),
                        NonTerminal("Sprite* (uint32) sprites[number]"),
                    ),
                    "byte data[info.size]"
                ),
            ),
            "byte spriteSection[8 + spriteSection.info.size]"
        )
    )

    Diagram(
        "sprite.svg",
        Start("simple", "Sprite"),
        Group(
            Sequence(
                NonTerminal("String.value* (uint32) name"),
                Stack(
                    NonTerminal("uint32 width"),
                    NonTerminal("uint32 height")
                ),
                NonTerminal("byte unknown[64]"),
                Stack(
                    NonTerminal("uint32 frameCount"),
                    NonTerminal("TextureRegion* (uint32) frames[frameCount]")
                ),
                NonTerminal("byte unknown[?]"),
            ),
            "byte sprite[76 + 4 * frameCount + ?]"
        ),
    )
