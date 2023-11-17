/*
  x86 OS Bootloader
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#include "conio.h"
#include "arch/x86/intrinsics.h"
#include "arch/x86/constants.h"
#include "kernel/exception_handler.h"
#include "kernel/debug_logger.h"

static int line;

/*
static
void clrscr_early()
{
  mem_set((void*)VGA_TEXT_BASE, 0, VGA_TEXT_SIZE);
}
*/

void k_log_early(log_level level, const char* str)
{
  static const char attrib_for_level[] = { 0x08, 0x07, 0x0F, 0x0A, 0x0E, 0x04, 0x0C };
  char attrib = attrib_for_level[level % 7];
  volatile char* videoMemory = (char*)VGA_TEXT_BASE;
  for (int i = 0; *str;)
  {
    videoMemory[i++ + line] = *str;
    videoMemory[i++ + line] = attrib;
    str++;
  }
  line += 160;
}

// Global initializer test
struct test_class_t
{
  test_class_t()
  {
    k_log_early(SUCCESS, "l");
  }
};
test_class_t foo;

void kputs(const char* s)
{
  k_log_fmt(SUCCESS, s);
}

/*

CR3" ,---> PageDirectory[]" ,----> PageTable[]" ,-----> 4K Page

                  ^                   ^                   ^
                  |                   |                   |
            page dir index       page tbl index       page offset
                  |                   |                   |

Virtual Address  [31-22]           [21-12]             [11-0]

*/

void initialize_page_directory(uint32_t page_directory_base, uint16_t table_count)
{
  uint32_t *page_directory = (uint32_t*)page_directory_base;

  for (int table = 0; table < table_count; table++)
  {
    uint32_t table_address = page_directory_base + 0x01000 + table * 0x01000;
    uint32_t* table_ptr = (uint32_t*)table_address;
    for (int i = 0; i < 1024; i++)
        table_ptr[i] = ((i + table * 0x1000) * 0x1000) | 0x07; // attributes: supervisor level, read/write, present.
    page_directory[table] = table_address | 0x07;
  }

  // initialize remaining entries to not present
  for (int table = table_count; table < 1024; table++)
    page_directory[table] = 0x00000002;

  write_cr3(page_directory_base);
}

// https://wiki.osdev.org/Setting_Up_Paging
// static
void enable_paging()
{
  write_cr0(0x80000001 | read_cr0());
}

// This is written in assembler and has a bunch of trampolines
extern "C" void load_idt();

void setup_interrupts_table()
{
  load_idt();
}

extern "C"
void remap_irqs(int pic1_base_irq, int pic2_base_irq)
{
  outportb(0x20, 0x11);  // starts the initialization sequence (in cascade mode)
  outportb(0xA0, 0x11);

  outportb(0x21, pic1_base_irq);  // set the IVT entry for the IRQs
  outportb(0xA1, pic2_base_irq);

  outportb(0x21, 0x04);  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  outportb(0xA1, 0x02);  // ICW3: tell Slave PIC its cascade identity (0000 0010)

  outportb(0x21, 0x01);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  outportb(0xA1, 0x01);  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
}

void set_irq_mask(uint16_t irqMask)
{
  outportb(0x21, irqMask & 0xff);
  outportb(0xA1, irqMask >> 8);
}

void setup_interrupts()
{
  load_idt();
  remap_irqs(0x20, 0x28);

  uint16_t irq_mask = 0;
  irq_mask |= 1 << 0;  // timer
  irq_mask |= 1 << 1;  // keyboard
  irq_mask |= 1 << 2;  // slave PIC
  irq_mask |= 1 << 3;  // serial com2,com4
  irq_mask |= 1 << 4;  // serial com1,com3
  irq_mask |= 1 << 12; // mouse

  // unmask the rest to check we don't have hardware generating interrupts we aren't expecting
  irq_mask |= 0xFF;

  set_irq_mask(~irq_mask); // 0xfffc); // unmask the timer and keyboard interrupts
}

