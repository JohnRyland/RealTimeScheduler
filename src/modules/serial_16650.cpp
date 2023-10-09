/*
  Real-time Scheduler
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include <config.h>

#ifdef ENABLE_SERIAL_16650

#include "module/serial.h"
#include "module_manager.h"

struct serial_driver_t
{
  uint16_t   port;
  uint16_t   irq;
};

/*

General:
  Acronyms:
    CD   - Carrier detect (remote connected or disconnected)
    DTR  - Data terminal ready
    RTS  - Request to send
    CTS  - Clear to send (remote is ready to receive)
    FIFO - First In / First Out


Chip specific:
  Acronyms:
    DLAB - Divisor Latch Access Bit (when 1, can set the baud rate)

  Registers:
    THR  - Transmitter Holding Buffer
    RBR  - Receiver Buffer
    DLL  - Divisor Latch Low Byte
    IER  - Interrupt Enable Register
    DLH  - Divisor Latch High Byte
    IIR  - Interrupt Identification Register
    FCR  - FIFO Control Register
    LCR  - Line Control Register
    MCR  - Modem Control Register
    LSR  - Line Status Register
    MSR  - Modem Status Register
    SR   - Scratch Register


References:
  [1] https://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming#Divisor_Latch_Bytes

*/

static inline
uint8_t inportb(uint16_t port)
{
  uint8_t ret;
  asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline
void outportb(uint16_t port, uint8_t val)
{
  asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

//                                    dlab
//      Register               off    state  i/o-access
#define COM_THR_REG_OFFSET      0   //  0     W
#define COM_RBR_REG_OFFSET      0   //  0     R
#define COM_DLL_REG_OFFSET      0   //  1     RW
#define COM_IER_REG_OFFSET      1   //  0     RW
#define COM_DLH_REG_OFFSET      1   //  1     RW
#define COM_IIR_REG_OFFSET      2   //  x     R
#define COM_FCR_REG_OFFSET      2   //  x     W
#define COM_LCR_REG_OFFSET      3   //  x     RW
#define COM_MCR_REG_OFFSET      4   //  x     RW
#define COM_LSR_REG_OFFSET      5   //  x     R
#define COM_MSR_REG_OFFSET      6   //  x     R
#define COM_SR_REG_OFFSET       7   //  x     RW

#define IER_BIT_RECV_DATA       0
#define IER_BIT_TRANSMIT_DATA   1
#define IER_BIT_LINE_STATUS     2
#define IER_BIT_MODEM_STATUS    3  // Remote CD, CTS etc
#define IER_BIT_SLEEP_MODE      4  // only on 16750 chips
#define IER_BIT_LOW_POWER       5  // only on 16750 chips
#define IER_BIT_RESERVED_1      6
#define IER_BIT_RESERVED_2      7

#define LCR_BIT_DLAB            7

#define FCR_BIT_ENABLE_FIFOS    0
#define FCR_BIT_CLR_RECV_FIFO   1
#define FCR_BIT_CLR_SEND_FIFO   2
#define FCR_BIT_DMA_MODE        3
#define FCR_BIT_RESERVED        4
#define FCR_BIT_ENABLE_64B_FIFO 5  // only on 16750 chips
#define FCR_BIT_TRIGGER_LVL_1   6
#define FCR_BIT_TRIGGER_LVL_2   7

#define MCR_BIT_DTR             0
#define MCR_BIT_RTS             1
#define MCR_BIT_AUX_OUT_1       2
#define MCR_BIT_AUX_OUT_2       3
#define MCR_BIT_LOOPBACK_MODE   4
#define MCR_BIT_FLOW_CONTROL_ON 5  // only on 16750 chips
#define MCR_BIT_RESERVED_1      6
#define MCR_BIT_RESERVED_2      7

#define HI(val)         ((val) >> 8)
#define LO(val)         ((val) & 0xFF)
#define BIT_VAL(bit)    (1U << (bit))
#define BIT_MASK(bit)   (~(1U << (bit)))

static
void set_baud_rate_divisor(uint16_t port, uint16_t divisor)
{
  if (divisor)
  {
    static const uint8_t DLAB_MASK = BIT_VAL(LCR_BIT_DLAB);
    outportb(port + COM_LCR_REG_OFFSET, inportb(port + COM_LCR_REG_OFFSET) | DLAB_MASK); // Set DLAB
    outportb(port + COM_DLH_REG_OFFSET, HI(divisor)); // Set divisor high byte
    outportb(port + COM_DLL_REG_OFFSET, LO(divisor)); // Set divisor low byte
    outportb(port + COM_LCR_REG_OFFSET, inportb(port + COM_LCR_REG_OFFSET) & ~DLAB_MASK); // Clear DLAB
  }
}

static
void set_line_control(uint16_t port, word_size_t word_size, stop_bits_t stop_bits, parity_t parity, bool break_enabled)
{
  uint8_t line_control_value = (uint8_t)word_size | (((uint8_t)stop_bits) << 2) | (((uint8_t)parity) << 3) | (((uint8_t)break_enabled) << 6);
  outportb(port + COM_LCR_REG_OFFSET, line_control_value);
}

//static
void handle_serial_irq()
{
  static const uint16_t ports[] = { 0x03F8, 0x02F8, 0x03E8, 0x02E8 };
  for (int i = 0; i < 4; ++i)
  {
    uint8_t interrupt_id_register = inportb(ports[i] + COM_IIR_REG_OFFSET);

    // COM1 triggered the interrupt
    if ((interrupt_id_register & 0x01) == 0x00)
    {
      uint8_t reason = (interrupt_id_register >> 1) & 0x07;
      switch (reason)
      {
        case 0: // modem status interrupt
          // action: read MSR
        case 1: // transmitter buffer empty
          // action: write to THR (or read IIR)
        case 2: // received data available
          // action: read RBR
        case 3: // receiver line status notification
          // action: read LSR
        case 4: // -
          // action: none
        case 5: // -
          // action: none
        case 6: // time-out
          // action: read RBR
        case 7: // -
          // action: none
        default:
          // action: none
          break;
      }
    }
  }
}

struct serial_capabilities
{
  bool    is_faulty;
  bool    has_working_fifo;
  bool    has_64_byte_fifo;
};

static
serial_capabilities query_capabilities(serial_driver_t* driver)
{
  serial_capabilities capabilities;

   //  1 << MCR_BIT_LOOPBACK_MODE;

  outportb(driver->port + COM_MCR_REG_OFFSET, 0x1E);                            // Set in loopback mode to test the serial chip
  outportb(driver->port + COM_THR_REG_OFFSET, 0xAE);                            // Send byte 0xAE to check it returns the same byte
  capabilities.is_faulty = inportb(driver->port + COM_RBR_REG_OFFSET) != 0xAE;  // Is faulty if it is not the same byte
  uint8_t interrupt_id_register = inportb(driver->port + COM_IIR_REG_OFFSET);
  capabilities.has_working_fifo = (interrupt_id_register & 0xC0) == 0xC0;       // has working FIFO
  capabilities.has_64_byte_fifo = (interrupt_id_register & 0x20) == 0x20;       // has 64 byte fifo (16750 only)
  return capabilities;
}

static
uint16_t divisor_for_baud(baud_rate_t baud)
{
  // Divisors for the 16650 based on a 115.2 kHz clock
  static const uint16_t divisors[] =
  {
    0x0180, // BAUD_300    
    0x00C0, // BAUD_600    
    0x0060, // BAUD_1200   
    0x0030, // BAUD_2400   
    0x0018, // BAUD_4800   
    0x000C, // BAUD_9600   
    0x0006, // BAUD_19200  
    0x0003, // BAUD_38400  
    0x0002, // BAUD_57600  
    0x0001, // BAUD_115200 
  };

  if (baud >= baud_rate_t::BAUD_300 && baud <= baud_rate_t::BAUD_115200)
    return divisors[static_cast<int>(baud)];

  return divisors[static_cast<int>(baud_rate_t::BAUD_115200)];
}

static
void initialize(serial_driver_t* driver, baud_rate_t baud, word_size_t bits, stop_bits_t stop, parity_t parity)
{
  outportb(driver->port + COM_IER_REG_OFFSET, 0x00);    // Disable all interrupts

  set_baud_rate_divisor(driver->port, divisor_for_baud(baud));
  set_line_control(driver->port, bits, stop, parity, false);

  outportb(driver->port + COM_FCR_REG_OFFSET, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  outportb(driver->port + COM_MCR_REG_OFFSET, 0x0B);    // IRQs enabled, RTS/DSR set       // really enables IRQs?

  serial_capabilities capabilities = query_capabilities(driver);
  if (!capabilities.is_faulty)
  {
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outportb(driver->port + COM_MCR_REG_OFFSET, 0x0F);  // really enables IRQs?
  }

  // Be able to receive IRQs for things which happen - can make the impl more efficient to be
  // interrupt driven
  //outportb(driver->port + COM_IER_REG_OFFSET, 0x0F);    // Enable interrupts
}

static
bool ready_to_recv(serial_driver_t* driver)
{
  return inportb(driver->port + 5) & 1;
}

static
uint8_t read_serial(serial_driver_t* driver)
{
  while (!ready_to_recv(driver))
    ;
  return inportb(driver->port);
}

static
bool ready_to_send(serial_driver_t* driver)
{
  return inportb(driver->port + 5) & 0x20;
}
 
static
void write_serial(serial_driver_t* driver, uint8_t a)
{
  while (!ready_to_send(driver))
    ;
  outportb(driver->port, a);
}

serial_driver_vtable_t serial_16650_vtable =
{
  .initialize = initialize,
  .ready_to_recv = ready_to_recv,
  .ready_to_send = ready_to_send,
  .recv = read_serial,
  .send = write_serial,
};

#define COM1_PORT 0x03F8
#define COM1_IRQ  4

#define COM2_PORT 0x02F8
#define COM2_IRQ  3

#define COM3_PORT 0x03E8
#define COM3_IRQ  4

#define COM4_PORT 0x02E8
#define COM4_IRQ  3

static
serial_driver_t serial_port_1 =
{
  .port = COM1_PORT,
  .irq  = COM1_IRQ,
};

static
serial_driver_t serial_port_2 =
{
  .port = COM2_PORT,
  .irq  = COM2_IRQ,
};

static
serial_driver_t serial_port_3 =
{
  .port = COM3_PORT,
  .irq  = COM3_IRQ,
};

static
serial_driver_t serial_port_4 =
{
  .port = COM4_PORT,
  .irq  = COM4_IRQ,
};

static
module_t serial_port_1_driver = 
{
  .type     = module_class::SERIAL_DRIVER,
  .id       = 0x12024, // TODO: how to assign these? in the register?
  .name     = { "serial_port_1" },
  .next     = nullptr,
  .prev     = nullptr,
  .vtable   = &serial_16650_vtable,
  .instance = &serial_port_1,
};

static
module_t serial_port_2_driver = 
{
  .type     = module_class::SERIAL_DRIVER,
  .id       = 0x12024, // TODO: how to assign these? in the register?
  .name     = { "serial_port_2" },
  .next     = nullptr,
  .prev     = nullptr,
  .vtable   = &serial_16650_vtable,
  .instance = &serial_port_2,
};

static
module_t serial_port_3_driver = 
{
  .type     = module_class::SERIAL_DRIVER,
  .id       = 0x12024, // TODO: how to assign these? in the register?
  .name     = { "serial_port_3" },
  .next     = nullptr,
  .prev     = nullptr,
  .vtable   = &serial_16650_vtable,
  .instance = &serial_port_3,
};

static
module_t serial_port_4_driver = 
{
  .type     = module_class::SERIAL_DRIVER,
  .id       = 0x12024, // TODO: how to assign these? in the register?
  .name     = { "serial_port_4" },
  .next     = nullptr,
  .prev     = nullptr,
  .vtable   = &serial_16650_vtable,
  .instance = &serial_port_4,
};

// 8250_uart is a predecesor
void register_ti_16650_uart_driver()
{
  module_register(serial_port_1_driver);
  module_register(serial_port_2_driver);
  module_register(serial_port_3_driver);
  module_register(serial_port_4_driver);
}

#endif // ENABLE_SERIAL_16650
