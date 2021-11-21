# Introduction

The cli application helps you to use the voxeloptimizer more efficiently in your workflow. At the moment there was only the V-Optimizer, which had the ability to optimize and display the model, but that was very uncomfortable for several meshes at the same time, as you could only load one voxel mesh at a time. Now there is a simpler solution, the cli.

# Commands

| Command   | Description    |
|--------------- | --------------- |
| -h, --help   | Show the help dialog  |
| -m, --mesher | Sets the mesher to meshify the voxel mesh. Default: simple. (simple, greedy) |
| -o, --output | Output path. If the output path doesn't exist it will be created |
| -w, --worldspace | Transforms all vertices to worldspace |

# Usage

Input files are just a list of files. This can contains placeholders like `*.vox`. This will convert all MagicaVoxel files.

**Note:** On error there will be returned -1 and a message is logged to the stderr stream.

`./cli [INPUT] [OPTIONS]`

## Examples:

> Converts the *.vox file to a *.glb

`./cli windmill.vox -o windmill.glb`

> Converts all *.vox files to *.glb with the same name as the *.vox files

`./cli voxels/*.vox -o *.glb`

> Converts all *.vox files to a *.glb with the names Mesh0.glb Mesh1.glb ...

`./cli voxels/*.vox -o output/Mesh{0}.glb`

> Converts all supported file formats inside a folder to *.glb files

`./cli voxels/ -o *.glb`

> Converts all supported file formats to *.glb files

`./cli *.* -o *.glb`