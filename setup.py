from skbuild import setup

setup(
    name="teahypercube",
    version="0.1",
    description="hypercube test and hypercube generator",
    author='Egor A. Tsvetkov aka tea',
    license="MIT",
    packages=["teahypercube"],
    package_dir = {'': 'src/python'},
    cmake_install_dir = "src/python/teahypercube",
    python_requires=">=3.7",
)

