try:
    from .node import Proxy as _Proxy, I2cProxy as _I2cProxy

    class ProxyMixin(object):
        '''
        Mixin class to add convenience wrappers around methods of the generated
        `node.Proxy` class.

        For example, expose config and state getters/setters as attributes.
        '''
        @property
        def config(self):
            from .config import Config

            return Config.FromString(self.serialize_config().tostring())

        @config.setter
        def config(self, value):
            return self.update_config(value)

        @property
        def state(self):
            from .config import State

            return State.FromString(self.serialize_state().tostring())

        @state.setter
        def state(self, value):
            return self.update_state(value)

        def update_config(self, **kwargs):
            from .config import Config

            config = Config(**kwargs)
            return super(ProxyMixin, self).update_config(config)

        def update_state(self, **kwargs):
            from .config import State

            state = State(**kwargs)
            return super(ProxyMixin, self).update_state(state)

        def _state_of_channels(self):
            return super(ProxyMixin, self).state_of_channels()

        @property
        def state_of_channels(self):
            '''
            Retrieve the state bytes from the device and unpacks them into an
            array with one entry per channel.  Return unpacked array.

            Notes
            -----

            State of each channel is binary, 0 or 1.  On device, states are
            stored in bytes, where each byte corresponds to the state of eight
            channels.
            '''
            import numpy as np

            return np.unpackbits(super(ProxyMixin, self).state_of_channels())

        @state_of_channels.setter
        def state_of_channels(self, states):
            self.set_state_of_channels(states)

        def set_state_of_channels(self, states):
            '''
            Pack array containing one entry per channel to bytes (8 channels
            per byte).  Set state of channels on device using state bytes.

            See also: `state_of_channels` (get)
            '''
            import numpy as np

            ok =  (super(ProxyMixin, self)
                    .set_state_of_channels(np.packbits(states)))
            if not ok:
                raise ValueError('Error setting state of channels.  Check '
                                 'number of states matches channel count.')


    class Proxy(ProxyMixin, _Proxy):
        pass

    class I2cProxy(ProxyMixin, _I2cProxy):
        pass

except (ImportError, TypeError):
    Proxy = None
    I2cProxy = None
