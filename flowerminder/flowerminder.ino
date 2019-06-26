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
  for (int i = 1; i <= days; i++) {
    stayInDeepSleepForHours(24); //sleeps for one day
  }
}

/**
  Stay in deep sleep for the specified amount of hours
  @param hours    The amount of hours to stay in deep sleep
*/
void stayInDeepSleepForHours(unsigned long hours) {
  for (int i = 1; i <= hours; i++) {
    stayInDeepSleepForMinutes(60); //sleeps for one hour
  }
}

/**
  Stay in deep sleep for the specified amount of days
  @param minutes    The amount of days to minutes in deep sleep
*/
void stayInDeepSleepForMinutes(unsigned long minutes) {
  for (int i = 1; i <= minutes; i++) { //counts if the amounts of minutes is less than or equal amount of 1
    stayInDeepSleepForMilliseconds(60000); //sleeps for one minute
  }
}

/**
  Disable ADC so to save power
*/
void disableADC() {
  ADCSRA = 0;
}

void setup() {
  pinMode(potSignalPin, INPUT);
  pinMode(potPowerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  int potValue = getPotValue(); //reads a value from potentiometer and saves it into the variable potValue
  disableADC();
  int days = 0;
  if (potValue < 128)
  {
    days = 1;
  }
  if (potValue >= 128 && potValue < 256)
  {
    days = 2;
  }
  if (potValue >= 256 && potValue < 384)
  {
    days = 3;
  }
  if (potValue >= 384 && potValue < 512)
  {
    days = 4;
  }
  if (potValue >= 512 && potValue < 640)
  {
    days = 5;
  }
  if (potValue >= 640 && potValue < 768)
  {
    days = 6;
  }
  if (potValue >= 768 && potValue < 896)
  {
    days = 7;
  }
  if (potValue >= 896)
  {
    for (int i = 1; i <= 10; i++) { //runs the loop below if it's less than or equals 10 times
      digitalWrite(ledPin, HIGH);
      stayInDeepSleepForMilliseconds(20);
      digitalWrite(ledPin, LOW);
      stayInDeepSleepForMilliseconds(20);
    }
    goToSleep(); //stay in deep sleep until woken up
  }
  for (int i = 1; i <= days; i++) {
    digitalWrite(ledPin, HIGH);
    stayInDeepSleepForMilliseconds(180);
    digitalWrite(ledPin, LOW);
    stayInDeepSleepForMilliseconds(180);
  }
  stayInDeepSleepForDays(days); //blink the led as many times as days is, and sleep for as many days as days is
}

void loop() {
  digitalWrite(ledPin, HIGH);
  stayInDeepSleepForMilliseconds(1000);
  digitalWrite(ledPin, LOW);
  stayInDeepSleepForMilliseconds(1000);
}
