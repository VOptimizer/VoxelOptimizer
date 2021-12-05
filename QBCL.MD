# Qubicle's proprietary format QBCL v2.0

## Header

| Name | Type | Size in bytes   | Description   |
|-------------- | -------------- | -------------- | -------------- |
| magic    | string | 4     | Magic sum of the file(always QBCL)     |
| programVersion | uint | 4 | Version of Qubicle that wrote this file (major, minor, release, build) |
| fileVersion | uint | 4 | Version of the file format(currently 2.0) |

## Thumbnail

| Name | Type | Size in bytes   | Description   |
|-------------- | -------------- | -------------- | -------------- |
| width | uint | 4 | Width of the thumbnail |
| height | uint | 4 | Height of the thumbnail |
| thumbnail | array | width * height * 4 | Uncompressed BGRA pixel values |

## Metadata

| Name | Type | Size in bytes   | Description   |
|-------------- | -------------- | -------------- | -------------- |
| valueSize | uint | 4 | Size of the metadata value (0 means not available ) |
| metadataValue | string | valueSize | Metadata value |

Order of the metadata value is like in the GUI of Qubicle.

1. Title
2. Description
3. Tags
4. Author
5. Company
6. Website
7. Copyright

## GUID or timestamp?

16 bytes. I don't know exactly what this value means.

## Model tree

The tree starts always with a model node.

### Model

| Name | Type | Size in bytes   | Description   |
|-------------- | -------------- | -------------- | -------------- |
| type | int | 4 | Type of the node (1 for model) |
| unknown | int | 4 | It is presented in all nodes. I don't know exactly what this value means.  |
| nameLen | uint | 4 | Size of the name of this node. |
| name | string | nameLen | Name of this node |
| unknown | bytes | 3 | It is presented in all nodes. I don't know exactly what this value means. |
| unknown | bytes | 36 | I don't know exactly what this value means. (Always the same) |
| childCount | uint | 4 | The count of the child nodes. |

## Matrix

| Name | Type | Size in bytes   | Description   |
|-------------- | -------------- | -------------- | -------------- |
| type | int | 4 | Type of the node (1 for model) |
| unknown | int | 4 | It is presented in all nodes. I don't know exactly what this value means.  |
| nameLen | uint | 4 | Size of the name of this node. |
| name | string | nameLen | Name of this node |
| unknown | bytes | 3 | It is presented in all nodes. I don't know exactly what this value means. |
| size | veci3 | 12 | The size of the model |
| position | veci3 | 12 | The position in 3D space |
| pivot | vecf3 | 12 | The pivot position |
| compressedDataSize | uint | 4 | The size of the following compressed data |
| compressedData | bytes | compressedDataSize | Zlib compressed voxel data |

### Zlib data

The content of the uncompressed Zlib data is compressed with an RLE (runtime length encoding).
The voxel data goes from bottom to top, left to right, and front to back. (Starts by (0, 0, 0) and ends by size)

The structure of the rle data is as follows: 
| Name | Type | Size in bytes   | Description   |
|-------------- | -------------- | -------------- | -------------- |
| dataNum | ushort | 2 | Number of integers that are either RGBM values or length values for RLE |
| data | array of int | dataNum | Voxel data |

Difference between RGBM and RLE data. If the M (or alpha) byte is 2, the red channel contains the number of how many times the next 4 bytes(RGBM value, always 0x00000000) should be repeated. The G and B value are some sort of metadata. If the M byte is not 2, this is an RGB value. The M byte is the mask byte.

## Compounds

Not found

## Parsing

Pseudo code

```
function loadQBCL(stream)
{
    magic = stream.readString(4);
    if(magic != "QBCL")
        return ERROR

    programVersion = stream.readInt
    fileVersion = stream.readInt

    if(fileVersion != 2)
        return ERROR

    width = stream.readUInt
    height = stream.readUInt
    stream.skip(width * height * 4) // Or read the image

    for(i = 0; i < 7; i++)
    {
        size = stream.readUInt
        stream.skip(size) // Or save the meta value
    }

    stream.skip(16) // GUID or timestamp

    return loadNode(stream)
}

function loadNode(stream)
{
    type = stream.readInt
    stream.skip(4)  // Unknown value

    nameLen = stream.readUInt
    Skip(nameLen) // Or save node name
    Skip(3) // Unknown 3 bytes

    switch(type)
    {
        case 0: //Matrix
            loadMatrix(stream)
        break

        case 1: //Model
            loadModel(stream)
        break

        //case 2: //Compound like in QBT

        //break

        default
            return ERROR
        break
    }

    return OK
}

function loadModel(stream)
{
    childCount = stream.readUInt
    for(i = 0; i < childCount; i++)
        loadNode(stream)
}

function loadMatrix(stream)
{
    size = stream.readVeci3
    position = stream.readVeci3
    pivot = stream.readVecf3

    compressedDataSize = stream.readUInt
    zlibStream = new zlibStream(stream, compressedDataSize)

    index = 0

    while(!zlibStream.isEOF)
    {
        y = 0
        dataSize = zlibStream.readUShort

        for(i = 0; i < dataSize; i++)
        {
            data = zlibStream.readColor

            if(data.A == 2) //RLE
            {
                color = zlibStream.readColor
                for(j = 0; j < data.R; j++)
                {
                    x = (index / size.z);
                    z = index % size.z;

                    voxelGrid[x, y, z] = color.toUInt;
                    y++
                }

                i++
            }
            else    // Uncompressed
            {
                x = (index / size.z);
                z = index % size.z;

                voxelGrid[x, y, z] = data.toUInt;
                y++
            }
        }

        index++
    }
}
```