static
void relocate_stack(size_t newStackPointer, size_t oldStackCopySize)
{
  // relocate the stack
  asm volatile (
    // Save as our source address the old stack pointer
    "mov   %%esp, %%esi\n"

    // Now point stack at new location (code assumes ds and ss use the same selector)
    "mov   %0, %%esp\n"

    // Copy from old stack oldStackCopySize bytes in to the new stack location
    "push  %%ds\n"
    "pop   %%es\n"                 // movsb uses es segment for the destination
    "mov   %%esp, %%edi\n"         // destination of the copy
    "mov   %1, %%ecx\n"            // count of bytes to copy
    "cld \n rep movsb \n std \n"   // copy in reverse direction
    : : "a"(newStackPointer), "c"(oldStackCopySize) );
}

//extern "C" bool a20_not_enabled();
extern "C" int getch();
extern "C" void gotoxy(unsigned,unsigned);
extern bool a20_is_enabled();
extern void k_sleep(int);
extern void initialize_mouse();
extern void pre_initialize_text();
extern void post_initialize_text();

void a20_check()
{
  if (!a20_is_enabled()) // a20_not_enabled())
  {
    k_log_early(ERROR, "[X] A20 Not okay");
    // halt();
    for (;;)
      ;
  }
}

extern "C" volatile uint32_t reg_buffer[13];
extern "C" int8_t real_mode_sw_interrupt;

void call_real_mode_interrupt(int interrupt, uint32_t eax, uint32_t ebx = 0,
                              uint32_t ecx = 0, uint32_t edx = 0, uint32_t esi = 0,
                              uint32_t edi = 0, uint32_t  es = 0, uint32_t  ds = 0)
{
  reg_buffer[0] = eax;
  reg_buffer[1] = ebx;
  reg_buffer[2] = ecx;
  reg_buffer[3] = edx;
  reg_buffer[4] = esi;
  reg_buffer[5] = edi;
  reg_buffer[6] = es & 0xFFFF;
  reg_buffer[7] = ds & 0xFFFF;
  real_mode_sw_interrupt = interrupt;
  asm volatile ( "call call_real_mode" : : : "memory" );  // Hint to compiler that after calling real-mode, memory could have changed
  asm volatile ( "mfence" : : : "memory" );
}

struct VbeInfoBlock {
   char     VbeSignature[4];         // == "VESA"
   uint16_t VbeVersion;              // == 0x0300 for VBE 3.0
   uint16_t OemStringPtr[2];         // isa vbeFarPtr
   uint8_t  Capabilities[4];
   uint16_t VideoModePtr[2];         // isa vbeFarPtr
   uint16_t TotalMemory;             // as # of 64KB blocks
   uint8_t  Reserved[492];
} __attribute__((aligned(64),packed));

