#include "pca9685_gpio.h"

namespace esphome {
namespace pca9685 {

static const char *const TAG = "pca9685";

void PCA9685GPIOPin::pin_mode(gpio::Flags flags) {
	if (flags != gpio::FLAG_INPUT) {
		ESP_LOGE(TAG, "Channel %02u: Unsupported flag %02x, only OUTPUT mode is supported", this->channel_, flags);
	}
}

gpio::Flags PCA9685GPIOPin::get_flags() const { return gpio::FLAG_INPUT; }

bool PCA9685GPIOPin::digital_read() {
	ESP_LOGW(TAG, "Channel %02u: Does not support digial read", this->channel_);
	return false;
}

void PCA9685GPIOPin::digital_write(bool value) {
	uint16_t pwm_value = 0;
	if (value != this->inverted_)
		pwm_value = 4096 - 1; // PWM value max
	this->parent_->set_channel_value_(this->channel_, pwm_value);
}

std::string PCA9685GPIOPin::dump_summary() const {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%u via PCA9685", this->channel_);
  return buffer;
}

void PCA9685GPIOPin::set_parent(PCA9685Output *parent) { this->parent_ = parent; }
void PCA9685GPIOPin::set_pin(uint8_t pin) { this->channel_ = pin; }
void PCA9685GPIOPin::set_inverted(bool inverted) { this->inverted_ = inverted; }

}  // namespace pca9685
}  // namespace esphome
