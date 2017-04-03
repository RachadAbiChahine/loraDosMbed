
#include "main.h"


/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   0
#define DEBUG_MESSAGE1   (DEBUG_MESSAGE||0)
/*
 *  Global variables declarations
 */

DigitalOut led(LED1);

volatile AppStates_t State = RX;

SX1276MB1xAS Radio(NULL);

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

int main()
{
    Serial pc(USBTX, USBRX);
    pc.baud(38400);
    debug("\n\n\r     SX1276 Ping Pong Demo Application \n\n\r");

    radio_initializes(&RadioEvents);
    // verify the connection with the board
    while (Radio.Read(REG_VERSION) == 0x00)
    {
        debug("Radio could not be detected!\n\r", NULL);
        wait(1);
    }

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
    while (1)
    {
        wait_ms(20);
        debug_if(DEBUG_MESSAGE, "BeforeRXn\r\n\r");
        Radio.Rx(RX_TIMEOUT_VALUE);
        debug_if(DEBUG_MESSAGE, "afterRX\n\r\n\r");
        led = !led;
        if (BufferSize > 0)
        {

            wait_ms(120);
            debug_if(DEBUG_MESSAGE, "Before if strcmp\n\r\n\r");
            if (strncmp((const char*) (Buffer + 4), (const char*) value_tester, 4) != 0)
            {
                recived++;
#if DEBUG_MESSAGE1 ==1

                pc.printf("BUFFER/ [ ");
                pc.printf("%c  ", Buffer[0]);
                pc.printf("%c  ", Buffer[1]);
                pc.printf("%c  ", Buffer[2]);
                pc.printf("%c  ", Buffer[3]);
#endif
                for (uint8_t ct = 4; ct <= 8; ct++)
                {
#if DEBUG_MESSAGE1 ==1
                    pc.printf("%d ", Buffer[ct]);
#endif
                    value_tester[ct - 4] = Buffer[ct];
                }
#if DEBUG_MESSAGE1 ==1
                if (DEBUG_MESSAGE1) pc.printf("\n\r ");
#endif

                /**
                 * @warning printf cause freeze
                 */

#if DEBUG_MESSAGE1 ==1
                __disable_irq();
                debug_if(DEBUG_MESSAGE, "before printf\n\r\n\r");
                printf("[[[[%d]]]]\n\r", recived);
                wait_ms(20);
                __enable_irq();
                debug_if(DEBUG_MESSAGE, "after printf\n\r\n\r");
#endif
            }
        }

    }
    wait_ms(20);
    printf("%d\n\r", recived);

}

void OnTxDone(void)
{
    Radio.Sleep();
    State = TX;
    debug_if(DEBUG_MESSAGE, "> OnTxDone\n\r");
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep();
    BufferSize = size;
    memcpy(Buffer, payload, BufferSize);
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    //  debug_if(DEBUG_MESSAGE, "> OnRxDone\n\r");
}

void OnTxTimeout(void)
{
    Radio.Sleep();
    State = TX_TIMEOUT;
    debug_if(DEBUG_MESSAGE, "> OnTxTimeout\n\r");
}

void OnRxTimeout(void)
{
    Radio.Sleep();
    Buffer[ BufferSize ] = 0;
    State = RX_TIMEOUT;
    debug_if(DEBUG_MESSAGE, "> OnRxTimeout\n\r");
}

void OnRxError(void)
{
    Radio.Sleep();
    State = RX_ERROR;
    debug_if(DEBUG_MESSAGE, "> OnRxError\n\r");
}

void radio_initializes(RadioEvents_t* RadioEvents)
{
    // Initialize Radio driver
    RadioEvents->TxDone = OnTxDone;
    RadioEvents->RxDone = OnRxDone;
    RadioEvents->RxError = OnRxError;
    RadioEvents->TxTimeout = OnTxTimeout;
    RadioEvents->RxTimeout = OnRxTimeout;
    Radio.Init(RadioEvents);
}