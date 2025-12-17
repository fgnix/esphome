#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "pca9685_output.h"

namespace esphome {
namespace pca9685 {

/// Helper class to expose a PCA6416A pin as an internal input GPIO pin.
class PCA9685GPIOPin : public GPIOPin {
 public:
  void setup() override {}
  void pin_mode(gpio::Flags flags) override;
  gpio::Flags get_flags() const override;
  bool digital_read() override;
  void digital_write(bool value) override;
  std::string dump_summary() const override;

  void set_parent(PCA9685Output *parent);
  void set_pin(uint8_t pin);
  void set_inverted(bool inverted);

 protected:
  PCA9685Output *parent_;
  uint8_t channel_;
  bool inverted_;
};

}  // namespace pca9685
}  // namespace esphome
