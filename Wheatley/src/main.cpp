#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "event.c"  //This crap doesn't want to compile with event.h for whatever what reason
#include "color.c"

#define RED 1//PB1
#define GREEN 0//PB0
#define BLUE 2//PB2
#define MAX_EVENTS 3
int setTIM1CompVal_A(byte value);
int setTIM1CompVal_B(byte value);
typedef enum {STARTUP=0, CURIOUS=1, ANOYED=2}state_t;
static state_t* state_p;  //Status de weatley. Puntero consta;nte, dato variable
org_t* organizador=NULL;
ISR(WDT_vect){
  //Vector de interrupcion del watchdog
  WDTCR |= 1<<WDIE; //Se setea el 6 bit de WDTCR para que la siguiente activacion del watchdog no sea un reset
  //PINB |= _BV(RED);
  switch(*state_p){
    case STARTUP:
      break;
    case CURIOUS:
      *state_p = STARTUP;
      break;
    case ANOYED:
      *state_p = CURIOUS;
      break;
    default: 
      *state_p = STARTUP;
      break;
  }
}
ISR(TIMER1_COMPA_vect){
  PORTB &= (ret_actual_ON_event_ID()>>8)&0xFF;    //Shut down
  PORTB  |= 0xFF&ret_actual_ON_event_ID();        //Set pin
  setTIM1CompVal_A(ret_next_ON_event_time()); //THIS FUCKER 
}
ISR(TIMER1_COMPB_vect){
  //PINB |= pin_mask;
  PINB |= ret_actual_OFF_event_ID();
  //PORTB &= !ret_actual_OFF_event_ID();
  setTIM1CompVal_B(ret_next_OFF_event_time());
}
void wakeup_anim(void);
void curious_anim(void);
void setup_watchdog(int ii);
void anoyed_anim(void);
void surprised(void);
void setup() {
  // PLLCSR |= _BV(PLLE);  //Start PLL 
  // do{
  //   _delay_us(100);
  // }while((PLLCSR & _BV(PLOCK)) == 0); //Wait for PLL to stabilize
  //DDRB = 0x00;
  init_org(&organizador, MAX_EVENTS);
  del_all_events();
  DDRB |= _BV(BLUE) | _BV(GREEN) | _BV(RED) | _BV(PB3);  //SET OUTPUT PINS
  //setup_watchdog(9);
    //*state_p=ANOYED;
  switch(*state_p) { 
    
    case STARTUP:
      *state_p = ANOYED;
      wakeup_anim();
      break;
    case CURIOUS:
      *state_p = ANOYED;
      curious_anim();
      break;
    case ANOYED:
      anoyed_anim();
      break;
    default:
      *state_p = ANOYED;
      break;
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  /*set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
   _delay_ms(1000);
   PINB |= _BV(GREEN);*/
   
   surprised();
   _delay_ms(50);
   //_delay_ms(2000);
    //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    //sleep_mode();
    //sleep_enable();
   //_delay_ms(1000);
}

int setTIM1CompVal_A(byte value){
  OCR1A = value;
  TIMSK |= _BV(OCIE1A);
  return OCR1A;
}
int setTIM1CompVal_B(byte value){
  OCR1B = value;
  TIMSK |= _BV(OCIE1B);
  return OCR1B;
}
int setTIM1ResetVal(byte value){
  OCR1C = value;
  return OCR1C;
}
int stopTIM1(void){
  TCCR1 = 0;
  TIMSK &= ~(_BV(OCIE1A)|_BV(OCIE1B));
  return TCCR1;
}
int setupTIM1(byte comp_val_A){
  //if(TCCR1 == 0){
    TCCR1 = 0;  //STOP TIMER1
    //TCNT1 = 0;  //ZERO TIMER1
    GTCCR = _BV(PSR1); //RESET PRESCALER
    setTIM1CompVal_A(comp_val_A);
    //setTIM1CompVal_B(comp_val_B);
    setTIM1ResetVal(255);
    TCCR1 = _BV(CTC1) | _BV(CS13);
    sei();
  //}
  return 0;

}
int setupTIM1(byte comp_val_A, byte comp_val_B){
  //if(TCCR1 == 0){
    TCCR1 = 0;  //STOP TIMER1
    TCNT1 = 0;  //ZERO TIMER1
    GTCCR = _BV(PSR1); //RESET PRESCALER
    setTIM1CompVal_A(comp_val_A);
    setTIM1CompVal_B(comp_val_B);
    setTIM1ResetVal(255);
    TCCR1 = _BV(CTC1) | _BV(CS13);
    sei();
  //}
  return 0;
} 
void setup_watchdog(int ii){
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
  byte bb;
  //int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  //ww=bb;

  
  MCUSR &= ~(1<<WDRF);  //WDFR contiene el flag de reset por watchdog. Se pone a 0 con operacion AND
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE); //Bits WDCE y WDE controlan el estado del watchdog. WDCE es Watchdog Change Enable. 
  // set new watchdog timeout value
  WDTCR = bb; //Setea el tiempo de activacion del watchdog
  WDTCR |= _BV(WDIE); //Activa el modo de Watchdog Timeout Interrupt Enable

  //INFO en pag 45 de datasheet
}
void wakeup_anim(void){
  //PORTB &= 0; //Make sure all leds are off
  setupTIM1(0, 1);
  for(int i=1; i<150; i++){
    _delay_ms(50);
    //setTIM1CompVal_B(i);
    set_full_event(_BV(BLUE), 0, i);
  }
  
  stopTIM1();
  del_full_event(_BV(BLUE));

  PORTB |= _BV(BLUE);
  for(int i=0; i<3; i++){
    PINB |= _BV(BLUE);
    _delay_ms(750);
    PINB |= _BV(BLUE);
    _delay_ms(500);
  }
  PORTB &= 0;
}
void curious_anim(void){
  PORTB &=0;
  for(int i=0; i<6; i++){
    PINB |= _BV(GREEN);
    _delay_ms(500);
    PINB |= _BV(GREEN);
    _delay_ms(750);
  }
}
void anoyed_anim(void){
  PORTB &=0;
  setupTIM1(0, 1);
  for(int i=1; i<150; i++){
    _delay_ms(150);
    set_full_event(_BV(RED), 0, i);
  }
  stopTIM1();
  del_full_event(_BV(RED));
}
double drand ( double low, double high ){
  return ( (double)rand() * ( high - low ) ) / (double)RAND_MAX + low;
}
void setcolor(RGB *color){
  /*So lets think about this problem:
    If 2 color components (CC) are the same, we dont need to make 2 interrupts for making it on or off
    If they are diferent but close enough, we may actually set them together if the diference isnt noticeable, but we also might
    shift the ON point to a later moment in a way that diferent interrupts dont interfere
    On the other hand, if a color is in a high value, most of the time, I think It shouldn't use
    the interrupt vector, as the pulse width doesnt correlate linear to the brightness perceived by the human eye

  */
  int t0, t1=0;
  byte OFF_mask, ON_mask=0;
  while(((PORTB>>BLUE)&1U)|((PORTB>>RED)&1U)|((PORTB>>GREEN)&1U));
  cli();  //Disable global interrupt
  stopTIM1();
  del_all_events();
  PINB |= _BV(PB3);
  for(int i=0; i<4; i++){
    //if(color->R+i!=0){
    if(*((byte*)color+i)!=0){
      OFF_mask = ~ON_mask;
      ON_mask = (i == 0)? _BV(RED) : ((i == 1)? _BV(GREEN) : (i == 2)? _BV(BLUE) : 0);
      t0 = t1;
      //t1 = (t0 + (color->R+i))%255;
      switch(i){
        case(0):
          t1 = (t0+(color->R))%255;
          //t1 = ()
          break;
        case(1):
          t1 = (t0+(color->G))%255;
          break;
        case(2):
          t1 = (t0+(color->B))%255;
          break;
      }
      //t1 = (t0 + *(byte*)(color+i))%255;
      //t0 = (t0<4)? 4 : t0;
      t1 = ((t1-t0)<4)? t0+4 : t1;
      set_ON_event(OFF_mask<<8|ON_mask, t0);
    }
    else{

    }
    /*else{
      del_full_event();
    }*/
    
  }
  PINB |= _BV(PB3);
  setupTIM1(0); //Internally the global interrupt will be enabled
  /*unsigned char mascara = 0;
  static unsigned char mascara_vieja;
  
  if(color.R != 0){
    mascara |= _BV(RED);
    set_OFF_event(_BV(RED), color.R);
  }else if(mascara_vieja&_BV(RED)){
    del_full_event(_BV(RED));
  }
  if(color.G != 0){
    mascara |= _BV(GREEN);
    set_OFF_event(_BV(GREEN), color.G);
  }else if(mascara_vieja&_BV(GREEN)){
    del_full_event(_BV(GREEN));
  }
  if(color.B != 0){
    mascara |= _BV(BLUE);
    set_OFF_event(_BV(BLUE), color.B);
  }else if(mascara_vieja&_BV(BLUE)){
    del_full_event(_BV(BLUE));
  }
  if(mascara_vieja != mascara){
    del_full_event(mascara_vieja);
    //ATENCION! Hay que eliminar todavia los colores del buffer OFF
    setupTIM1(0, 40);
    set_ON_event(mascara, 0);
    mascara_vieja = mascara;
    //PINB |= _BV(RED);
  }*/
}
void surprised(void){
  static int h1, h2;
  if(((h1-h2)==0))h2=360*drand(0,1);
  if(h1>=h2){
    h1--;
  }else{
    h1++;
  }
  //if(h1<=255)h1++;
  //else h1=0;
  struct HSV color = {h1, 1, 0.1};
  setcolor(&HSVToRGB(color));
  //struct RGB color ={128, 128, 128};
  //setcolor(&color);
  //setcolor({255, 255, 0});
  /*double golden_ratio_conjugate = 0.618033988749895;
  static double h1;
  static double h2;
  struct HSV color;
  double cociente = (h2+golden_ratio_conjugate)/1;
  h1=h2-cociente;//drand(0, 1);
  setupTIM1(0, 40);
  if(h1<h2){
    for(h1; h1<h2; h1=h1+0.05){
      _delay_ms(50);
      color = {h1, 0.7, 0.5};
      setcolor(HSVToRGB(color));
    }
  }
  else if(h2<h1){
    for(h1; h1>h2; h1=h1-0.05){
      _delay_ms(50);
      color = {h1, 0.7, 0.5};
      setcolor(HSVToRGB(color));
    }
  }*/
  //struct HSV color={h, 0.5, 0.95};
  //RGB colorrgb1 = HSVToRGB(color);
  
  //h = h - cociente;
  //color={h, 0.5, 0.95};
  //RGB colorrgb2 = HSVToRGB(color);
  //h2 = h1;
}
