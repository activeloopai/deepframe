import re
import sys
import os
import urllib.request

from setuptools import setup, find_packages
from setuptools.dist import Distribution

project_name = "deepframe"


class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True


init_file = os.path.join("deepframe", "__init__.py")


def get_property(prop):
    result = re.search(
        rf'{prop}\s*=\s*[\'"]([^\'"]*)[\'"]',
        open(init_file).read(),
    )
    return result.group(1)


install_requires = [
    "numpy",
]

long_description = urllib.request.urlopen("https://raw.githubusercontent.com/activeloopai/deepframe/refs/heads/main/README.md").read().decode("utf-8")

config = {
    "name": project_name,
    "version": get_property("__version__"),           #
    "long_description": long_description,             #
    "long_description_content_type": "text/markdown", #
    "author": "activeloop.ai",
    "author_email": "support@activeloop.ai",
    "packages": find_packages(exclude=["tests"]),
    "distclass": BinaryDistribution,
    "install_requires": install_requires,
    "python_requires": ">=3.9",
    "classifiers": [
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: Python :: 3.14",
        "Programming Language :: Python :: 3.15",
        "License :: OSI Approved :: Apache Software License",
    ],
    "include_package_data": True,
    "zip_safe": False,
    "project_urls": {
        "Source": "https://github.com/activeloopai/deepframe",
    },
}

setup(**config)
