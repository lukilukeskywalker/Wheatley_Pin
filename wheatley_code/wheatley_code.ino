/*
 * Autor: Lukilukeskywalker
 * For attiny85
 * Pinout for wheatley badge is as follows:
 * 5, 6, 7 RGB leds outputs
 * 1 (RESET) is button. The idea is that it enters in deep sleep and reset wakes it
 */
 #define RED PB1
 #define GREEN PB0
 #define BLUE PB2
 
 #include <avr/sleep.h>
 #include <avr/wdt.h>
 #include <util/delay.h>

 boolean unit;
 volatile int pwm_len = 100;
 struct pwm_struct{
  uint8_t active;
  uint16_t start_point;
  uint16_t end_point;
 };
 volatile pwm_struct PWM_BLUE{0, 0, 0};
 volatile pwm_struct PWM_GREEN{0, 0, 0};
 volatile pwm_struct PWM_RED{0, 0, 0};
ISR(WDT_vect){
  //Vector de interrupcion del watchdog
  unit!=unit;
  WDTCR |= 1<<WDIE; //Se setea el 6 bit de WDTCR para que la siguiente activacion del watchdog no sea un reset
  digitalWrite(RED, HIGH);
  
}
ISR(TIMER1_COMPA_vect){
  if(pwm_len <= 100)pwm_len++;
  else pwm_len=0;
  if(pwm_len>=PWM_BLUE.end_point) PORTB &= ~_BV(BLUE);//if(pwm_len>=PWM_BLUE.end_point+PWM_BLUE.start_point && pwm_len<PWM_BLUE.start_point) PORTB &= ~_BV(BLUE);
  else PORTB |= _BV(BLUE);
}
ISR(TIMER1_COMPB_vect){
  PINB |= _BV(GREEN);
}
 
void setup() {
  // put your setup code here, to run once:
  /*if((MCUSR&PORF)!=0){
    MCUSR |= _BV(PORF);
    //Only start animation if reset button was pressed. 
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
    //Shouldnt get out of this unless a reset.
  }*/
  
  PLLCSR |= _BV(PLLE);
  {
    unsigned char counter;
    
    /* Wait at least 100 us for PLL to stabilize */
    for(counter = 0; counter < 250; counter++)
    {
      asm("nop"); /* Each NOP should take 500 ns -- MCU Clock at 2MHz */
    }
  }
  while((PLLCSR & (1<<PLOCK)) == 0);
  
  //PLLCSR |= _BV(PCKE);  //Sets PLL Clock to Timer1 pag 94
  //PLLCSR &=~_BV(LSM);   //Resets  LSM (Low Speed Mode) pag 94
  //CLKPR = _BV(CLKPCE);    //Prepares for sys clk prescaler
  //CLKPR = 0;              //Sets after preparing sys clk prescaler to 0
  setup_watchdog(9);  //8s
  initTimerCounter1();
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  if(PLLCSR & (1<<PLOCK))digitalWrite(BLUE, HIGH);
  //wakeup_anim();
}

void loop() {
  // put your main code here, to run repeatedly:
    // fade in from min to max in increments of 5 points:
    PWM_BLUE.active=1;
  //while(1)yield();
  //delay(1000);
  _delay_us(1000000);
  PINB |= _BV(BLUE);
//  for (int fadeValue = 0 ; fadeValue <= 100; fadeValue += 1) {
//    // sets the value (range from 0 to 255):
//    PWM_BLUE.end_point=fadeValue;
//    //OCR1B=fadeValue;
//    // wait for 30 milliseconds to see the dimming effect
//    delay(30);
//  }
//  for (int fadeValue = 100 ; fadeValue >= 0; fadeValue -= 1) {
//    // sets the value (range from 0 to 255):
//    PWM_BLUE.end_point=fadeValue;
//    delay(30);
//  }

}

void setup_watchdog(int ii){
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  
  MCUSR &= ~(1<<WDRF);  //WDFR contiene el flag de reset por watchdog. Se pone a 0 con operacion AND
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE); //Bits WDCE y WDE controlan el estado del watchdog. WDCE es Watchdog Change Enable. 
  // set new watchdog timeout value
  WDTCR = bb; //Setea el tiempo de activacion del watchdog
  WDTCR |= _BV(WDIE); //Activa el modo de Watchdog Timeout Interrupt Enable

  //INFO en pag 45 de datasheet
}
void initTimerCounter1(void){
    TCCR1 = 0;                  //stop the timer
    TCNT1 = 0;                  //zero the timer
    GTCCR = _BV(PSR1);          //reset the prescaler
    OCR1A = 1;                //set the compare value
    OCR1B = 0;
    OCR1C = 0;
    //TIMSK = _BV(OCIE1A);// | _BV(OCIE1B);        //interrupt on Compare Match A
    TIMSK =  _BV(OCIE1B);
    //start timer, ctc mode, prescaler clk/16384    
    //TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12) | _BV(CS11) | _BV(CS10);
    TCCR1 = _BV(CTC1) | _BV(CS13);
    sei();
}


void wakeup_anim(){
  PWM_BLUE.active=1;
  for (int fadeValue = 0 ; fadeValue <= 25; fadeValue += 5) {
    PWM_BLUE.end_point=fadeValue;
    delay(50);
  }
  
}
