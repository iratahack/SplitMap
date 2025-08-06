# SplitMap

_SplitMap_ is a utility for splitting and compressing tile maps in the _.nxm_ format, as well as generating tables for collectible items. The output source code is in Z80 assembly language, compatible with the z88dk assembler (_z80asm_). Level data is compressed using [ZX0 by Einar Saukas](https://github.com/einar-saukas/ZX0).

## Basic Operation

A complete tilemap (e.g., all game levels), created in _Tiled_ and converted to _.nxm_ format using Gfx2Nxt, is provided as input to the utility. _SplitMap_ divides the tilemap into individual levels of a size specified by the _--level-size_ parameter. Each level is then compressed and saved to a file. Additionally, a table is generated containing the address of each level, ordered left-to-right and top-to-bottom.

The utility can also scan each level for collectible items specified by the _--item_ parameter. All items of the specified type are listed in a table for each level. A top-level table for each item type is also generated, pointing to the item tables for each level.

### Level Table Format

Below is an example of a level table in z80asm assembly format:

```assembly
        public  _levelTable

_levelTable:
        dw      ForestEscape_0000
        dw      ForestEscape_0100
        dw      ForestEscape_0200
        ...

ForestEscape_0000:
        binary  "ForestEscape_0000.nxm.zx0"
ForestEscape_0100:
        binary  "ForestEscape_0100.nxm.zx0"
ForestEscape_0200:
        binary  "ForestEscape_0200.nxm.zx0"
        ... 
```

### Item Table Format

Below is an example of an item table in z80asm assembly format:

```assembly
        public  _coinTables

_coinTables:
        dw      level0coin
        dw      level1coin
        dw      level2coin
        ...

level0coin:
        db      $01, $a8, $68, $00
        db      $01, $70, $78, $00
        db      $01, $d0, $90, $00
        db      $ff

level1coin:
        db      $01, $00, $60, $00
        db      $01, $c8, $70, $00
        db      $ff

level2coin:
        db      $01, $60, $48, $00
        db      $01, $20, $50, $00
        db      $01, $d8, $60, $00
        db      $01, $c8, $68, $00
        db      $01, $e8, $88, $00
        db      $01, $f0, $90, $00
        db      $ff

        ...
```

Each item entry uses the following format (a value of `$ff` in the flags field marks the end of the table):

- **u8_t flags**: Bit 0 indicates visibility
- **u8_t xPos**: X pixel position within the level
- **u8_t yPos**: Y pixel position within the level
- **u8_t frame**: Animation frame or item-specific value

## Usage

_SplitMap_ provides several parameters to control how maps are processed. Below is a summary of the available options:

```sh
Usage: ./splitmap --map <filename> --map-size <WIDTHxHEIGHT> --level-size <WIDTHxHEIGHT> [--blank <tile ID>] [--tablesection <section name>] [--[ro]datasection <section name>] [--items-only] [--item <name>,<ID>[,<FRAME>] [...]]
```

| Parameter         | Description                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| --map             | Name of the map file (in .nxm format)                                       |
| --map-size        | Size of the complete map, in WIDTHxHEIGHT format                            |
| --level-size      | Size of each level within the map, in WIDTHxHEIGHT format                   |
| --blank           | ID of the blank tile (depends on the tilemap)                               |
| --tablesection    | Name of the linker section for item tables                                  |
| --rodatasection   | Name of the linker section for read-only data (RODATA)                      |
| --datasection     | Name of the linker section for writable data (DATA)                         |
| --items-only      | Output only items and item tables; do not output compressed map data        |
| --item            | Define a collectible item within the map (format: &lt;name&gt;,&lt;ID&gt;[,&lt;FRAME&gt;])    |

## Examples

To generate item tables for multiple item types using **SplitMap**, you need to use the `--item` parameter multiple times—once for each item type you want to include. Each `--item` parameter specifies the item’s name, its tile ID in the map, and optionally, a default animation frame.

### Example Command

```sh
./splitmap --map mymap.nxm --map-size 128x64 --level-size 32x16 \
  --item coin,5 \
  --item gem,6,2 \
  --item heart,7
```

**Explanation:**
- `--item coin,5` tells SplitMap to look for tiles with ID 5 and treat them as "coin" items.
- `--item gem,6,2` looks for tiles with ID 6, treats them as "gem" items, and assigns a default frame value of 2.
- `--item heart,7` looks for tiles with ID 7 and treats them as "heart" items.

SplitMap will scan each level for these item types and generate a separate item table for each type. It will also create a top-level table for each item type, pointing to the per-level tables.

### Output

- For each item type, you’ll get a table (e.g., `_coinTables`, `_gemTables`, `_heartTables`) listing the addresses of the item tables for each level.
- Each per-level item table contains the positions and properties of the items found in that level.

### Tips

- You can specify as many `--item` parameters as needed for different collectible types.
- Make sure the tile IDs match those used in your map editor.
- If you only want item tables (and not compressed map data), add the `--items-only` flag.

---

