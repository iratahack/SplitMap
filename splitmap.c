#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "zx0.h"

#define MAX_OFFSET_ZX0 32640
#define MAX_OFFSET_ZX7 2176

#define MAX_ITEMS 256
#define MAX_STRIPS 16

typedef struct item_data
{
    struct item_data *next;
    int id;
    unsigned char frame;
} item_data_t;

typedef struct
{
    char *tableName;
    int count;
    item_data_t *data;
} item_t;

item_t items[MAX_ITEMS];

item_t *addItem(char *name, int id, unsigned char frame)
{
    int n;
    item_t *foundEntry = NULL;
    item_data_t *itemData = NULL;

    // Find the item with the same name
    for (n = 0; n < MAX_ITEMS; n++)
    {
        if (items[n].tableName == NULL)
        {
            foundEntry = &items[n];
            foundEntry->tableName = name;
            break;
        }
        else if (strcmp(items[n].tableName, name) == 0)
        {
            foundEntry = &items[n];
            break;
        }
    }

    if (n == MAX_ITEMS)
    {
        fprintf(stderr, "No space to add item (%s)\n", name);
        exit(-1);
    }

    if ((itemData = malloc(sizeof(item_data_t))) == NULL)
    {
        perror("Could not allocate memory for item");
        exit(-1);
    }

    itemData->id = id;
    itemData->frame = frame;

    // Add to list
    itemData->next = foundEntry->data;
    foundEntry->data = itemData;

    return (foundEntry);
}

void writeRaw(char *fileName, unsigned char *input_data, int input_size)
{
    FILE *ofp = fopen(fileName, "wb");
    if (!ofp)
    {
        fprintf(stderr, "Error: Cannot create output file %s\n", fileName);
        exit(1);
    }

    if (fwrite(input_data, sizeof(char), input_size, ofp) != input_size)
    {
        fprintf(stderr, "Error: Cannot write output file %s\n", fileName);
        fclose(ofp);
        exit(1);
    }

    fclose(ofp);
}

void doCompression(char *fileName, unsigned char *input_data, int input_size)
{
    FILE *ofp = NULL;
    char *output_name = NULL;
    int output_size;
    int skip = 0;
    unsigned char *output_data;
    int delta;
    int quick_mode = FALSE;
    int backwards_mode = FALSE;
    int classic_mode = TRUE;
    void *blockStart = NULL;

    output_name = (char *)malloc(strlen(fileName) + 5);
    strcpy(output_name, fileName);
    strcat(output_name, ".zx0");

    /* create output file */
    ofp = fopen(output_name, "wb");
    if (!ofp)
    {
        fprintf(stderr, "Error: Cannot create output file %s\n", output_name);
        free(output_name);
        exit(1);
    }

    output_data = compress(optimize(input_data, input_size, skip, quick_mode ? MAX_OFFSET_ZX7 : MAX_OFFSET_ZX0, &blockStart),
                           input_data, input_size, skip, backwards_mode, !classic_mode && !backwards_mode, &output_size, &delta);

    free(blockStart);
    /* write output file */
    if (fwrite(output_data, sizeof(char), output_size, ofp) != output_size)
    {
        fprintf(stderr, "Error: Cannot write output file %s\n", output_name);
        fclose(ofp);
        free(output_name);
        free(output_data);
        exit(1);
    }

    /* close output file */
    fclose(ofp);
    free(output_name);
    free(output_data);
}