struct VbeModeInfo {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__((aligned(64),packed));

#define VideoParamBuffer    0x00005000

#define ADDR_OFF(addr)        ((addr) & 0xFFFF)
#define ADDR_SEG(addr)        (((addr) >> 4) & 0xF000)
#define LINEAR_ADDR(seg, off) ((uint32_t(seg) << 4) | (uint32_t(off) & 0xFFFF))

static
void signatureSet(char signature[4], const char sig[4])
{
  for (int i = 0; i < 4; i++)
    signature[i] = sig[i];
}

static
bool signatureCheck(char signature[4], const char sig[4])
{
  for (int i = 0; i < 4; i++)
    if (signature[i] != sig[i])
      return false;
  return true;
}

static
uint16_t modeTab[32];

static
uint32_t fbModeIndex;
static
uint32_t fbPtr32;
static
uint32_t fbWidth;
static
uint32_t fbHeight;
static
uint32_t fbDepth;

static
bool getInfoBlock()
{
  uint32_t address = VideoParamBuffer;
  volatile VbeInfoBlock* vbeInfoBlockPtr = (volatile VbeInfoBlock*)address; // Give this an address in low memory which should be free
  volatile VbeInfoBlock& vbeInfoBlock = *vbeInfoBlockPtr;
  signatureSet((char*)vbeInfoBlock.VbeSignature, "VBE2");

  // Get VBE info
  call_real_mode_interrupt(0x10, 0x4F00, 0, 0, 0, 0, ADDR_OFF(address), ADDR_SEG(address), 0);

  uint16_t* modes = (uint16_t*)LINEAR_ADDR(vbeInfoBlock.VideoModePtr[1], vbeInfoBlock.VideoModePtr[0]);
  for (int i = 0; i < 32; i++)
    modeTab[i] = modes[i];

  bool sigOkay = signatureCheck((char*)vbeInfoBlock.VbeSignature, "VESA");
  return (reg_buffer[0] == 0x004f) && sigOkay;
}

static
bool getModeInfo(int modeIdx, volatile VbeModeInfo*& paramsPtr)
{
  uint32_t address = VideoParamBuffer + 256 * modeIdx;
  volatile VbeModeInfo* vbeModeInfoPtr = (volatile VbeModeInfo*)address; // Give this an address in low memory which should be free
  volatile VbeModeInfo& vbeModeInfo = *vbeModeInfoPtr;

  // Get VBE mode info
  call_real_mode_interrupt(0x10, 0x4F01, 0, modeTab[modeIdx], 0, 0, ADDR_OFF(address), ADDR_SEG(address), 0);

  paramsPtr = vbeModeInfoPtr;
  fbPtr32 = vbeModeInfo.framebuffer;  
  // k_log_fmt(DEBUG, "   Mode  %i, %x  dims:  %i x %i  %i bpp  @ %x \n", modeIdx, modeTab[modeIdx], paramsPtr->width, paramsPtr->height, paramsPtr->bpp, paramsPtr->framebuffer);

  return (reg_buffer[0] == 0x004f);
}

#define HAS_LINEAR_FRAMEBUFFER    0x90   // Check , "this is 2 bits

static
bool supportedDepth(int bpp)
{
  return (bpp == 8 || bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32);
}

static
int findMode(int w, int h, int d)
{
  if (!getInfoBlock())
  {
    k_log_early(ERROR, "[ ] Didn't get VBE Info Block");  
    return -1;
  }

  k_log_early(SUCCESS, "[X] Got VBE Info Block");
  int bestDelta = 0x7FFFFFFF;
  int bestMode = -1;
  for (int i = 0; i < 32; i++)
  {
    if (modeTab[i] == 0xFFFF)
      break;
    // Get VBE mode info
    volatile VbeModeInfo* paramsPtr = nullptr;
/*
// Linux has a check like this which checks if linear + ?  and layout is specific types
if ((vminfo.mode_attr & 0x99) == 0x99 &&
			   (vminfo.memory_layout == 4 ||
			    vminfo.memory_layout == 6) &&
			   vminfo.memory_planes == 1) {

// Tests if text mode with this:
(vminfo.mode_attr & 0x15) == 0x05)

EDID:
void vesa_store_edid(void)
{
	struct biosregs ireg, oreg;
	memset(&boot_params.edid_info, 0x13, sizeof(boot_params.edid_info));
	if (vginfo.version < 0x0200)
		return;		// EDID requires VBE 2.0+ 
	initregs(&ireg);
	ireg.ax = 0x4f15;		// VBE DDC 
	ireg.es = 0;			// ES:DI must be 0 by spec 
	intcall(0x10, &ireg, &oreg);
	if (oreg.ax != 0x004f)
		return;		// No EDID

	// BH = time in seconds to transfer EDD information
	// BL = DDC level supported

	ireg.ax = 0x4f15;		// VBE DDC 
	ireg.bx = 0x0001;		// Read EDID 
	ireg.es = ds();
	ireg.di =(size_t)&boot_params.edid_info; // (ES:)Pointer to block
	intcall(0x10, &ireg, &oreg);
}
*/
    if (getModeInfo(i, paramsPtr) && paramsPtr && (paramsPtr->attributes & HAS_LINEAR_FRAMEBUFFER) && supportedDepth(paramsPtr->bpp))
    {
      //k_log_fmt(DEBUG, "  Mode  %i, %x  dims:  %i x %i  %i bpp  @ %x  attribs: 0x%x \n", i, modeTab[i], paramsPtr->width,
      //                      paramsPtr->height, paramsPtr->bpp, paramsPtr->framebuffer, paramsPtr->attributes);
      int dw = paramsPtr->width - w;
      int dh = paramsPtr->height - h;
      int dd = paramsPtr->bpp - d;
      int delta = dw*dw + dh*dh + dd*dd;
      if (delta < bestDelta && paramsPtr->width > 320 && paramsPtr->height > 240)
      {
        bestMode = i;
        bestDelta = delta;
      }
    }
  }

  if (bestMode != -1)
  {
    volatile VbeModeInfo* paramsPtr = nullptr;
    getModeInfo(bestMode, paramsPtr);
    fbWidth = paramsPtr->width;
    fbHeight = paramsPtr->height;
    fbDepth = paramsPtr->bpp;
  }
  return bestMode;
}

//static
//inline
void putPixel(int x, int y, uint16_t r, uint16_t g, uint16_t b)
{
  switch (fbDepth)
  {
    case 15:
      ((uint16_t*)fbPtr32)[y * fbWidth + x] = ((unsigned(r >> 3) & 0x1F) << 10) | ((unsigned(g >> 3) & 0x1F) << 5) | ((unsigned(b >> 3) & 0x1F));
      break;
    case 16:
      ((uint16_t*)fbPtr32)[y * fbWidth + x] = ((unsigned(r >> 3) & 0x1F) << 11) | ((unsigned(g >> 2) & 0x3F) << 5) | ((unsigned(b >> 3) & 0x1F));
      break;
    case 24:
      ((uint8_t*)fbPtr32)[y * fbWidth * 3 + x * 3 + 0] = r;
      ((uint8_t*)fbPtr32)[y * fbWidth * 3 + x * 3 + 1] = g;
      ((uint8_t*)fbPtr32)[y * fbWidth * 3 + x * 3 + 2] = b;
      break;
    case 32:
      ((uint8_t*)fbPtr32)[y * fbWidth * 4 + x * 4 + 0] = r;
      ((uint8_t*)fbPtr32)[y * fbWidth * 4 + x * 4 + 1] = g;
      ((uint8_t*)fbPtr32)[y * fbWidth * 4 + x * 4 + 2] = b;
      ((uint8_t*)fbPtr32)[y * fbWidth * 4 + x * 4 + 3] = 0xff;
      break;
  }
}

/*
static
void putch(int ch)
{
  if (ch == '\n')
    k_log_fmt(SUCCESS, "\n");
  else
    k_log_fmt(SUCCESS, "%c", ch);
}
*/

void banner(const char *str)
{
  volatile char *font = (volatile char*)LINEAR_ADDR(0xffa6,0x000e);
  static const uint8_t mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
  for (int height = 0; height < 8; height++)
  {
    k_log_fmt(WARNING, "\n    ");
    for (const char* string = str; *string; string++)
      for (int pixel = 0; pixel < 8; pixel++)
        putch((font[((*string) << 3) + height] & mask[pixel])?'*':' ');
  }
}

static
bool saved_under = false;
static
int saved_x, saved_y;
static
uint32_t saved_under_pointer[8*16];

bool overlapsMouse(int x, int y, int w, int h)
{
  return !(
      (x > (saved_x + 8)) || 
      ((x + w) < saved_x) ||
      (y > (saved_y + 16)) || 
      ((y + h) < saved_y)
     );
}

void restoreMouse()
{
  if (saved_under)
  {
    for (int y2 = 0; y2 < 16; y2++)
      for (int x2 = 0; x2 < 8; x2++)
      {
        if (fbDepth == 15 || fbDepth == 16)
          ((uint16_t*)fbPtr32)[(saved_y+y2) * fbWidth + saved_x+x2] = saved_under_pointer[y2*8+x2];
        else if (fbDepth == 24)
        {
          ((uint8_t*)fbPtr32)[((saved_y+y2) * fbWidth + saved_x+x2) * 3 + 0] = saved_under_pointer[y2*8+x2] >> 16;
          ((uint8_t*)fbPtr32)[((saved_y+y2) * fbWidth + saved_x+x2) * 3 + 1] = saved_under_pointer[y2*8+x2] >> 8;
          ((uint8_t*)fbPtr32)[((saved_y+y2) * fbWidth + saved_x+x2) * 3 + 2] = saved_under_pointer[y2*8+x2] >> 0;
        }
        else if (fbDepth == 32)
          ((uint32_t*)fbPtr32)[(saved_y+y2) * fbWidth + saved_x+x2] = saved_under_pointer[y2*8+x2];
      }
    saved_under = false;
  }
}

void saveMouse(int x, int y)
{
  saved_x = x;
  saved_y = y;
  for (int y2 = 0; y2 < 16; y2++)
    for (int x2 = 0; x2 < 8; x2++)
    {
        if (fbDepth == 15 || fbDepth == 16)
          saved_under_pointer[y2*8+x2] = ((uint16_t*)fbPtr32)[(saved_y+y2) * fbWidth + saved_x+x2];
        else if (fbDepth == 24)
        {
          uint32_t pix = 0;
          pix |= ((uint8_t*)fbPtr32)[((saved_y+y2) * fbWidth + saved_x+x2) * 3 + 0] << 16;
          pix |= ((uint8_t*)fbPtr32)[((saved_y+y2) * fbWidth + saved_x+x2) * 3 + 1] << 8;
          pix |= ((uint8_t*)fbPtr32)[((saved_y+y2) * fbWidth + saved_x+x2) * 3 + 2] << 0;
          saved_under_pointer[y2*8+x2] = pix;
        }
        else if (fbDepth == 32)
          saved_under_pointer[y2*8+x2] = ((uint32_t*)fbPtr32)[(saved_y+y2) * fbWidth + saved_x+x2];
    }
  saved_under = true;
}

void paintMouse(int x, int y)
{
  static const uint8_t mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
  static const uint8_t pointer[] =
  {
    0b11000000, 0b00000000,
    0b11100000, 0b01000000,
    0b11110000, 0b01100000,
    0b11111000, 0b01110000,
    0b11111100, 0b01111000,
    0b11111110, 0b01111100,
    0b11111111, 0b01111110,
    0b11111111, 0b01111000,
    0b11111110, 0b01001100,
    0b11011110, 0b00001100,
    0b00001111, 0b00000110,
    0b00001111, 0b00000110,
    0b00000111, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000,
    0b00000000, 0b00000000
  };

  for (int height = 0; height < 16; height++)
    for (int pixel = 0; pixel < 8; pixel++)
    {
      if (pointer[height*2 + 0] & mask[pixel])
        putPixel(x + pixel, y + height, 0xff, 0xff, 0xff);
      if (pointer[height*2 + 1] & mask[pixel])
        putPixel(x + pixel, y + height, 0x00, 0x00, 0x00);
    }
}

void drawMousePointer2(int x, int y)
{
  x *= 5;
  y *= 5;

  if (x >= 1280)
    x = 1280;
  if (y >= 1024)
    y = 1024;

  restoreMouse();
  saveMouse(x, y);
  paintMouse(x, y);
}

void drawText(uint8_t attrib, int x, int y, const char* str)
{
  int strlen = 0;
  for (const char* string = str; *string; string++)
    strlen++;
  bool overlapsM = overlapsMouse(x, y, strlen * 8, 8);
  if (overlapsM)
    restoreMouse();

  int r = 0xff, g = 0xff, b = 0xff;
  switch (attrib)
  {
    case 0x08: r = 0x40; g = 0x40; b = 0x40; break; // DARK
    case 0x07: r = 0xc0; g = 0xc0; b = 0xc0; break; // LIGHT
    case 0x0F: r = 0xff; g = 0xff; b = 0xff; break; // BOLD
    case 0x0A: r = 0x00; g = 0xff; b = 0x00; break; // GREEN
    case 0x0E: r = 0xff; g = 0xff; b = 0x00; break; // YELLOW
    case 0x04: r = 0x80; g = 0x00; b = 0x00; break; // RED
    case 0x0C: r = 0xff; g = 0x00; b = 0x00; break; // BRIGHT RED
  }

  volatile char *font = (volatile char*)LINEAR_ADDR(0xffa6,0x000e);
  static const uint8_t mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
  for (int height = 0; height < 8; height++)
    for (const char* string = str; *string; string++)
      for (int pixel = 0; pixel < 8; pixel++)
      {
        if (font[((*string) << 3) + height] & mask[pixel])
          putPixel(x + pixel + (string - str) * 8, y + height, r, g, b);
        else
          putPixel(x + pixel + (string - str) * 8, y + height, 0x00, 0x00, 0x00);
      }

  if (overlapsM)
  {
    saveMouse(saved_x, saved_y);
    paintMouse(saved_x, saved_y);
  }
}

/*
void edit_font(int di)
{
  // 32 bytes per char?  16x8 -> 16, then 2 bits?
  bx = (uint16_t)old_mouse_ptr_y1[di] << 5
  for (int i = 16; i >= 0; i--)
  {
    ax = ((uint16_t*)font_data)[i*2 + dx]
    ax = (ax << 5 | ax >> 11) & masks;
    al = ~(ax & 0xff)  &  es:[i + bx];
    al |= (ax >> 8);
    es:[i + font_index] = al;
  }

  not     word ptr masks                                                                  
  add     di,2                                                                            
  add     word ptr font_index,32                                                          
  ret                                                                                     
edit_font endp               
*/

void initialize_vesa()
{
  pre_initialize_text();

  fbModeIndex = findMode(1024, 768, 16);
  // fbModeIndex = findMode(1280, 1024, 32);
  if (fbModeIndex == (uint32_t)-1)
  {
    k_log_early(ERROR, "[ ] Suitable screen mode not found.");
    return;
  }
  k_log_early(SUCCESS, "[X] Found suitable screen mode.");

  gotoxy(0, line / 160);
  k_log_fmt(SUCCESS, "[X] Found mode %i x %i %i bpp\n", fbWidth, fbHeight, fbDepth);
  line += 160;
}

void initialize_banner()
{
  // clrscr_early();
  banner("MagicCore");
}

void draw_test_pattern()
{
  // Draw a test pattern
  for (int i = 0; i < 64; i++)
    for (int y = 0; y < 1024; y++)
      for (int x = 0; x < 1280; x++)
        putPixel(x, y, x / 4, y / 4, i * 4);
  drawText(NORMAL, 100, 100, "Hello world!");
  k_sleep(1);
}

void initialize_screen()
{
  if (fbModeIndex == (uint32_t)-1)
  {
    return;
  }

  call_real_mode_interrupt(0x10, 0x4F02, 0x4000 | modeTab[fbModeIndex]);
  if (reg_buffer[0] == 0x004F)
  {
    k_log_early(SUCCESS, "[X] screen mode set");
    // draw_test_pattern();
  }
  else
  {
    k_log_early(ERROR, "[ ] screen mode was not set");  
  }

  post_initialize_text();
}

///////////////////////////////////////////////////////////////////////////////
// Memory map

struct memorymap_entry
{
  uint64_t    base_address;
  uint64_t    length;
  uint32_t    type;
  uint32_t    reserved;
};

int memorymap_count = 0;
// static
memorymap_entry memorymap[64];

/*

Reference:
  https://forum.osdev.org/viewtopic.php?f=1&t=418

+------------------------+---------+----------------------------------------------+
| Beginning adress       | Size    | Description                                  |
+------------------------+---------+----------------------------------------------+
| 00000 -   0KB - 000000 |     1KB | Table of pointers to IRQ                     |
| 00400 -   1KB - 001024 |   172B  | BDA (Bios Data Area)                         |
| 004AC - 1196B - 001196 |    68B  | Reserved by IBM                              |
| 004F0 - 1264B - 001264 |    16B  | User communication area (?)                  |
| 00500 - 1280B - 001280 | 29.75KB | ---                                          |
| 07C00 -  31KB - 031744 |   0.5KB | Here is loaded bootloader                    |
| 07E00 -31.5KB - 032256 | 607.5KB | ---                                          |
|*9FC00 - 639KB - 654366 |     1KB | EBDA (Extended BIOS Data Area)               |
| A0000 - 640KB - 655360 |    64KB | Graphic screen of Video Card                 |
| B0000 - 704KB - 720896 |    32KB | Hercules text screen area                    |
| B8000 - 736KB - 753664 |    32KB | 8 planes of VGA text screen area             |
| C0000 - 768KB - 786432 |    32KB | BIOS Of Video Card                           |
| C8000 - 800KB - 819200 |   160KB | BIOS Shadow Area                             |
| F0000 - 960KB - 983040 |    64KB | BIOS of System (main BIOS ROM)               |
+------------------------+---------+----------------------------------------------+

*/

void dump_memorymap()
{
  k_log_fmt(SUCCESS, "Memory Map Layout\n");
  k_log_fmt(SUCCESS, "-----------------\n");
  const char* type_descriptions[] = {
    "Undefined",                 // treat as reserved
    "System RAM",                // available to OS
    "reserved",                  // system ROM, memory-mapped device
    "ACPI Reclaim Memory",       // usable after reading ACPI tables
    "ACPI Non-volatile Storage", // required to be saved
    "Bad RAM"                    // don't use
  };
  for (int i = 0; i < memorymap_count; ++i)
  {
    const char* desc = type_descriptions[(memorymap[i].type > 5) ? 0 : memorymap[i].type];
    k_log_fmt(SUCCESS, "0x%X - 0x%X (%s)\n", memorymap[i].base_address, memorymap[i].base_address + memorymap[i].length, desc);
    // k_log_fmt(SUCCESS, "trace: 0x%x -> 0x%x \n", edi_traces[i].b4, edi_traces[i].af);
  }
  k_log_fmt(SUCCESS, "-----------------\n");
}

static
void initialize_memorymap()
{
  reg_buffer[1] = 0;
  for (memorymap_count = 0; memorymap_count < 64; ++memorymap_count)
  {
    const uint32_t SMAP = 0x534D4150; // 'SMAP'
    uint32_t bufaddr = (uint32_t)(&memorymap[memorymap_count]);

    // Reference:
    //   https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/int-15h-e820h---query-system-address-map.html
    call_real_mode_interrupt(0x15UL, 0xE820UL, reg_buffer[1], 20UL, SMAP, 0, ADDR_OFF(bufaddr), ADDR_SEG(bufaddr), 0);

    if (reg_buffer[1] == 0 || reg_buffer[3] != SMAP)
      break;
    if (bufaddr != LINEAR_ADDR(reg_buffer[6], reg_buffer[5]))
      k_log_early(ERROR, "initialize_memorymap: different address returned");
  }
}

///////////////////////////////////////////////////////////////////////////////

static
void initialize_globals()
{
  typedef void initfunc_t(void);
  extern initfunc_t *init_array_start[], *init_array_end[];
  for (initfunc_t **p = init_array_start; p != init_array_end; p++)
    (*p)();
}

extern "C"
int __main(int argc, const char* argv[]);

struct kernel_parameter_block_t
{
  char     entry_name[8];
  uint32_t disk_lba_start;
  uint16_t kernel_size;
  uint16_t kernel_load_address;
  char     args[16];
};

extern kernel_parameter_block_t* bootloader_parameter_block_ptr;

extern "C"
void _start32()
{
  line = 800; // Display this after the bootloader's text
  k_log_early(SUCCESS, "[X] Start32 reached.");
  initialize_memorymap();
  k_log_early(SUCCESS, "[X] Initialized memory map.");
  a20_check();
  k_log_early(SUCCESS, "[X] A20 okay.");
#if ENABLE_PAGING
  initialize_page_directory(0x0100000, 64); // (After 1MB)
  k_log_early(SUCCESS, "[X] Pagetables Initialized");
  enable_paging();  // TODO: kernel param to decide if enable paging or not. Under emulated it appears a lot slower and problematic for real-time
  k_log_early(SUCCESS, "[X] Paging Enabled");
#endif
  relocate_stack(0x0208000, 256); // (After 2MB)
//  relocate_stack(0x0200000, 256); // (After 1MB) - this value is tied to the exception_handler.cpp code for tracing callstacks
  k_log_early(SUCCESS, "[X] Initialized stack.");
  initialize_globals();
  k_log_early(SUCCESS, "[X] Initialized globals.");
  // TODO: move to main.cpp
  initialize_mouse();
  k_log_early(SUCCESS, "[X] Initialized mouse.");
  setup_interrupts();
  k_log_early(SUCCESS, "[X] Initialized interrupts.");

  // Test exiting protected mode, calling int in real mode, then returning to protected mode
  //remap_irqs(0x08, 0x70);
  // call_real_mode_interrupt();
  //remap_irqs(0x20, 0x28);

  int argc = 2;
  const char* argv[] = { bootloader_parameter_block_ptr->entry_name, bootloader_parameter_block_ptr->args };

  // Start the scheduler
  int ret = __main(argc, argv);

  // Main returned
  k_critical_error(ret, "main exited");
}

extern "C"
void start32()
{
  // Mach-O compiled version comes in via this function
  ((char*)VGA_TEXT_BASE)[0] = 'L'; // debug - outputs something without using pointers to data (changes 'l' to 'L', subtle way to know which toolchain was used)
  _start32();
}

/*
extern "C"
void _start()
{
  kernel_parameter_block_t params
  {
    .entry_name          = "hosted",
    .disk_lba_start      = 0xffffffffUL,
    .kernel_size         = 0xffff,
    .kernel_load_address = 0x0000,
    .args                = "hosted       "
  };

  // Mach-O compiled version comes in via this function
  ((char*)VGA_TEXT_BASE)[0] = '.'; // debug - outputs something without using pointers to data
  _start32(&params);
}
*/
