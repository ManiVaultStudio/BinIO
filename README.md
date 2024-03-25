# BinaryIO Plugin ![Build Status](https://github.com/ManiVaultStudio/BinIO/actions/workflows/build.yml/badge.svg?branch=master)

Binary data loader and writer plugins for the [ManiVault](https://github.com/ManiVaultStudio/core) visual analytics framework.


<p align="middle">
  <img src="https://github.com/ManiVaultStudio/BinIO/assets/58806453/29c68f78-ff34-44d6-8e1a-be791b40c948" align="middle" width="40%" />
  <img src="https://github.com/ManiVaultStudio/BinIO/assets/58806453/47d0a07e-0bbf-4aa3-8701-b62aac99d059" align="middle"  width="20%" /> </br>
  Binary loader and exporter UIs
</p>

The exporter creates a `.txt` file in a addition to a binary file with some meta data, e.g. `file.txt`:
```cpp
file.bin
Num dimensions: 42
Num data points: 238
Data type: float 
```
## How to use
- In Manivault, exporters are opened by right-clicking on a data set in the data hierarchy, selecting the "Export" field and further chosing the desired exporter (`BIN Exporter`).
- Either right-click an empty area in the data hierachy and select `Import` -> `BIN Loader` or in the main menu, open `File` -> `Import data...` -> `BIN Loader`
