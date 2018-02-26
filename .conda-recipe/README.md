Conda recipe to build `teensy-minimal-rpc` package.

Build
=====

Install `conda-build`:

    conda install conda-build

Build recipe:

    conda build . -c sci-bots -m variants.yaml


Install
=======

The pre-built package may be installed from the [`sci-bots`][2] channel using:

    conda install -c sci-bots teensy-minimal-rpc


[1]: https://anaconda.org/sci-bots/teensy-minimal-rpc
[2]: https://anaconda.org/sci-bots
