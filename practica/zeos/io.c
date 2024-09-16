/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

volatile Word* vga_buffer = (Word*)0xb8000;

volatile Word color = 0x0200;

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | color;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}

void set_xy(Byte mx, Byte my){
  x = mx;
  y = my;
}

void set_text_color(enum vga_color fg, enum vga_color bg){
  //fg = VGA_COLOR_LIGHT_GRAY;
  //bg = VGA_COLOR_LIGHT_RED;
  color = (Byte)fg | ((Byte)bg << 4); //esto esta OK creo
  color = color << 8;
    // Iterar a través del buffer de pantalla y actualizar los colores
    /*for (int i = 0; i < NUM_COLUMNS * NUM_ROWS; i++) {
        vga_buffer[i] = (vga_buffer[i] & 0x00FF) | color; // Preservar el carácter, actualizar el color
    }*/
}
