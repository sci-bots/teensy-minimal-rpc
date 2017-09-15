#ifndef ___TEENSY_MINIMAL_RPC_STATE_VALIDATE___
#define ___TEENSY_MINIMAL_RPC_STATE_VALIDATE___

namespace teensy_minimal_rpc {
namespace state_validate {

template <typename NodeT>
class Validator : public MessageValidator<0> {
public:

  Validator() {
  }

  void set_node(NodeT &node) {
  }
};

}  // namespace state_validate
}  // namespace teensy_minimal_rpc

#endif  // #ifndef ___TEENSY_MINIMAL_RPC_STATE_VALIDATE___
    
