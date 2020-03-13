Artekmed Application Framework for Development and Research
===========================================================

Tool to export frames from an MKV file recorded using the Kinect for Azure SDK

Building:
 - ```conan install .. -s build_type=Debug|Release --build "*"```
 - ```cmake .. -DCMAKE_BUILD_TYPE=Debug```
 - ```make -j 12```
 
 Usage:
 - ```source activate_run.sh```
 - ```./bin/export_mkv_k4a --help```
 
 ```
Usage:
  ./bin/extract_mkv_k4a [--magnum-...] [-h|--help] [--output-dir DIR] [--first-frame FIRST_FRAME] [--last-frame LAST_FRAME] [--color] [--depth] [--infrared] [--pointcloud] [--] input

Arguments:
  input                      Input file
  -h, --help                 display this help message and exit
  --output-dir DIR           Output directory
                             (default: ../test_data)
  --first-frame FIRST_FRAME  First frame to export
                             (default: 0)
  --last-frame LAST_FRAME    Last frame to export
                             (default: 0)
  --color                    Export Color stream
  --depth                    Export Depth stream
  --infrared                 Export Infrared stream
  --pointcloud               Export Pointcloud stream
  --magnum-...               engine-specific options
                             (see --magnum-help for details)

```