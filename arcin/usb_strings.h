#ifndef USB_STRINGS_H
#define USB_STRINGS_H

#include <cstdint>
#include <usb/usb.h>

uint32_t serial_num() {
  uint32_t *uid = (uint32_t *)0x1ffff7ac;
  return uid[0] * uid[1] * uid[2];
}

class USB_strings : public USB_class_driver {
private:
  USB_generic &usb;
  const uint8_t *label;

public:
  USB_strings(USB_generic &usbd, const uint8_t *l) : usb(usbd), label(l) {
    usb.register_driver(this);
  }

protected:
  virtual SetupStatus handle_setup(uint8_t bmRequestType, uint8_t bRequest,
                                   uint16_t wValue, uint16_t wIndex,
                                   uint16_t wLength) {
    // Get string descriptor.
    if (bmRequestType == 0x80 && bRequest == 0x06 &&
        (wValue & 0xff00) == 0x0300) {
      const void *desc = nullptr;
      uint32_t i = 1;
      uint16_t buf[256] = {0x300};
      switch (wValue & 0xff) {
      case 0: {
        desc = u"\u0304\u0409";
        uint8_t len = *(uint8_t *)desc;
        uint32_t dp = reinterpret_cast<std::uintptr_t>(desc);
        usb.write(0, (uint32_t *)dp, len);
      } break;
      case 1: {
        buf[0] = 0x0300;
        for (const char *p = "Konami Amusement"; *p; p++) {
          buf[i++] = *p;
        }
        buf[0] |= i * 2;
        desc = buf;
        uint8_t len = *(uint8_t *)desc;
        uint32_t dp = reinterpret_cast<std::uintptr_t>(desc);
        usb.write(0, (uint32_t *)dp, len);
      } break;

      case 2: {
        buf[0] = 0x0300;
        for (const char *p =
                 "beatmania IIDX controller premium model";
             *p; p++) {
          buf[i++] = *p;
        }
        buf[0] |= i * 2;
        desc = buf;
        uint8_t len = *(uint8_t *)desc;
        uint32_t *dp = (uint32_t *)desc;
        while (len > 64) {
          usb.write(0, dp, 64);
          dp += 16;
          len -= 64;
          while (!usb.ep_ready(0))
            ;
        }
        usb.write(0, dp, len);
      } break;

      case 3: {
        buf[0] = 0x0312;
        uint32_t id = serial_num();
        for (int i = 8; i > 0; i--) {
          buf[i] = (id & 0xf) > 9 ? 'A' + (id & 0xf) - 0xa : '0' + (id & 0xf);
          id >>= 4;
        }
        desc = buf;
        uint8_t len = *(uint8_t *)desc;
        uint32_t dp = reinterpret_cast<std::uintptr_t>(desc);
        usb.write(0, (uint32_t *)dp, len);
      } break;
      }

      if (!desc) {
        return SetupStatus::Unhandled;
      }
      return SetupStatus::Ok;
    }
    return SetupStatus::Unhandled;
  }
};

#endif
