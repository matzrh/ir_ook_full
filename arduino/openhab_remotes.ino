#include <Arduino.h>

/*
   Set library modifiers first to set output pin etc.
*/
#define IRMP_INPUT_PIN   3 // To be compatible with interrupt example, pin 3 is chosen here (which is default).
#define IRSND_OUTPUT_PIN 4
#define TONE_PIN         5
#define IRMP_TIMING_TEST_PIN 6

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ALTERNATIVE_IRMP_FEEDBACK_LED_PIN 7 // used for examples which use LED_BUILDIN for example output.
//#define IR_OUTPUT_IS_ACTIVE_LOW
#define IRSND_IR_FREQUENCY          38000

#define IRSND_PROTOCOL_NAMES        1 // Enable protocol number mapping to protocol strings - requires some FLASH.

//#include <irsndSelectMain15Protocols.h>
#define IRSND_SELECT_PROTOCOLS_H
#define IRSND_SUPPORT_NEC_PROTOCOL              1       // NEC + APPLE          >= 10000                 ~100 bytes
#define IRSND_SUPPORT_KASEIKYO_PROTOCOL         1
#define IRSND_SUPPORT_RF_HME_PROTOCOL           1
#define IRSND_SUPPORT_RF_AC104_PROTOCOL         1
#define IRSND_SUPPORT_RF_GEN24_PROTOCOL         1
#define IRSND_RF_OUTPUT
#define RF_AC104_ID        0x4647a8

#include <irsnd.c.h>
#include "irremotes.h";


IRMP_DATA irsnd_data;
const IRMP_DATA irsnd_ac104_after {
  RF_AC104_PROTOCOL,
  SCREEN_ADDRESS,
  SCREEN_AFTER,
  1 //repetitions
};

void writeAvailableFunctions() {
  for(uint8_t i = 0; i< noOfRemotes; i++) {
    Serial.print(remoteTypes[i]); Serial.print(":");Serial.print(remoteNames[i]); Serial.print(":");
    if(remoteTypes[i] != "enum") {
      Serial.println();
      continue;
    }
    const char** remoteCommands=NULL;
    uint8_t commandSize=remoteSizes[i];
    switch(i) {
      case 0: remoteCommands=amNames; break;
      case 1: remoteCommands=prNames; break;
      case 2: remoteCommands=boxNames; break;
      case 3: remoteCommands=scrNames; break;
      default: continue; break;
    }
    for(uint8_t j=0; j<commandSize; j++) {
      Serial.print(*remoteCommands++);Serial.print(",");
    }
    Serial.println();
  }
}


uint8_t readChar() {
  while (Serial.available() == 0){
  }
  uint8_t incoming=Serial.read();
  return incoming;
}

bool charToIRMP(uint8_t code, IRMP_DATA* irmp_p){
  uint8_t protIndex=0;
  uint8_t runner=0;
  while(code>=(runner + remoteSizes[protIndex])) {
    if(protIndex>=noOfRemotes) return false;
    runner+=remoteSizes[protIndex++];
  }
  uint8_t cmdIndex=code-runner;
  irmp_p->protocol=remoteProtocols[protIndex];
  irmp_p->address=remoteAddresses[protIndex];
  irmp_p->flags=remoteRepeats[protIndex];
  switch(protIndex) {
    case 0: irmp_p->command=amCommands[cmdIndex]; break;
    case 1: irmp_p->command=prCommands[cmdIndex]; break;
    case 2: irmp_p->command=boxCommands[cmdIndex]; break;
    case 3: irmp_p->command=scrCommands[cmdIndex]; break;
    case 4:
    case 5: irmp_p->command=(irmp_p->address & 0x3ff) << 6;
      irmp_p->address>>=10;
      irmp_p->command |= (protIndex==3 ? SW1_GROUP : SW2_GROUP);
      irmp_p->command |= (cmdIndex << 4);
      break;
    default: return false;
  }
  return true;  
}

void setup() {
  Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
  delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
  irsnd_init();
  irmp_irsnd_LEDFeedback(true); // Enable send signal feedback at LED_BUILTIN
  writeAvailableFunctions();
}

void loop() {
  uint8_t cmd=readChar();
  if(cmd==0xff) {
    writeAvailableFunctions();
    return;
  }
  if(!charToIRMP(cmd,&irsnd_data)) {
    return;
  }
  irsnd_send_data(&irsnd_data, true);
  if(irsnd_data.protocol==RF_AC104_PROTOCOL) {
    irsnd_send_data(&irsnd_ac104_after, true);
  }
  irsnd_data_print(&Serial, &irsnd_data);  
}
