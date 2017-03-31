#include "mbed.h"
#include "main.h"
#include "sx1276-hal.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1

/* Set this flag to '1' to use the LoRa modulation or to '0' to use FSK modulation */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   !USE_MODEM_LORA

#define RF_FREQUENCY                                    868000000 // Hz
#define TX_OUTPUT_POWER                                 14     // 14 dBm

#if USE_MODEM_LORA == 1

#define LORA_BANDWIDTH                              1        // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
//  2: 4/6,
//  3: 4/7,
//  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_FHSS_ENABLED                           false
#define LORA_NB_SYMB_HOP                            4
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            true
#else
#error "Please define a modem in the compiler options."
#endif

#define RX_TIMEOUT_VALUE                                2000000   // in us
#define BUFFER_SIZE                                     32        // Define the payload size here

DigitalOut led(LED1);

/*
 *  Global variables declarations
 */
typedef enum {
    LOWPOWER = 0,
    IDLE,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
    CAD,
    CAD_DONE
} AppStates_t;

volatile AppStates_t State = RX;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1276MB1xAS Radio(NULL);

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

int main() {
    Serial pc(USBTX, USBRX);
    pc.baud(38400);
    debug("\n\n\r     SX1276 Ping Pong Demo Application \n\n\r");

    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxError = OnRxError;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    Radio.Init(&RadioEvents);

    // verify the connection with the board
    while (Radio.Read(REG_VERSION) == 0x00) {
        debug("Radio could not be detected!\n\r", NULL);
        wait(1);
    }

    debug_if((DEBUG_MESSAGE & (Radio.DetectBoardType() == SX1276MB1LAS)), "\n\r > Board Type: SX1276MB1LAS < \n\r");
    debug_if((DEBUG_MESSAGE & (Radio.DetectBoardType() == SX1276MB1MAS)), "\n\r > Board Type: SX1276MB1MAS < \n\r");

    Radio.SetChannel(RF_FREQUENCY);

#if USE_MODEM_LORA == 1

    debug_if(!LORA_FHSS_ENABLED, "\n\n\r             > LORA Mode < \n\n\r");

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
            LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
            LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
            LORA_IQ_INVERSION_ON, true);
#else
#error "Please define a modem in the compiler options."
#endif

    debug_if(DEBUG_MESSAGE, "Starting receive loop\r\n");
    led = 0;
    Radio.Rx(RX_TIMEOUT_VALUE);
    int recived = 0;
    uint8_t value_tester[4];
    while (1) {
        wait_ms(20);
        Radio.Rx(RX_TIMEOUT_VALUE);
        // led = !led;
        if (BufferSize > 0) {

            wait_ms(120);

            if (strncmp((const char*) (Buffer + 4), (const char*) value_tester, 4) != 0) {
                recived++;
                pc.printf("BUFFER/ [ ");
                pc.printf("%c  ", Buffer[0]);
                pc.printf("%c  ", Buffer[1]);
                pc.printf("%c  ", Buffer[2]);
                pc.printf("%c  ", Buffer[3]);

                for (uint8_t ct = 4; ct <= 8; ct++) {
                    pc.printf("%d ", Buffer[ct]);
                    value_tester[ct - 4] = Buffer[ct];
                }
                pc.printf("recived =%d ]\n\r", recived);
            }
        } else pc.printf("buffer <0");
    }
    pc.printf("error");
}

void OnTxDone(void) {
    Radio.Sleep();
    State = TX;
    debug_if(DEBUG_MESSAGE, "> OnTxDone\n\r");
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    Radio.Sleep();
    BufferSize = size;
    memcpy(Buffer, payload, BufferSize);
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    debug_if(DEBUG_MESSAGE, "> OnRxDone\n\r");
}

void OnTxTimeout(void) {
    Radio.Sleep();
    State = TX_TIMEOUT;
    debug_if(DEBUG_MESSAGE, "> OnTxTimeout\n\r");
}

void OnRxTimeout(void) {
    Radio.Sleep();
    Buffer[ BufferSize ] = 0;
    State = RX_TIMEOUT;
    debug_if(DEBUG_MESSAGE, "> OnRxTimeout\n\r");
}

void OnRxError(void) {
    Radio.Sleep();
    State = RX_ERROR;
    debug_if(DEBUG_MESSAGE, "> OnRxError\n\r");
}
