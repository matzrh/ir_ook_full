#define AM_NAME "Technics Amplifier"
#define AM_PROTOCOL  IRMP_KASEIKYO_PROTOCOL
#define AM_ADDRESS  0x2002
#define AM_REPEATS 3
#define AM_POWER  0xa3d0
#define AM_ON     0xa3e0
#define AM_OFF    0xa3f0
#define AM_TV     0xa9f0
#define AM_VCR    0xa9e0
#define AM_DVD    0xaa30
#define AM_MD     0xa930
#define AM_TUNER  0xa920
#define AM_TAPE   0xa960
#define AM_CD     0xa940
#define AM_SFC    0xa311
#define AM_SNDMD  0xab01
#define AM_MUTE   0xa320
#define AM_VUP      0xa200
#define AM_VDOWN      0xa210

#define PROJ_NAME "Epson Projector"
#define PROJ_PROTOCOL IRMP_NEC_PROTOCOL
#define PROJ_ADDRESS  0x5583
#define PROJ_REPEATS 3
#define PROJ_ON   0x90
#define PROJ_OFF  0x91

#define BOX_NAME "Epson Bridge"
#define BOX_PROTOCOL IRMP_NEC_PROTOCOL
#define BOX_ADDRESS   0x381
#define BOX_REPEATS   3
#define BOX_POWER     0x40
#define BOX_INPUT     0x41
#define BOX_OUTPUT    0x42

#define SCREEN_NAME "Projector Screen"
#define SCREEN_PROTOCOL RF_AC104_PROTOCOL
#define SCREEN_ADDRESS  0x0100
#define SCREEN_UP     0xb
#define SCREEN_DOWN   0x43
#define SCREEN_STOP   0x23
#define SCREEN_AFTER  0x24
//id defined



#define SW1_NAME "HomeEasy1"
#define SW1_PROTOCOL  RF_HME_PROTOCOL
#define SW1_ADDRESS   0x7D0
#define SW1_GROUP     0x1

#define SW2_NAME "HomeEasy2"
#define SW2_PROTOCOL  RF_HME_PROTOCOL
#define SW2_ADDRESS   0x7DA
#define SW2_GROUP     0x1

/*
 * The below definitions are pseude hash maps, so the order is important....
 */

const uint16_t amCommands[]={
  AM_POWER,
  AM_ON,
  AM_OFF,  
  AM_TV,   
  AM_VCR,  
  AM_DVD,  
  AM_MD,   
  AM_TUNER,
  AM_TAPE, 
  AM_CD,   
  AM_SFC,  
  AM_SNDMD,
  AM_MUTE,
  AM_VUP, 
  AM_VDOWN  
};
const uint8_t amSize=sizeof(amCommands) / 2;
const char* amNames[amSize] ={
  "power",
  "on",
  "off",
  "tv",
  "vcr",
  "dvd",
  "md",
  "tuner",
  "tape",
  "cd",
  "sfc",
  "sndmd",
  "mute",
  "volume+",
  "volume-"
};

const uint8_t prCommands[] = {PROJ_ON, PROJ_OFF};
const uint8_t prSize=sizeof(prCommands);
const char* prNames[prSize] = {"on","off"};

const uint8_t boxCommands[] = {BOX_POWER, BOX_INPUT, BOX_OUTPUT};
const uint8_t boxSize=sizeof(boxCommands);
const char* boxNames[boxSize] = {"power","input","output"};

const uint8_t scrCommands[] = {SCREEN_UP, SCREEN_DOWN, SCREEN_STOP};  //do not expose 'after'
const uint8_t scrSize=sizeof(scrCommands);
const char* scrNames[scrSize] = {"up", "down", "stop"};

const uint8_t noOfRemotes = 6;
const char* remoteNames[noOfRemotes] = {
  AM_NAME,
  PROJ_NAME,
  BOX_NAME,
  SCREEN_NAME,
  SW1_NAME,
  SW2_NAME
};
const char* remoteTypes[noOfRemotes] = {
  "enum",
  "enum",
  "enum",
  "enum",
  "bool",
  "bool"
};
const uint8_t remoteSizes[noOfRemotes] = {
  amSize,prSize,boxSize,scrSize,2,2
};

uint8_t remoteProtocols[] = {
  AM_PROTOCOL,
  PROJ_PROTOCOL,
  BOX_PROTOCOL,
  SCREEN_PROTOCOL,
  SW1_PROTOCOL,
  SW2_PROTOCOL
};

uint16_t remoteAddresses[] = {
  AM_ADDRESS,
  PROJ_ADDRESS,
  BOX_ADDRESS,
  SCREEN_ADDRESS,
  SW1_ADDRESS,
  SW2_ADDRESS
};

uint8_t remoteRepeats[] = {
  AM_REPEATS,
  PROJ_REPEATS,
  BOX_REPEATS,
  0,
  0,
  0
};
