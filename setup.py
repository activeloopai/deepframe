from setuptools import setup, find_packages
from setuptools.dist import Distribution
import sys

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

# Define numpy version based on Python version
numpy_version = "numpy>=1.26.0,<2.0.0" if sys.version_info >= (3, 12) else "numpy>=1.24.0,<2.0.0"

setup(
    name="pyvframe",
    version="0.1.0",
    packages=find_packages(exclude=["tests"]),
    include_package_data=True,  # Includes non-Python files like `.so`
    install_requires=[numpy_version],
    distclass=BinaryDistribution,
    zip_safe=False,
)