int main(int argc, char *argv[])
{
    char *outputFileName = NULL;
    char *fileName = NULL;
    char *ext = NULL;
    char *tablesSection = NULL;
    char *dataSection = NULL;
    char *roDataSection = NULL;
    unsigned char *inputData = NULL;
    unsigned char *compressData = NULL;
    FILE *inFile = NULL;
    FILE *cFile = NULL;
    int mapWidth = 0;
    int mapHeight = 0;
    int levelWidth = 0;
    int levelHeight = 0;
    int compressWidth = -1;
    int compressHeight = -1;
    int itemsOnly = 0;
    int inputSize = 0;
    int compressSize = 0;
    int blank = 0;

    memset(items, 0, sizeof(items));

    int horizontalStrip = 0;
    int noCompress = 0;

    if (argc < 2)
    {
        printf(
            "%s --map <filename> --map-size <WIDTHxHEIGHT> --level-size <WIDTHxHEIGHT> [--blank <tile ID>] [--tablessection <section name>] [--[ro]datasection <section name>] [--items-only] [--horizontal-strip] [--no-compress] [--item <name>,<ID>[,<FRAME>] [...]]\n",
            argv[0]);
        return 0;
    }

    for (int param = 1; param < argc; param++)
    {
        if (!strcmp(argv[param], "--items-only"))
        {
            itemsOnly = 1;
        }
        else if (!strcmp(argv[param], "--horizontal-strip"))
        {
            horizontalStrip = 1;
        }
        else if (!strcmp(argv[param], "--no-compress"))
        {
            noCompress = 1;
        }
        else if (!strcmp(argv[param], "--map-size"))
        {
            param++;
            mapWidth = atoi(strtok(argv[param], "x"));
            mapHeight = atoi(strtok(NULL, "x"));
        }
        else if (!strcmp(argv[param], "--level-size"))
        {
            param++;
            levelWidth = atoi(strtok(argv[param], "x"));
            levelHeight = atoi(strtok(NULL, "x"));
        }
        else if (!strcmp(argv[param], "--output-size"))
        {
            param++;
            compressWidth = atoi(strtok(argv[param], "x"));
            compressHeight = atoi(strtok(NULL, "x"));
        }
        else if (!strcmp(argv[param], "--map"))
        {
            param++;
            fileName = argv[param];
        }
        else if (!strcmp(argv[param], "--datasection"))
        {
            param++;
            dataSection = argv[param];
        }
        else if (!strcmp(argv[param], "--rodatasection"))
        {
            param++;
            roDataSection = argv[param];
        }
        else if (!strcmp(argv[param], "--tablessection"))
        {
            param++;
            tablesSection = argv[param];
        }
        else if (!strcmp(argv[param], "--blank"))
        {
            param++;
            blank = atoi(argv[param]);
        }
        else if (!strcmp(argv[param], "--item"))
        {
            char *tmp;
            char *name;
            int id;
            unsigned char frame;

            param++;
            // Items are represented in a comma separated list
            // of <itemname>,<itemID>
            name = strtok(argv[param], ",");
            id = atoi(strtok(NULL, ","));

            if ((tmp = strtok(NULL, ",")) != NULL)
            {
                frame = atoi(tmp);
            }
            else
            {
                frame = 0;
            }

            addItem(name, id, frame);
        }
    }

    if (!mapWidth || !mapHeight || !levelWidth || !levelHeight || !fileName)
    {
        fprintf(stderr, "Invalid parameter!\n");
        exit(-1);
    }

    if (compressWidth == -1)
    {
        compressWidth = levelWidth;
    }
    if (compressHeight == -1)
    {
        compressHeight = levelHeight;
    }

    if ((inFile = fopen(fileName, "r+b")) == NULL)
    {
        fprintf(stderr, "Could not open in file\n");
        exit(-1);
    }

    ext = strrchr(fileName, '.');
    *ext++ = 0;

    if (!(outputFileName = malloc(strlen(fileName) + 128)))
    {
        fprintf(stderr, "Error allocating memory for output filename\n");
        fclose(inFile);
        exit(-1);
    }

    inputSize = levelHeight * levelWidth;
    if (!(inputData = malloc(inputSize)))
    {
        free(outputFileName);
        fclose(inFile);
        fprintf(stderr, "Error allocating input buffer\n");
        exit(-1);
    }

    compressSize = compressHeight * compressWidth;
    if (!(compressData = malloc(compressSize)))
    {
        free(outputFileName);
        free(inputData);
        fclose(inFile);
        fprintf(stderr, "Error allocating compressed data buffer\n");
        exit(-1);
    }

    sprintf(outputFileName, "%s.inc", fileName);
    if ((cFile = fopen(outputFileName, "w")) == NULL)
    {
        fprintf(stderr, "Could not open in file\n");
        free(outputFileName);
        free(inputData);
        free(compressData);
        exit(-1);
    }

    fprintf(cFile, ";\n");
    fprintf(cFile, "; Auto-generated by splitmap do not modify.\n");
    fprintf(cFile, ";\n");

    if (roDataSection)
    {
        fprintf(cFile, "        section %s", roDataSection);
        fprintf(cFile, "\n");
        fprintf(cFile, "eot:\n");
        fprintf(cFile, "        db      $ff\n\n");
    }

    if (tablesSection)
    {
        fprintf(cFile, "        section %s", tablesSection);
        fprintf(cFile, "\n");
    }

    if (horizontalStrip)
    {
        // Horizontal strip mode: output strip-based tables
        int numStrips = mapHeight / levelHeight;
        int levelsPerStrip = mapWidth / levelWidth;

        for (int n = 0; items[n].tableName != NULL; n++)
        {
            fprintf(cFile, "        public  _%sTables\n\n", items[n].tableName);
            fprintf(cFile, "_%sTables:\n", items[n].tableName);
            for (int strip = 0; strip < numStrips; strip++)
            {
                fprintf(cFile, "        dw      strip%d%s\n", strip, items[n].tableName);
            }
            fprintf(cFile, "\n");
        }
    }
    else
    {
        // Original per-level mode
        for (int n = 0; items[n].tableName != NULL; n++)
        {

            fprintf(cFile, "        public  _%sTables\n\n", items[n].tableName);
            fprintf(cFile, "_%sTables:\n", items[n].tableName);
            for (int levels = 0; levels < (mapHeight / levelHeight) * (mapWidth / levelWidth); levels++)
            {
                fprintf(cFile, "        dw      level%d%s\n", levels, items[n].tableName);
            }

            fprintf(cFile, "\n");
        }
    }

    if (dataSection)
    {
        fprintf(cFile, "        section %s", dataSection);
        fprintf(cFile, "\n");
    }

    if (horizontalStrip)
    {
        // Horizontal strip mode: process items per strip with world coordinates
        // Single pass: read strip, extract items, blank, write back
        int numStrips = mapHeight / levelHeight;

        // Allocate buffer for entire strip
        unsigned char *stripData = malloc(mapWidth * levelHeight);
        if (!stripData)
        {
            fprintf(stderr, "Error allocating strip buffer\n");
            free(outputFileName);
            free(inputData);
            free(compressData);
            fclose(inFile);
            exit(-1);
        }

        for (int strip = 0; strip < numStrips; strip++)
        {
            // Read entire strip row by row
            for (int row = 0; row < levelHeight; row++)
            {
                fseek(inFile, (strip * levelHeight * mapWidth) + (row * mapWidth), SEEK_SET);
                if ((fread(&stripData[row * mapWidth], mapWidth, 1, inFile)) != 1)
                {
                    fprintf(stderr, "Error reading strip %d row %d\n", strip, row);
                    free(stripData);
                    free(outputFileName);
                    free(inputData);
                    free(compressData);
                    fclose(inFile);
                    exit(-1);
                }
            }

            // Output items for this strip with world coordinates
            for (int n = 0; items[n].tableName != NULL; n++)
            {
                bool oneShot = FALSE;

                for (item_data_t *data = items[n].data; data != NULL; data = data->next)
                {
                    // Scan entire strip for items
                    for (int map = 0; map < levelHeight * mapWidth; map++)
                    {
                        if (stripData[map] == data->id)
                        {
                            // Calculate world X coordinate (16-bit: 0 to strip width in pixels)
                            int worldX = (map % mapWidth) * 8;
                            int worldY = (map / mapWidth) * 8;

                            // Blank item from strip data
                            stripData[map] = blank;

                            if (!oneShot)
                            {
                                oneShot = TRUE;
                                fprintf(cFile, "strip%d%s:\n", strip, items[n].tableName);
                            }
                            // Output: flags, worldX_low, worldX_high, Y, frame
                            fprintf(cFile, "        db      $01, $%02x, $%02x, $%02x, $%02x\n",
                                    worldX & 0xFF, (worldX >> 8) & 0xFF, worldY, data->frame);
                            items[n].count++;
                        }
                    }
                }
                if (!oneShot)
                {
                    fprintf(cFile, "strip%d%s equ eot\n\n", strip, items[n].tableName);
                }
                else
                    fprintf(cFile, "        db      $ff\n\n");
            }

            // Write back blanked strip
            for (int row = 0; row < levelHeight; row++)
            {
                fseek(inFile, (strip * levelHeight * mapWidth) + (row * mapWidth), SEEK_SET);
                if ((fwrite(&stripData[row * mapWidth], mapWidth, 1, inFile)) != 1)
                {
                    fprintf(stderr, "Error writing blanked strip %d\n", strip);
                    free(stripData);
                    free(outputFileName);
                    free(inputData);
                    free(compressData);
                    fclose(inFile);
                    exit(-1);
                }
            }
        }
        free(stripData);
    }
    else
    {
        // Original per-level mode
        for (int y = 0; y < (mapHeight / levelHeight); y++)
        {
            for (int x = 0; x < (mapWidth / levelWidth); x++)
            {
                for (int row = 0; row < levelHeight; row++)
                {
                    fseek(inFile, (y * levelHeight * mapWidth) + (x * levelWidth) + (row * mapWidth), SEEK_SET);

                    if ((fread(&inputData[row * levelWidth], levelWidth, 1, inFile)) != 1)
                    {
                        fprintf(stderr, "Error reading input file\n");
                        free(outputFileName);
                        free(inputData);
                        free(compressData);
                        fclose(inFile);
                        exit(-1);
                    }
                }

                for (int n = 0; items[n].tableName != NULL; n++)
                {
                    bool oneShot = FALSE;

                    for (item_data_t *data = items[n].data; data != NULL; data = data->next)
                    {
                        // Find items
                        for (int map = 0; map < levelHeight * levelWidth; map++)
                        {
                            if (inputData[map] == data->id)
                            {
                                // Items in tables don't need to be in
                                // the map, blank them out.
                                inputData[map] = blank;
                                if (!oneShot)
                                {
                                    oneShot = TRUE;
                                    fprintf(cFile, "level%d%s:\n", (y * (mapWidth / levelWidth)) + x, items[n].tableName);
                                }
                                fprintf(cFile, "        db      $01, $%02x, $%02x, $%02x\n", (map % levelWidth) * 8,
                                        (map / levelWidth) * 8, data->frame);
                                items[n].count++;
                            }
                        }
                    }
                    if (!oneShot)
                    {
                        fprintf(cFile, "level%d%s equ eot\n\n", (y * (mapWidth / levelWidth)) + x, items[n].tableName);
                    }
                    else
                        fprintf(cFile, "        db      $ff\n\n");
                }

                // Write out updated map
                for (int row = 0; row < levelHeight; row++)
                {
                    fseek(inFile, (y * levelHeight * mapWidth) + (x * levelWidth) + (row * mapWidth), SEEK_SET);

                    if ((fwrite(&inputData[row * levelWidth], levelWidth, 1, inFile)) != 1)
                    {
                        fprintf(stderr, "Error writing updated map\n");
                        free(outputFileName);
                        free(inputData);
                        free(compressData);
                        fclose(inFile);
                        exit(-1);
                    }
                }
            }
        }
    }

    // See if compressed maps need to be written
    if (itemsOnly == 0)
    {
        rewind(inFile);

        if (horizontalStrip)
        {
            // Horizontal strip mode: output entire rows as strips
            int numStrips = mapHeight / levelHeight;
            int stripSize = mapWidth * levelHeight;
            unsigned char *stripBuffer = malloc(stripSize);

            if (!stripBuffer)
            {
                fprintf(stderr, "Error allocating strip buffer for compression\n");
                free(outputFileName);
                free(inputData);
                free(compressData);
                fclose(inFile);
                exit(-1);
            }

            for (int strip = 0; strip < numStrips; strip++)
            {
                sprintf(outputFileName, "%s_strip%d.%s", fileName, strip, ext);

                // Read entire strip row by row
                for (int row = 0; row < levelHeight; row++)
                {
                    fseek(inFile, (strip * levelHeight * mapWidth) + (row * mapWidth), SEEK_SET);

                    if ((fread(&stripBuffer[row * mapWidth], mapWidth, 1, inFile)) != 1)
                    {
                        fprintf(stderr, "Error reading strip %d row %d\n", strip, row);
                        free(stripBuffer);
                        free(outputFileName);
                        free(inputData);
                        free(compressData);
                        fclose(inFile);
                        exit(-1);
                    }
                }
                fprintf(stderr, "%s ", outputFileName);
                if (noCompress)
                    writeRaw(outputFileName, stripBuffer, stripSize);
                else
                    doCompression(outputFileName, stripBuffer, stripSize);
            }
            free(stripBuffer);
        }
        else
        {
            // Original per-level mode
            for (int y = 0; y < (mapHeight / compressHeight); y++)
            {
                for (int x = 0; x < (mapWidth / compressWidth); x++)
                {
                    sprintf(outputFileName, "%s_%02d%02d.%s", fileName, x, y, ext);

                    for (int row = 0; row < compressHeight; row++)
                    {
                        fseek(inFile, (y * compressHeight * mapWidth) + (x * compressWidth) + (row * mapWidth), SEEK_SET);

                        if ((fread(&compressData[row * compressWidth], compressWidth, 1, inFile)) != 1)
                        {
                            fprintf(stderr, "Error reading input file\n");
                            free(outputFileName);
                            free(inputData);
                            free(compressData);
                            fclose(inFile);
                            exit(-1);
                        }
                    }
                    fprintf(stderr, "%s ", outputFileName);
                    if (noCompress)
                        writeRaw(outputFileName, compressData, compressSize);
                    else
                        doCompression(outputFileName, compressData, compressSize);
                }
            }
        }
    }
    fclose(inFile);
    free(outputFileName);
    free(inputData);
    free(compressData);

    for (int n = 0; items[n].tableName != NULL; n++)
    {
        if (items[n].tableName)
        {
            printf("\t#define\t%s_COUNT %d\n", items[n].tableName, items[n].count);
        }
    }

    //
    // Write out the level/strip table and the compressed tilemap data table
    //
    if (itemsOnly == 0)
    {
        fprintf(cFile, "\n");
        if (roDataSection)
        {
            fprintf(cFile, "        section  %s\n", roDataSection);
            fprintf(cFile, "\n");
        }

        char *fname = basename(fileName);

        if (horizontalStrip)
        {
            // Horizontal strip mode: output strip table
            int numStrips = mapHeight / levelHeight;

            fprintf(cFile, "        public  _stripTable\n");
            fprintf(cFile, "\n");
            fprintf(cFile, "_stripTable:\n");

            // Generate 4-byte entries using dq directive
            // The assembler automatically encodes the full address including bank number
            for (int strip = 0; strip < numStrips; strip++)
            {
                fprintf(cFile, "        dq      %s_strip%d\n", fname, strip);
            }

            fprintf(cFile, "\n");

            // Also output _levelTable for backwards compatibility (pointing to strip 0 start)
            fprintf(cFile, "        public  _levelTable\n");
            fprintf(cFile, "_levelTable equ _stripTable\n");
            fprintf(cFile, "\n");

            // Output strip width constant
            fprintf(cFile, "        public  _stripWidthTiles\n");
            fprintf(cFile, "_stripWidthTiles equ %d\n", mapWidth);
            fprintf(cFile, "\n");

            // Output strips uncompressed for direct ROM access
            // Distribution:
            // Bank 2: strips 0-3 (4 strips = 16128 bytes)
            // Bank 7: strip 4 (1 strip = 4032 bytes)
            for (int strip = 0; strip < numStrips; strip++)
            {
                int bankNum;
                if (strip < 4) bankNum = 2;
                else bankNum = 7;  // Strip 4 in bank 7
                
                fprintf(cFile, "        section  RODATA_%d\n", bankNum);
                fprintf(cFile, "%s_strip%d:\n", fname, strip);
                // Always output uncompressed for direct ROM access
                fprintf(cFile, "        binary  \"%s_strip%d.nxm\"\n", fname, strip);
                fprintf(cFile, "\n");
            }
        }
        else
        {
            // Original per-level mode
            fprintf(cFile, "        public  _levelTable\n");
            fprintf(cFile, "\n");
            fprintf(cFile, "_levelTable:\n");

            for (int y = 0; y < (mapHeight / compressHeight); y++)
            {
                for (int x = 0; x < (mapWidth / compressWidth); x++)
                {
                    fprintf(cFile, "        dw      %s_%02d%02d\n", fname, x, y);
                }
            }

            fprintf(cFile, "\n");

            for (int y = 0; y < (mapHeight / compressHeight); y++)
            {
                for (int x = 0; x < (mapWidth / compressWidth); x++)
                {
                    fprintf(cFile, "%s_%02d%02d:\n", fname, x, y);
                    if (noCompress)
                        fprintf(cFile, "        binary  \"%s_%02d%02d.nxm\"\n", fname, x, y);
                    else
                        fprintf(cFile, "        binary  \"%s_%02d%02d.nxm.zx0\"\n", fname, x, y);
                }
            }
        }
    }
    fclose(cFile);

    // Free allocated memory
    for (int n = 0; items[n].tableName != NULL; n++)
    {
        while (items[n].data)
        {
            item_data_t *tmp = items[n].data;
            items[n].data = items[n].data->next;
            free(tmp);
        }
    }
    return 0;
}
