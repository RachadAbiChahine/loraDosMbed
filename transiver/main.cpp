#include "main.h"
/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   0

/* Set this flag to '1' to use the LoRa modulation or to '0' to use FSK modulation */
#if( defined ( TARGET_KL25Z ) || defined ( TARGET_LPC11U6X ) )
DigitalOut led(LED2);
#else
DigitalOut led(LED4

        );
#endif
/*###################"Global Variable##########*/
/*
 *  Global variables declarations
 */


volatile AppStates_t State = LOWPOWER;

SX1276MB1xAS Radio(NULL);


const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;


int main() {
    uint8_t i;
    Serial pc(SERIAL_TX, SERIAL_RX);
    pc.baud(38400);
    debug("\n\n\r     SX1276 Ping Pong Demo Application \n\n\r");


    radio_initializes(&RadioEvents);
    // verify the connection with the board
    while (Radio.Read(REG_VERSION) == 0x00) {
        debug("Radio could not be detected!\n\r", NULL);
        wait(1);
    }

    debug_if((DEBUG_MESSAGE & (Radio.DetectBoardType() == SX1276MB1LAS)), "\n\r > Board Type: SX1276MB1LAS < \n\r");
    debug_if((DEBUG_MESSAGE & (Radio.DetectBoardType() == SX1276MB1MAS)), "\n\r > Board Type: SX1276MB1MAS < \n\r");

    Radio.SetChannel(RF_FREQUENCY);

#if USE_MODEM_LORA == 1

    debug_if(LORA_FHSS_ENABLED, "\n\n\r             > LORA FHSS Mode < \n\n\r");
    debug_if(!LORA_FHSS_ENABLED, "\n\n\r             > LORA Mode < \n\n\r");

    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
            LORA_SPREADING_FACTOR, LORA_CODINGRATE,
            LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP,
            LORA_IQ_INVERSION_ON, 2000000);


#elif USE_MODEM_FSK == 1

    debug("\n\n\r              > FSK Mode < \n\n\r");
    Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
            FSK_DATARATE, 0,
            FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
            FSK_CRC_ENABLED, 0, 0, 0, 2000000);

    Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
            0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
            0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, FSK_CRC_ENABLED,
            0, 0, false, true);

#else

#error "Please define a modem in the compiler options."

#endif

    debug_if(DEBUG_MESSAGE, "Start sending packets\r\n");

    led = 0;
    uint8_t j = 0;
    uint16_t cmp = 0;

    while (1) {
        strcpy((char*) Buffer, (char*) PongMsg);
        // We fill the buffer with numbers for the payload
        for (i = 5; i < BufferSize; i++) {
            Buffer[i] = i - 4;
        }
        Buffer[4] = j++;
        wait_ms(200);
        Radio.Send(Buffer, BufferSize);
        pc.printf("%d sent ->%d\r\n", cmp,Buffer[4]  );
        led = !led;
        cmp ++;
    }
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

void radio_initializes(RadioEvents_t* RadioEvents) {
    // Initialize Radio driver
    RadioEvents->TxDone = OnTxDone;
    RadioEvents->RxDone = OnRxDone;
    RadioEvents->RxError = OnRxError;
    RadioEvents->TxTimeout = OnTxTimeout;
    RadioEvents->RxTimeout = OnRxTimeout;
    Radio.Init(RadioEvents);
}