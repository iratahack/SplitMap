#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zx0.h"

#define MAX_OFFSET_ZX0    32640
#define MAX_OFFSET_ZX7     2176

#define MAX_ITEMS   256

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

item_t* addItem(char *name, int id, unsigned char frame)
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

    output_name = (char*) malloc(strlen(fileName) + 5);
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
    char *dataSection = NULL;
    char *roDataSection = NULL;
    unsigned char *inputData = NULL;
    FILE *inFile = NULL;
    FILE *cFile = NULL;
    int mapWidth = 0;
    int mapHeight = 0;
    int levelWidth = 0;
    int levelHeight = 0;
    int itemsOnly = 0;
    int inputSize;

    memset(items, 0, sizeof(items));

    if (argc < 2)
    {
        printf(
                "%s --map <filename> --map-size <WIDTHxHEIGHT> --level-size <WIDTHxHEIGHT> [--[ro]datasection <section name>] [--items-only] [--item <name>,<ID>[,<FRAME>] [...]]\n",
                argv[0]);
        return 0;
    }

    for (int param = 1; param < argc; param++)
    {
        if (!strcmp(argv[param], "--items-only"))
        {
            itemsOnly = 1;
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

    if ((inFile = fopen(fileName, "rb")) == NULL)
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

    sprintf(outputFileName, "%s.inc", fileName);
    if ((cFile = fopen(outputFileName, "w")) == NULL)
    {
        fprintf(stderr, "Could not open in file\n");
        free(outputFileName);
        exit(-1);
    }

    fprintf(cFile, ";\n");
    fprintf(cFile, "; Auto-generated by splitmap do not modify.\n");
    fprintf(cFile, ";\n");

    if (roDataSection)
    {
        fprintf(cFile, "        section %s", roDataSection);
        fprintf(cFile, "\n");
    }

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

    if (dataSection)
    {
        fprintf(cFile, "        section %s", dataSection);
        fprintf(cFile, "\n");
    }

    for (int y = 0; y < (mapHeight / levelHeight); y++)
    {
        for (int x = 0; x < (mapWidth / levelWidth); x++)
        {
            sprintf(outputFileName, "%s_%02d%02d.%s", fileName, x, y, ext);

            for (int row = 0; row < levelHeight; row++)
            {
                fseek(inFile, (y * levelHeight * mapWidth) + (x * levelWidth) + (row * mapWidth), SEEK_SET);

                if ((fread(&inputData[row * levelWidth], levelWidth, 1, inFile)) != 1)
                {
                    fprintf(stderr, "Error reading input file\n");
                    free(outputFileName);
                    free(inputData);
                    fclose(inFile);
                    exit(-1);
                }
            }

            if (itemsOnly == 0)
            {
                fprintf(stderr, "%s ", outputFileName);
                doCompression(outputFileName, inputData, inputSize);
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
        }
    }
    fclose(inFile);
    free(inputData);

    for (int n = 0; items[n].tableName != NULL; n++)
    {
        if (items[n].tableName)
        {
            printf("\t#define\t%s_COUNT %d\n", items[n].tableName, items[n].count);
        }
    }

    //
    // Write out the level table and the compressed tilemap data table
    //
    if (itemsOnly == 0)
    {
        fprintf(cFile, "\n");
        if (roDataSection)
        {
            fprintf(cFile, "        section  %s\n", roDataSection);
            fprintf(cFile, "\n");
        }
        fprintf(cFile, "eot:\n");
        fprintf(cFile, "        db      $ff\n\n");

        fprintf(cFile, "        public  _levelTable\n");
        fprintf(cFile, "\n");
        fprintf(cFile, "_levelTable:\n");

        for (int y = 0; y < (mapHeight / levelHeight); y++)
        {
            for (int x = 0; x < (mapWidth / levelWidth); x++)
            {
                fprintf(cFile, "        dw      %s_%02d%02d\n", fileName, x, y);
            }
        }

        fprintf(cFile, "\n");

        for (int y = 0; y < (mapHeight / levelHeight); y++)
        {
            for (int x = 0; x < (mapWidth / levelWidth); x++)
            {
                fprintf(cFile, "%s_%02d%02d:\n", fileName, x, y);
                fprintf(cFile, "        binary  \"%s_%02d%02d.nxm.zx0\"\n", fileName, x, y);
            }
        }
    }
    fclose(cFile);
    free(outputFileName);

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
