#if defined(ARDUINO_AVR_UNO)

#include <avr/io.h>
#include <avr/interrupt.h>
#include "UnoTimer.h"

#define PRESCALING         1
#define FREQUENCY_STEPPER  25000
#define INTERRUPTS_PER_MILLISECOND 25

volatile uint8_t current_interrupt_ = 0;
volatile int32_t current_time_ms_ = 0;
IActivity **activities_ = 0;
int num_activities_ = 0;
volatile bool running_ = false;

static void clock_init();

static void start_activities();
static void stop_activities();
static inline void update_activities(int32_t ms);

        
UnoTimer::UnoTimer()
{
}

void UnoTimer::init()
{
        cli();
        clock_init();
        sei(); 
}

static void start_timer()
{
        /* Initialize counter */
        TCNT1 = 0;              
        /* Enable Timer1 */     
        TIMSK1 |= (1 << OCIE1A);
}

static void stop_timer()
{
        /* Disable Timer1 interrupt */
        TIMSK1 &= ~(1 << OCIE1A);
}

void UnoTimer::start(IActivity **a, int n)
{
        stop();
        activities_ = a;
        num_activities_ = n;
        start_activities();
        current_interrupt_ = INTERRUPTS_PER_MILLISECOND - 1;
        current_time_ms_ = 0;
        running_ = true;
        start_timer();
}

void UnoTimer::stop()
{
        running_ = false;
        stop_timer();
        stop_activities();
        num_activities_ = 0;
        activities_ = nullptr;
}

bool UnoTimer::isActive()
{
        return running_;
}

// FIXME
static void start_activities()
{
        for (int i = 0; i < num_activities_; i++) {
                activities_[i]->start();
        }
}

// FIXME
static void stop_activities()
{
        for (int i = 0; i < num_activities_; i++) {
                activities_[i]->stop();
        }
}

// FIXME
static inline void update_activities(int32_t ms)
{
        for (int i = 0; i < num_activities_; i++) {
                if (!activities_[i]->isSecondary())
                        activities_[i]->update(ms);
        }
        for (int i = 0; i < num_activities_; i++) {
                if (activities_[i]->isSecondary())
                        activities_[i]->update(ms);
        }
}

static inline void call_activities()
{
        current_interrupt_++;
        if (current_interrupt_ == INTERRUPTS_PER_MILLISECOND) {
                current_interrupt_ = 0;
                if (running_) {
                        update_activities(current_time_ms_);
                }
                current_time_ms_++;
        }
}

/*
 
  https://fr.wikiversity.org/wiki/Micro_contr%C3%B4leurs_AVR/Le_Timer_1
  http://maxembedded.com/2011/07/avr-timers-ctc-mode/
  
*/
/**
 * \brief: Configure Timer1 to drive the stepper's STEP pulse train.
 *
 *  Timer1 is used as the "stepper timer", i.e. this timer will pulse
 *  the STEP pins of the stepper drivers.
 */
static void clock_init()
{
        current_interrupt_ = 0;
        current_time_ms_ = 0;

        /* Don't enable the timer, yet */
        TIMSK1 &= ~(1 << OCIE1A);
        
        // Use the waveform generation mode, or Clear Timer on Compare
        // (CTC) mode.
        //
        // Register  WGM13 WGM12 WGM11 WGM10  
        // TCCR1A                0     0    
        // TCCR1B    0     1                
        TCCR1A &= ~(1 << WGM10);
        TCCR1A &= ~(1 << WGM11);
        TCCR1B |=  (1 << WGM12);
        TCCR1B &= ~(1 << WGM13); 

        // Disconnect OC1 output: Don't send the PWM to an output
        // pin.
        TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0)
                    | (1 << COM1B1) | (1 << COM1B0));
        
        // Set the prescaling
        //   CS12  CS11  CS10
        //   0     0     0     Disabled
        //   0     0     1     1
        //   0     1     0     8
        //   0     1     1     64
        //   1     0     0     256
        //   1     0     1     1025
        //   1     1     0     Use external clock, falling edge
        //   1     1     1     Use external clock, rising edge

        switch (PRESCALING) {
        case 1:
                TCCR1B |=  (1 << CS10);
                TCCR1B &= ~(1 << CS11);
                TCCR1B &= ~(1 << CS12);
                break;
        case 8:
                TCCR1B &=  ~(1 << CS10);
                TCCR1B |= (1 << CS11);
                TCCR1B &= ~(1 << CS12);
                break;
        }
        
        uint16_t compare_value = F_CPU / PRESCALING / FREQUENCY_STEPPER - 1;
        
        /* Set the compare value: 
           
           F_STEPPER = 25000 (25 kHz)
           P_STEPPER = 1/25000 = 40 µs
           F_CPU = 16 MHz
           Prescaling = 1
           F_CLOCK = 16 MHz
           P_CLOCK = 1/16 µs
           N = 40 µs / (1/16 µs) = 640
           N-1 = 639

           F_STEPPER = 10000 (10 kHz)
           P_STEPPER = 1/10000 = 100 µs
           F_CPU = 16 MHz
           Prescaling = 8
           F_CLOCK = 16 MHz / 8 = 2 MHz
           P_CLOCK = 1/2 µs
           N = 100 µs / (1/2 µs) = 200
           N-1 = 199

           int n = (1 / F_STEPPER) / (1 / F_CPU / prescaling) - 1
                 = F_CPU / (F_STEPPER * prescaling) - 1
                 = 639
         */
        OCR1A = compare_value;
        // Serial.print("stepper.c: compare_value=");
        // Serial.println(compare_value);
}

/**
 * \brief The interrupt service routine for the stepper timer.
 *
 */
ISR(TIMER1_COMPA_vect)
{
        call_activities();
}

#endif // if UNO
