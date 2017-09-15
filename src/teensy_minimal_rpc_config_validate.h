#ifndef ___TEENSY_MINIMAL_RPC_CONFIG_VALIDATE___
#define ___TEENSY_MINIMAL_RPC_CONFIG_VALIDATE___

namespace teensy_minimal_rpc {
namespace config_validate {

template <typename NodeT>
struct OnConfigI2cAddressChanged : public ScalarFieldValidator<uint32_t, 1> {
  typedef ScalarFieldValidator<uint32_t, 1> base_type;

  NodeT *node_p_;
  OnConfigI2cAddressChanged() : node_p_(NULL) {
    this->tags_[0] = 3;
  }

  void set_node(NodeT &node) { node_p_ = &node; }
  virtual bool operator()(uint32_t &source, uint32_t target) {
    if (node_p_ != NULL) { return node_p_->on_config_i2c_address_changed(source); }
    return false;
  }
};

template <typename NodeT>
class Validator : public MessageValidator<1> {
public:
  OnConfigI2cAddressChanged<NodeT> i2c_address_;

  Validator() {
    register_validator(i2c_address_);
  }

  void set_node(NodeT &node) {
    i2c_address_.set_node(node);
  }
};

}  // namespace config_validate
}  // namespace teensy_minimal_rpc

#endif  // #ifndef ___TEENSY_MINIMAL_RPC_CONFIG_VALIDATE___
    
