========================================================================
    DYNAMIC LINK LIBRARY : DxfReader Project Overview
========================================================================

Author: Deping Chen
Date: 2017/10/15 - 2017/11/15
Email: cdp97531@sina.com

This project is a C++11 project, building using VS2015.
Because it uses very few platform-specific API, it should be very easy to port to other platform.

DXF lib features:
1. DXF file to be read should be encoded in UTF-8. AutoCAD 2007 and later verson will write DXF in UTF-8.
2. Written DXF is AutoCAD 2010 format.
3. Different instances of DxfData, DxfReader, DxfWriter can be used in different threads. The same instance of DxfData, DxfReader, DxfWriter can't be used in multiple threads.
4. Each thread can has its debug log file. This file can tell you why DXF reading or writing fails. You can call function SetThreadDebugInfo to set this file.
5. You must provide line type file(*.lin) and pattern file(*.pat) to write a DXF file.
6. You can manually fill a DxfData instance and write it to a DXF file. DxfWriter will automatically add layer "0", text style "standard", dim style "ISO-25", etc. If DxfData has no layout, DxfWriter will automatically add a default layout.
7. AutoCAD 2010 DXF must provide block definition for each dimension, or dimension won't be shown. It is a hard task to generate accurate block definitions for dimensions, because a dimension can have many parameters. For now the workaround can be done by 3 steps:
    a. Select all entities by pressing ctrl+A.
    b. Move all entities by 0 offset(Press "m", press "Enter" 3 times). All dimensions will be visible.
8. You can disable debug info by calling EnableDebugInfo(false);