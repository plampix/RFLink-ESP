// Version 0.2
//    - LoopsPerMilli automatic changed with the device
//    - added protection for only include once

// Version 0.1

#ifndef RawSignal_h
#define RawSignal_h   0.2

// ***********************************************************************************
// Some time critical parameters are declared global
// ***********************************************************************************
unsigned long FETCH_Pulse_Plus_1 ;  //  here 3320,  inside 3540
// ***********************************************************************************
boolean FetchSignal () {
  // ************************************************************
  //   LoopsPerMilli      maxloops       timeout [msec)
  //      500               3500          0.8 ... 1.7  (zeer wisselend)
  //     1500              10500          2.4
  //     4000              28000          6.3
  // detectie van Notenhout knop A ( QIAchip = EV1527)
  //  1500:  2370,270,690,270,690,270,690,270,690,270,690,270,690,270,690,270,720,270,690,750,210,240,690,750,210,750,210,750,210,240,720,750,240,750,210,720,240,720,240,720,240,240,720,240,720,240,720,720,210,210,
  //              2640;
  //  4000:  660,270,690,270,690,270,690,240,690,270,690,270,690,270,750,210,690,270,690,720,240,240,720,750,210,750,210,750,210,270,690,750,210,750,210,750,210,750,240,720,240,240,690,240,720,210,720,690,270,240,
  //            7050;
  // We zien de beginpuls korter worden en de eindpuls langer (dus 1500 was toch wel een hele mooie waarde
  // BELANGRIJK: de laatste puls is dus de timeout in usec !!!
  // We see the initial pulse becoming shorter and the end pulse longer (so 1500 was a very nice value anyway)
  // IMPORTANT: the last pulse is the timeout in usec !!!
  // ************************************************************
  // for the ESP8266 we've tested the width of the last pulse in Learning_Mode = 5
  //     LoopsPerMille     Lastpulswidth
  //        800               3900
  //       1000               4890
  // so 800 is a good value !!
  // ************************************************************
  //const unsigned long LoopsPerMilli = 345;   // <<< OORSPRONKELIJKE WAARDE VOOR ARDUINO MEGA
  // 2500 was optimaal, maar we hebben de loop sneller gemaakt 3540 naar 3320 usec
  // dus verhogen we met 3540/3320 =>  2670
  // 2500 was optimal, but we made the run faster from 3540 to 3320 usec
  // so we increase with 3540/3320 =>  2670
  /*
    #ifdef __AVR_ATmega2560__
    const unsigned long LoopsPerMilli = 345;
    #elif ESP32
    const unsigned long LoopsPerMilli = 2700;
    #elif ESP8266
    const unsigned long LoopsPerMilli = 800;
    #endif
  */
  // ************************************************************
  // ************************************************************

  // ******************************************************************************************
  // WEIRD een aantal parameters niet globaal definieeren, maar hier, maakt de routine sneller
  // in de comment staat de breedte van de laatste puls (getriggered door een timeout, dus korter is sneller)
  // ******************************************************************************************
  // ******************************************************************************************
  // WEIRD do not define a number of parameters globally, but here, the routine makes faster
  // in the comment is the width of the last pulse (triggered by a timeout, so shorter is faster)
  // ******************************************************************************************

  unsigned long LastPulse            ;   // 3320 i.p.v. 3540
  bool          Toggle        = true ;   // 3320 i.p.v. 3750
  int           RawCodeLength = 0    ;   // maakt niet uit
  unsigned long PulseLength          ;   // 3320 i.p.v. 3540
  unsigned long numloops             ;   // 3320 i.p.v. 4200
  bool          Start_Level   = LOW  ;   // via #define maakt niet uit
  //unsigned      maxloops      = ( SIGNAL_TIMEOUT * LoopsPerMilli );  // via #define maakt niet uit
  const unsigned long          maxTime          = ( SIGNAL_TIMEOUT * 1000);
  unsigned long timeStartLoop;
  // ******************************************************************************************


  // ************************************************************
  // wacht op lang laag nivo (Start_Level) = start van nieuwe sequence
  // wait for long low level (Start_Level) = start of new sequence
  // ************************************************************
  bool Started = false ;
  unsigned long Start_Time = 100 + millis() ;  // 100 .. 500 msec, maakt niet zo veel uit  // 100 .. 500 msec, does not really matter (may be smaller give better ping)
  while ( !Started ) {
    // ************************************************************
    // als het nivo laag is, wacht tot het einde van dit laag nivo
    // if the level is low, wait until the end of this low level
    // ************************************************************
    //    while ( ( digitalRead ( PIN_RF_RX_DATA ) == Start_Level ) ) ;
    while ( ( digitalRead ( RECEIVE_PIN ) == Start_Level ) && ( millis() < Start_Time ) ) ;

    // ************************************************************
    // hier is het nivo hoog, wacht totdat het naar laag springt
    // als we de vorige while lus niet doorlopen zijn,
    //   zijn we halverwege een positieve puls deze routine binnen gekomen
    //   en kunnen we deze positieve puls niet nauwkeurig meten
    //   maar vanwege de herhalende sequences zullen we toch regelmatig de juiste waarde meten

    // here the level is high, wait until it jumps to low
    // if we did not go through the previous while loop,
    // we entered this routine halfway through a positive pulse
    // and we can not accurately measure this positive pulse
    // but due to the repeating sequences we will still regularly measure the correct value
    // ************************************************************
    FETCH_Pulse_Plus_1 = micros() ;
    //    while ( ( digitalRead ( RECEIVE_PIN ) != Start_Level ) ) ;
    while ( ( digitalRead ( RECEIVE_PIN ) != Start_Level ) && ( millis() < Start_Time ) ) ;

    // ************************************************************
    // Wacht tot het einde van de laag periode
    // Wait until the end of the low period
    // ************************************************************
    LastPulse = micros() ;
    //    while ( ( digitalRead ( RECEIVE_PIN ) == Start_Level ) ) ;
    while ( ( digitalRead ( RECEIVE_PIN ) == Start_Level ) && ( millis() < Start_Time ) ) ;
    PulseLength = micros() - LastPulse;

    // ************************************************************
    // Als de laag periode voldoende lang is, is het de startpuls, Berg dan ook de positieve en negatieve startpuls op
    // If the low period is sufficiently long, it is the start pulse Store the positive and negative start pulse
    // ************************************************************
    if ( PulseLength > 5000 ) {
      //Serial.print ("PulseLength: ");
      //Serial.println ( PulseLength ) ;
      RawSignal.Pulses [ RawCodeLength++ ] = LastPulse - FETCH_Pulse_Plus_1 ;
      RawSignal.Pulses [ RawCodeLength++ ] = PulseLength ;
      Started = true ;
    }
    if ( millis() > Start_Time ) return false;
  }

  RawSignal.Min  = 10000 ;
  RawSignal.Max  = 0     ;
  RawSignal.Mean = 0     ;
  // ************************************************************
  // Na een start, wordt hier de hele serie pulsen gemeten
  // en er wordt gestopt als een (te) lange puls wordt gevonden (of het buffer vol is)

  // After a start, the whole series of pulses is measured here
  // and it stops when a (too) long pulse is found (or the buffer is full)
  // ************************************************************
  do {
    // ************************************************************
    // Meet de breedte van het huidige nivo
    //   break als het nivo te lang duurt
    // ************************************************************
    numloops  = 0 ;
    timeStartLoop = micros();
    LastPulse = micros () ;
    while ( ( digitalRead ( RECEIVE_PIN ) == Start_Level ) ^ Toggle )
      //if ( numloops++ == maxloops ) break ;
      if ((micros() - timeStartLoop) > maxTime ) break;
    PulseLength = micros() - LastPulse;

    // ************************************************************
    // spring uit de loop als we een te korte puls detecteren

    // jump out of the loop if we detect too short a pulse
    // ************************************************************
    if ( PulseLength < MIN_PULSE_LENGTH ) break ;

    // ************************************************************
    // Inverteer het nivo waar naar we gaan zoeken

    // Invert the level to which we are going to search
    // ************************************************************
    Toggle = !Toggle ;

    // ************************************************************
    // Geldige puls gevonden, dus opbergen

    // Valid pulse found, so store
    // ************************************************************
    RawSignal.Pulses [ RawCodeLength++ ] = PulseLength ;

    // ************************************************************
    // keep track of ststistics
    // ************************************************************
    //if ( numloops < maxloops ) {
    if ((micros() - timeStartLoop) < maxTime ) {
      if ( PulseLength < RawSignal.Min ) RawSignal.Min = PulseLength ;
      if ( PulseLength > RawSignal.Max ) RawSignal.Max = PulseLength ;
      RawSignal.Mean += PulseLength ;
    }

    // ************************************************************
    // stop als er een lange puls is gevonden of als het buffer vol is

    // stop if a long pulse is found or if the buffer is full
    // ************************************************************
    //} while ( ( RawCodeLength < RAW_BUFFER_SIZE ) && ( numloops < maxloops ) ) ;
  } while ( ( RawCodeLength < RAW_BUFFER_SIZE ) && ( (micros() - timeStartLoop) < maxTime ) ) ;
  //Serial.print ("RawCodeLength: ");
  //Serial.println ( RawCodeLength ) ;

  // ************************************************************
  // We hebben nu het einde van een signaal bereikt
  //   als we genoeg pulsen hebben,

  // We have now reached the end of a signal
  //   if we have enough pulses,

  //     return true
  // KAKU has a long startperiod of 2500 usec
  // ************************************************************
  if ( ( RawCodeLength >= MIN_RAW_PULSES ) &&
       ( RawCodeLength <= MAX_RAW_PULSES ) &&
       ( RawSignal.Min > 150 ) &&
       ( RawSignal.Max < 3000 ) ) {
    RawSignal.Repeats = 0;                          // no repeats
    RawSignal.Mean = RawSignal.Mean / ( RawCodeLength - 3 ) ;
    RawSignal.Number   = RawCodeLength - 1 ;          // Number of received pulse times (pulsen *2)
    RawSignal.Pulses [ RawSignal.Number + 1 ] = 0 ;   // Last element contains the timeout.
    RawSignal.Time = millis() ;                       // Time the RF packet was received (to keep track of retransmits
    //Serial.print ( "D" ) ;
    //Serial.print ( RawCodeLength ) ;
    return true ;
  }

  // ************************************************************
  // anders opnieuw beginnen
  // otherwise start again
  // ************************************************************
  else {
    RawSignal.Number = 0 ;
  }

  return false;
}

#endif
