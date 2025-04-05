#define CS_NAME "EverFocus Gateway"
#define DEBUG

//DCCEX Interface
#define DCCEX                  
#define DCCEX_LOG Serial
#define DCCEX_STREAM Serial1   

//LocoNet Interface
#define LOCONET                
#ifdef ESP8266
  #define LOCONET_RX 12
  #define LOCONET_TX 13
#endif
#ifdef __AVR__
  #define LOCONET_TX 7
#endif

//XpressNet Interface
#define XPRESSNET   
#define XNetAddress 30    //Adresse im XpressNet
#define XNetSRPin 9       //Max485 Busdriver Send/Receive-PIN
#define XNetStatusLed 13

