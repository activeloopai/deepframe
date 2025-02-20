Python module to extract frames from a video file.

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