from setuptools import setup, find_packages

setup(
    name="pyvframe",
    version="0.1.0",
    packages=find_packages(),
    include_package_data=True,  # Includes non-Python files like `.so`
    package_data={
        "pyvframe": ["*.so"],  # Ensure the shared object file is included
    },
    install_requires=["numpy"],
)