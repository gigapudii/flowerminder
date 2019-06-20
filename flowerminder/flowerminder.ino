#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

const int ledPin = PB2;
const int potSignalPin = PB3;
const int potPowerPin = PB4;

volatile bool watchdogBarked = false;

enum WatchDogTimeout {
  WDT_16ms = 0,
  WDT_32ms,
  WDT_64ms,
  WDT_128ms,
  WDT_250ms,
  WDT_500ms,
  WDT_1sec,
  WDT_2sec,
  WDT_4sec,
  WDT_8sec
};

/**
  Watchdog interrupt routine to be triggered when watchdog times out.
*/
ISR(WDT_vect) {
  watchdogBarked = true;
}

void goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_all_disable ();
  sleep_enable();
  sleep_cpu(); // Sleep here and wait for the interrupt
  sleep_disable();
  power_all_enable(); // power everything back on
}

/**
    Sets up watchdog to be triggered (once) after the specified time
    @param wdt  the watchdog timeout duration
*/
void triggerWatchDogIn(WatchDogTimeout wdt) {
  // Adopted from InsideGadgets (www.insidegadgets.com)
  byte timeoutVal = wdt & 7;
  if (wdt > 7) {
    timeoutVal |= (1 << 5);
  }
  timeoutVal |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  WDTCR |= (1 << WDCE) | (1 << WDE); // Start timed sequence
  WDTCR = timeoutVal;
  WDTCR |= _BV(WDIE);
  wdt_reset(); // Pat the dog
}

/**
   A utility method to derive a watchdog timeout's duration
   @param wdt the watchdog timeout
   @return    the amount of milliseconds corresponding to a watchdog timeout
*/
unsigned long getTimeoutDuration(WatchDogTimeout wdt) {
  return 1 << (wdt + 4);
}


/**
   Blocks and stays in deep sleep until the specified time has elapsed
   using the current watchdog timeout.
   @param sleepDuration     how long to stay in deep sleep in milliseconds
   @param timeoutInterval   the watchdog timeout interval
*/
void stayInDeepSleepForMilliseconds(unsigned long sleepDuration, WatchDogTimeout timeoutInterval = WDT_16ms) {
  unsigned long sleepTime = 0;

  // Start by triggering the watchdog to wake us up every `timeoutInterval`
  triggerWatchDogIn(timeoutInterval);
  while (sleepTime <= sleepDuration) {
    // Sleep until an interrupt occurs (external, change or watchdog)
    goToSleep();
    // Verify we woke up because of the watchdog and not
    // a spurious wake up due to some other unrelated interrupt.
    if (watchdogBarked) {
      // Note down that we have processed the watchdog bark
      watchdogBarked = false;
      // Increase the time we have already slept
      sleepTime += getTimeoutDuration(timeoutInterval);
    }
  }
  wdt_disable(); // Disable watchdog so it stops barking
}

/**
  Get a measurement from the potentiometer
  @return The potentiometer value
*/
int getPotValue() {
  digitalWrite(potPowerPin, HIGH); //turn potentiometer on
  int potValue = analogRead(potSignalPin); //reading the value
  digitalWrite(potPowerPin, LOW); //turn it off
  return potValue;
}

/**
  Stay in deep sleep for the specified amount of days
  @param days    The amount of days to stay in deep sleep
*/
void stayInDeepSleepForDays(unsigned long days) {
  // TODO
}

/**
  Stay in deep sleep for the specified amount of hours
  @param hours    The amount of hours to stay in deep sleep
*/
void stayInDeepSleepForHours(unsigned long hours) {
  // TODO
}

/**
  Stay in deep sleep for the specified amount of days
  @param minutes    The amount of days to minutes in deep sleep
*/
void stayInDeepSleepForMinutes(unsigned long minutes) {
  // TODO
}

/**
  Disable ADC so to save power
*/
void disableADC() {
ADCSRA = 0;
}

void setup() {
  // TODO
}

void loop() {
  // TODO
}
