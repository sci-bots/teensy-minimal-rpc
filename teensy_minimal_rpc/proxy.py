import datetime as dt
import logging
import uuid

from path_helpers import path
import numpy as np


logger = logging.getLogger(__name__)


try:
    from base_node_rpc.proxy import ConfigMixinBase, StateMixinBase
    import arduino_helpers.hardware.teensy as teensy

    from .adc_sampler import AdcDmaMixin
    from .node import (Proxy as _Proxy, I2cProxy as _I2cProxy,
                       SerialProxy as _SerialProxy)
    from .config import Config
    from .state import State


    HW_TCDS_ADDR = 0x40009000
    TCD_RECORD_DTYPE = [('SADDR', 'uint32'),
                        ('SOFF', 'uint16'),
                        ('ATTR', 'uint16'),
                        ('NBYTES', 'uint32'),
                        ('SLAST', 'uint32'),
                        ('DADDR', 'uint32'),
                        ('DOFF', 'uint16'),
                        ('CITER', 'uint16'),
                        ('DLASTSGA', 'uint32'),
                        ('CSR', 'uint16'),
                        ('BITER', 'uint16')]


    class ConfigMixin(ConfigMixinBase):
        @property
        def config_class(self):
            return Config

        @property
        def uuid(self):
            '''
            Returns
            -------

                (uuid.UUID) : UUID constructed from the [Unique Identification
                    Register][1] (12.2.19 page 265).


            [1]: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
            '''
            return uuid.UUID(bytes=np.array(self._uuid(),
                                            dtype='uint8').tostring())


    class StateMixin(StateMixinBase):
        @property
        def state_class(self):
            return State


    class ProxyMixin(ConfigMixin, StateMixin, AdcDmaMixin):
        '''
        Mixin class to add convenience wrappers around methods of the generated
        `node.Proxy` class.

        For example, expose config and state getters/setters as attributes.
        '''
        host_package_name = str(path(__file__).parent.name.replace('_', '-'))

        def __init__(self, *args, **kwargs):
            super(ProxyMixin, self).__init__(*args, **kwargs)
            self.init_dma()
            logger.debug('Initialized DMA')


    class Proxy(ProxyMixin, _Proxy):
        pass

    class I2cProxy(ProxyMixin, _I2cProxy):
        pass

    class SerialProxy(ProxyMixin, _SerialProxy):
        pass

except (ImportError, TypeError):
    Proxy = None
    I2cProxy = None
    SerialProxy = None
