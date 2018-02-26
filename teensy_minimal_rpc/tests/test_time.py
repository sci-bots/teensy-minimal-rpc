from __future__ import absolute_import
import nose.tools as nt
import teensy_minimal_rpc as tr


def setup_func():
    global proxy
    proxy = tr.SerialProxy()


def teardown_func():
    global proxy
    del proxy


@nt.with_setup(setup_func, teardown_func)
def test_milliseconds():
    '''
    Test reading millisecond counter from device.
    '''
    time_a = proxy.milliseconds()
    time_b = proxy.milliseconds()
    nt.ok_(time_a < time_b)


@nt.with_setup(setup_func, teardown_func)
def test_microseconds():
    '''
    Test reading microsecond counter from device.
    '''
    time_a = proxy.microseconds()
    time_b = proxy.microseconds()
    nt.ok_(time_a < time_b)
