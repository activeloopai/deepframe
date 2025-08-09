# PyVFrame

Python module to extract frames from a video file.

## API

Package provides a simple API to get video frames from a video file. The frames are returned as [buffer view](https://docs.python.org/3/c-api/buffer.html) objects which can be converted to numpy arrays.

```python
import deepframe
import numpy as np

# Extract whole video as a buffer view
buffer_view = deepframe.extract_frames('path/to/video.mp4')
frames = np.asarray(buffer_view)
```

```python
import deepframe
import numpy as np
# Extract first 100 frames from a video file
buffer_view = deepframe.extract_frames('path/to/video.mp4', slice(0, 100))
frames = np.asarray(buffer_view)
```

```python
import deepframe
import numpy as np
# Extract each 10th frame from a video file
buffer_view = deepframe.extract_frames('path/to/video.mp4', slice(0, None, 10))
frames = np.asarray(buffer_view)
```

## C++ Build

### Requirements

- vcpkg

You also need to install python requirements.
`pip install -r requirements-dev.txt`

## VCPkg Setup

VCPkg is installed via a git repository which a VCPKG_ROOT environment variable points to.
The VCPKG_ROOT can be whatever location on your machine you want.

``` bash
git clone https://github.com/microsoft/vcpkg.git

cd vcpkg
export VCPKG_ROOT=`pwd`

./bootstrap-vcpkg.sh

echo "export VCPKG_ROOT=$VCPKG_ROOT" >> ~/.zshrc # or path to your shell config file
echo "export PATH=$PATH:$VCPKG_ROOT" >> ~/.zshrc # or path to your shell config file
```

## Build

Debug and Release modes are supported. From project root run:

``` bash
python scripts/build.py debug
python scripts/build.py release
```

After the build you can find the python package in `./build/{mode}` or the latest one is always available in `./deepframe/` which is used to run the tests.

## Test

For testing we use `pytest`. To run all tests simply call from project root.

``` bash
pytest
```

## Build wheel

From project root call

``` bash
python -m build
```

Once the run finishes you will find the files under `./dist/` directory.
