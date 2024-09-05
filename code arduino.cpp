#include <Arduino.h>
#include <math.h>

// Frequency of sin wave
#define SIN_F 1
// Speed Factor 
#define SF 5
// PI value
#define PI		3.14159265358979323846

// Variables to store sin value 
uint8_t SIN_0[360];
uint8_t SIN_1[360];
uint8_t SIN;
// Variable to store theta value
uint16_t t = 0;
// Variable to Store ADC value (signal input)
uint16_t ANALOG_SINAL;
// Input capture unit (ICU) variables
volatile uint16_t Tovf,Tovf1,Capt1,Capt2,Capt3,waitTovf;
volatile uint8_t flag = 0;
uint32_t Rising1,Rising2,Falling1;
uint8_t Dutycycle;
uint16_t freq;

// initialise timer1 for Input capture unit (ICU) function
void InitTimer1()
{
  // disable global interrupts
  noInterrupts();
  // clear timer1 registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  // first capture on rising edge
  TCCR1B |= (1<<ICES1);
  // enable inbut capture and overflow interrupts
  TIMSK1 |= (1<<ICIE1) | (1<<TOIE1);
  // enable timer1 with  prescaller = 1
  TCCR1B |= (1<<CS10);
  // enable global interrupts
  interrupts();
}
// Timer1 overflow interrupt functions
ISR(TIMER1_OVF_vect)
{
  // increment overflow counter
  Tovf++;
}

// Timer1 ICU interrupt functions
ISR(TIMER1_CAPT_vect)
{
  // First rising
  if(flag == 0)
  {
    // save captured time
    Capt1 = ICR1;
    Tovf = 0;
    // chane capture on falling edge
    TCCR1B &= ~(1<<ICES1);
  }
  // First falling
  else if(flag == 1)
  {
    // save captured time
    Capt2 = ICR1;
    Tovf1 = Tovf;
    // chane capture on rising edge
    TCCR1B |= (1<<ICES1);
  }
  // Last rising (Signal period end)
  else if(flag == 2)
  {
    // save captured time
    Capt3 = ICR1;
    waitTovf = Tovf;
    // disable input capture unit interrupts
    TIMSK1 &= ~(1<<ICIE1);
  }
  // increment flag
  flag++;
}

void setup() {
  // initialise serial
  Serial.begin(9600);
  // setup Pin Direction (INPUT or OUTPUT)
  DDRD = 0xFC;
  DDRB = 0x06;
  pinMode(8,INPUT);
  // change prescaller for timer 2 to 1024
  TCCR2B |= 0x07;
  // initialise timer 1 and ICU
  InitTimer1();
  // store sin values per theta and frequency
  for(uint16_t i = 0;i<360;i++)
	{
    SIN = (127*sin(PI*SF*SIN_F*i/180.0)+127);
    SIN_0[i] = SIN & 0xFC;
    SIN_1[i] = ((SIN & 0x03)<<1);
	}
}

void loop() {
  // if ICU end before 20 overflow
  if(flag == 3 && waitTovf + 20 < Tovf)
  {
    // First rising time
    Rising1 = Capt1;
    // First falling time
    Falling1 = (Tovf1*65536) + Capt2;
    // last rising time
    Rising2 = (waitTovf*65536) + Capt3;
    // calculate Duty cycle time
    Dutycycle = round((Falling1 - Rising1)*100.0/(Rising2 - Rising1));
    // calculate Duty cycle frequency
    freq = round(16.0*1000000/(Rising2 - Rising1));
    // reset Variables of ICU
    flag = 0;
    Tovf = 0;
    Tovf1 = 0;
    // clear ICU flag
    TIFR1 = (1<<ICF1);
    // enable input capture unit interrupts
    TIMSK1 |= (1<<ICIE1);
  }
  // store input signal
  ANALOG_SINAL = analogRead(A0);
  // Generate BWM dependent on input signal
  analogWrite (11,ANALOG_SINAL/4);
  // Special cases of ICU
  if(ANALOG_SINAL == 0)
  {
    Dutycycle = 0;
    freq = 30;
  }
  else if(ANALOG_SINAL >= 1020 )
  {
    Dutycycle = 100;
    freq = 30;

  }
  // map on input signal to be from 0 to 1000
  ANALOG_SINAL = ((uint32_t)ANALOG_SINAL*1000)/1023.0;
  // send input signal value
  Serial.print(ANALOG_SINAL);
  Serial.print(",");
  // send PWM signal value
  Serial.print(((PINB & (0x08))>>3));
  Serial.print(",");
  // send PWM Duty cycle value
  Serial.print(Dutycycle);
  Serial.print(",");
  // send PWM frequency value
  Serial.println(freq);
  // Extract the sin signal value to DAC
  PORTD &= 0x03;
  PORTB &= 0xFC;
  PORTD |= (SIN_0[t]);
  PORTB |= (SIN_1[t]);
  // increment theta value
  t++;
  // reset theta to 0 when theta equal 360
  if(t>259)
	  t%=360;
}