# PyVFrame

Python module to extract frames from a video file.

# API

Package provides a simple API to get video frames from a video file. The frames are returned as [buffer view](https://docs.python.org/3/c-api/buffer.html) objects which can be converted to numpy arrays.

```python
import pyvframe
import numpy as np

buffer_view = pyvframe.extract_video_frames_from_video_at_url('path/to/video.mp4')
frames = np.asarray(buffer_view)
```

# C++ Build

## Requirements

- vcpkg

You also need to install python requierments.
`pip install -r requierments.txt`

## VCPkg Setup

VCPkg is installed via a git repository which a VCPKG_ROOT environment variable points to.
The VCPKG_ROOT can be whatever location on your machine you want.

```
git clone https://github.com/microsoft/vcpkg.git

cd vcpkg
export VCPKG_ROOT=`pwd`

./bootstrap-vcpkg.sh

echo "export VCPKG_ROOT=$VCPKG_ROOT" >> ~/.zshrc # or path to your shell config file 
echo "export PATH=$PATH:$VCPKG_ROOT" >> ~/.zshrc # or path to your shell config file 
```

## Build

Debug and Release modes are supported. From project root run:
```
python scripts/build.py debug
python scripts/build.py release
```

After the build you can find the python package in `./build/{mode}` or the latest one is always available in `./install/` which is used to run the tests.

## Test

For testing we use `pytest`. To run all tests simply call from project root.
```
pytest
```
