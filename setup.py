from skbuild import setup  
setup(
    name="teahypercube",
    version="0.1",
    description="hypercube test and hypercube generator",
    author='Egor A. Tsvetkov aka tea',
    license="MIT",
    package_dir = {'': 'src/python'},
    packages=['teahypercube'],
    python_requires=">=3.7",
)

