from setuptools import setup, find_packages
from setuptools.dist import Distribution

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(
    name="pyvframe",
    version="0.1.0",
    packages=find_packages(exclude=["tests"]),
    include_package_data=True,  # Includes non-Python files like `.so`
    install_requires=["numpy"],
    distclass=BinaryDistribution,
    zip_safe=False,
)