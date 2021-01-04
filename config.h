// config: ////////////////////////////////////////////////////////////
// 
//#define BLUETOOTH
#define ONEWIRE
//#define OTA_HANDLER 
#define MODE_AP // phone connects directly to ESP


#define PROTOCOL_TCP

bool debug = true;

#define VERSION "1.20-huzzah32-1wire-V2"

// For AP mode:
const char *ssid = "GLIDER";  // You will connect your phone to this Access Point
const char *pw = "segelflyg";   // and this is the password, 
IPAddress ip(192, 168, 10, 1); // From app, connect to this IP
IPAddress netmask(255, 255, 255, 0);

// You must connect the phone to this AP, then:
// menu -> connect -> Internet(TCP) -> 192.168.10.1:4352  for UART0
//                                  -> 192.168.10.1:2000  for UART1
//                                  -> 192.168.10.1:4353  for UART2
// Försöker felsöka med att ha wifi & blåtand samtidigt.
// Huzzah32 med viss begränsningar beskrivna på adc2 och även dac1, flyttar portar och testar.




#define NUM_COM   3                 // total number of COM Ports
#define DEBUG_COM 0                 // debug output to COM0
/*************************  COM Port 0 *******************************/
#define UART_BAUD0 9600 
// Baudrate UART0
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop UART0
#define SERIAL0_RXPIN 21            // receive Pin UART0
#define SERIAL0_TXPIN 19            // transmit Pin UART0
#define SERIAL0_TCP_PORT 4352       // Wifi Port UART0
/*************************  COM Port 1 ****************************
***/
#define UART_BAUD1 4800             // Baudrate UART1
#define SERIAL_PARAM1 SERIAL_8N1    // Data/Parity/Stop UART1
#define SERIAL1_RXPIN 16            // receive Pin UART1
#define SERIAL1_TXPIN 17            // transmit Pin UART1
#define SERIAL1_TCP_PORT 2000       // Wifi Port UART1
/*************************  COM Port 2 *******************************/
#define UART_BAUD2 4800             // Baudrate UART2
#define SERIAL_PARAM2 SERIAL_8N1    // Data/Parity/Stop UART2
#define SERIAL2_RXPIN 18            // receive Pin UART2
#define SERIAL2_TXPIN 5             // transmit Pin UART2
#define SERIAL2_TCP_PORT 4353       // Wifi Port UART2

#define bufferSize 1024

//////////////////////////////////////////////////////////////////////////
