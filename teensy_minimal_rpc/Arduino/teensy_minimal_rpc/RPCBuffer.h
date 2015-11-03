#ifndef ___RPC_BUFFER__H___
#define ___RPC_BUFFER__H___

#include <stdint.h>


#ifndef PACKET_SIZE
#define PACKET_SIZE   512
#endif  // #ifndef PACKET_SIZE

/* ## default settings ## */
#ifndef I2C_PACKET_SIZE
#define I2C_PACKET_SIZE   PACKET_SIZE
#endif  // #ifndef I2C_PACKET_SIZE


#endif  // #ifndef ___RPC_BUFFER__H___

