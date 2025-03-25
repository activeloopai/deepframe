import re
import sys
import os
from setuptools import setup, find_packages
from setuptools.dist import Distribution

project_name = "pyvframe"

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

init_file = os.path.join("pyvframe", "__init__.py")

def get_property(prop):
    result = re.search(
        rf'{prop}\s*=\s*[\'"]([^\'"]*)[\'"]',
        open(init_file).read(),
    )
    return result.group(1)

# Define numpy version based on Python version
numpy_version = "numpy>=1.26.0,<2.0.0" if sys.version_info >= (3, 12) else "numpy>=1.24.0,<2.0.0"

install_requires = [
    numpy_version,
]

config = {
    "name": project_name,
    "version": get_property("__version__"),
    "author": "activeloop.ai",
    "author_email": "support@activeloop.ai",
    "packages": find_packages(exclude=["tests"]),
    "distclass": BinaryDistribution,
    "install_requires"=install_requires,
    "include_package_data": True,
    "zip_safe": False,
}

setup(**config)