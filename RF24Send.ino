
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };


void setup(void)
{
  Serial.begin(57600);
  
  Serial.println();
  Serial.println("RF24 SEND started");
  Serial.println("*** ENTER NUMBER to transmit to the other node");

  // Setup and configure rf radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to improve reliability
  // radio.setPayloadSize(8);
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
}

void loop(void)
{
  if ( Serial.available() )
  {
    boolean success = SendMessage(Serial.parseFloat());
      
    if(success){
      Serial.println("SEND Successful");
    }else{
      Serial.println("SEND FAILED");
    }
  }
}

// sends a message that can be as large as a long
boolean SendMessage(unsigned long message){
    long start = millis();
  
    boolean success = false;
    radio.stopListening();

    Serial.print("Now sending...");
    Serial.print(message);
    
    // Take the message and send it.  This will block until complete
    bool ok = radio.write( &message, sizeof(unsigned long) );
    
    if (ok)
      Serial.println(" ok...");
    else
      Serial.println(" FAILED.");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      Serial.println("Failed, response timed out.");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_message;
      radio.read( &got_message, sizeof(unsigned long) );

      Serial.print("Got response: ");
      Serial.println(got_message);
      
      if(message==got_message){
        Serial.println("send/response EQUAL");
        success = true;
      }else{
        Serial.println("send/response NOT EQUAL");
      }
    }
    
    Serial.print("roundtrip time: ");
    Serial.println(millis() - start);
}

