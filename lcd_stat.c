#include <rflpc17xx/rflpc17xx.h>
#include <nhd_spi_lcd.h>
#include "lcd_stat.h"

uint16_t stat_max_nb;
uint8_t lcd_stat_occurence_max;
uint16_t stat_min_nb;
uint8_t lcd_stat_occurence_min;

uint8_t stat_index;
uint16_t previous_nb[LCD_WIDTH];

uint8_t lcd_stat_buffer[LCD_WIDTH * (LCD_HEIGHT >> 3)];

uint16_t min_search(uint16_t* array, uint8_t size, uint8_t* occ){
  uint8_t i;
  uint16_t min = array[0];
  *occ = 1;

  for(i=1;i<size;i++){
    if(array[i]<min){
      min = array[i];
      *occ = 1;
    }
    else if(array[i]==min)
      (*occ)++;
  }
  
  return min;
}


uint16_t max_search(uint16_t* array, uint8_t size, uint8_t* occ){
  uint8_t i;
  uint16_t max = array[0];
  *occ = 1;

  for(i=1;i<size;i++){
    if(array[i]>max){
      max = array[i];
      *occ = 1;
    }
    else if(array[i]==max)
      (*occ)++;
  }
  
  return max;
}


void fill_buffer(uint8_t* buffer, uint16_t nb, uint8_t colonne){
  uint8_t i;
  uint8_t value = (LCD_HEIGHT-1)*(nb-stat_min_nb)/(stat_max_nb-stat_min_nb);

  for(i=0;i<LCD_HEIGHT>>3;i++){
    if(value>>3 == i)
      buffer[(LCD_HEIGHT/8-i-1)*LCD_WIDTH+colonne] = (1<<7)>>(value & 0b111);
    else
      buffer[(LCD_HEIGHT/8-i-1)*LCD_WIDTH+colonne] = 0;
  }
}

RFLPC_IRQ_HANDLER lcd_stat_callback(){
  uint8_t i, j;
  uint16_t current = current_nb;
  uint16_t previous;
  current_nb=0;

  /*Update max and min occurences*/
  if(current==stat_min_nb)
    lcd_stat_occurence_min++;
  if(current==stat_max_nb)
    lcd_stat_occurence_max++;

  /*Update max and min value*/
  if(current>stat_max_nb){
    stat_max_nb = current;
    lcd_stat_occurence_max = 1;
  }
  if(current<stat_min_nb){
    stat_min_nb=current;
    lcd_stat_occurence_min = 1;
    }
  
  /*Update nb and index*/
  previous = previous_nb[stat_index];
  previous_nb[stat_index] = current;
  if(stat_index)
    stat_index--;
  else
    stat_index = LCD_WIDTH-1;

  /*Update the occurence (max or min may be overwrite)*/
  if( previous == stat_min_nb){
    lcd_stat_occurence_min--;
    if(lcd_stat_occurence_min==0){
      stat_min_nb = min_search(previous_nb,LCD_WIDTH,&lcd_stat_occurence_min);
    }
  }
  if( previous == stat_max_nb){
    lcd_stat_occurence_max--;
    if(lcd_stat_occurence_max==0)
      stat_max_nb = max_search(previous_nb,LCD_WIDTH,&lcd_stat_occurence_max);
  }

  /*Update buffer*/
   for( i=stat_index+1, j=0 ; i<128 ; i++, j++)
    fill_buffer(lcd_stat_buffer,previous_nb[i],j);
  for( i=0 ; i<stat_index+1 ; i++, j++)
    fill_buffer(lcd_stat_buffer,previous_nb[i],j);

  /*Draw buffer*/
  nhd_spi_lcd_display_buffer(lcd_stat_buffer);

  /*Stop the interrupt*/
  rflpc_rit_clear_pending_interrupt();
}

void lcd_stat_init(){
  uint8_t i;

  /*Init the lcd screen*/
  nhd_spi_lcd_init(NHD_MAKE_SIZE(LCD_WIDTH,LCD_HEIGHT), MBED_DIP6, MBED_DIP8, MBED_DIP11, RFLPC_SPI1);

  /*Init the stat values*/
  current_nb = 0;
  stat_max_nb = 1;
  stat_min_nb = 0;
  lcd_stat_occurence_max = 0;
  lcd_stat_occurence_min = 128;
  for(i=0;i<LCD_WIDTH;i++)
    previous_nb[i] = 0;
  stat_index =  LCD_WIDTH-1;

  /*Init rit*/
  rflpc_rit_enable ();

  /*Set the callback to be called every 500ms ?*/
  rflpc_rit_set_callback ( rflpc_clock_get_system_clock()>>3, 0, 1, lcd_stat_callback);

  printf("LCD stat enabled\r\n");
}
