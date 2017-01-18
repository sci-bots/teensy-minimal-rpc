from collections import OrderedDict

from path_helpers import path
try:
    from .config import Config, State
except (ImportError, TypeError):
    pass
from .proxy import Proxy, I2cProxy, SerialProxy


def conda_prefix():
    '''
    Returns
    -------
    path_helpers.path
        Path to Conda environment prefix corresponding to running Python
        executable.

        Return ``None`` if not running in a Conda environment.
    '''
    if any(['continuum analytics, inc.' in sys.version.lower(),
            'conda' in sys.version.lower()]):
        # Assume running under Conda.
        if 'CONDA_PREFIX' in os.environ:
            conda_prefix = ph.path(os.environ['CONDA_PREFIX'])
        else:
            # Infer Conda prefix as parent directory of Python executable.
            conda_prefix = ph.path(sys.executable).parent.realpath()
    else:
        # Assume running under Conda.
        conda_prefix = None
    return conda_prefix


def conda_arduino_include_path():
    if platform.system() in ('Linux', 'Darwin'):
        return conda_prefix().joinpath('include', 'Arduino')
    elif platform.system() == 'Windows':
        return conda_prefix().joinpath('Library', 'include', 'Arduino')
    raise 'Unsupported platform: %s' % platform.system()


def package_path():
    return path(__file__).parent


def get_sketch_directory():
    '''
    Return directory containing the Arduino sketch.
    '''
    return package_path().joinpath('Arduino', package_path().name)


def get_lib_directory():
    return package_path().joinpath('..', 'lib').realpath()


def get_includes():
    '''
    Return directories containing the Arduino header files.

    Notes
    =====

    For example:

        import arduino_rpc
        ...
        print ' '.join(['-I%s' % i for i in arduino_rpc.get_includes()])
        ...

    '''
    return list(conda_arduino_include_path().walkdirs('src'))


def get_sources():
    '''
    Return Arduino source file paths.  This includes any supplementary source
    files that are not contained in Arduino libraries.
    '''
    import base_node_rpc

    return (get_sketch_directory().files('*.c*') +
            list(get_lib_directory().walkfiles('*.c*')) +
            base_node_rpc.get_sources())


def get_firmwares():
    '''
    Return compiled Arduino hex file paths.

    This function may be used to locate firmware binaries that are available
    for flashing to [Arduino][1] boards.

    [1]: http://arduino.cc
    '''
    return OrderedDict([(board_dir.name, [f.abspath() for f in
                                          board_dir.walkfiles('*.hex')])
                        for board_dir in
                        package_path().joinpath('firmware').dirs()])
