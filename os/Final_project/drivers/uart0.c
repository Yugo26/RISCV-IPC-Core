// uart0 in Qemu driver
// interrupt rx and tx polling
// so driver must implement 
// 1. rxReady for rx interrupt
// 2. write for tx polling send a string
// 3. open 
// 4. no read, close, init

#include "os.h"

/*
 * The UART control registers are memory-mapped at address UART0. 
 * This macro returns the address of one of the registers.
 */
#define UART_REG(reg) ((volatile uint8_t *)(UART0 + reg))

/*
 * Reference
 * [1]: TECHNICAL DATA ON 16550, http://byterunner.com/16550.html
 */

/*
 * UART control registers map. see [1] "PROGRAMMING TABLE"
 * note some are reused by multiple functions
 * 0 (write mode): THR/DLL
 * 1 (write mode): IER/DLM
 */
#define RHR 0	// Receive Holding Register (read mode)
#define THR 0	// Transmit Holding Register (write mode)
#define DLL 0	// LSB of Divisor Latch (write mode)
#define IER 1	// Interrupt Enable Register (write mode)
#define DLM 1	// MSB of Divisor Latch (write mode)
#define FCR 2	// FIFO Control Register (write mode)
#define ISR 2	// Interrupt Status Register (read mode)
#define LCR 3	// Line Control Register
#define MCR 4	// Modem Control Register
#define LSR 5	// Line Status Register
#define MSR 6	// Modem Status Register
#define SPR 7	// ScratchPad Register

/*
 * POWER UP DEFAULTS
 * IER = 0: TX/RX holding register interrupts are both disabled
 * ISR = 1: no interrupt penting
 * LCR = 0
 * MCR = 0
 * LSR = 60 HEX
 * MSR = BITS 0-3 = 0, BITS 4-7 = inputs
 * FCR = 0
 * TX = High
 * OP1 = High
 * OP2 = High
 * RTS = High
 * DTR = High
 * RXRDY = High
 * TXRDY = Low
 * INT = Low
 */

/*
 * LINE STATUS REGISTER (LSR)
 * LSR BIT 0:
 * 0 = no data in receive holding register or FIFO.
 * 1 = data has been receive and saved in the receive holding register or FIFO.
 * ......
 * LSR BIT 5:
 * 0 = transmit holding register is full. 16550 will not accept any data for transmission.
 * 1 = transmitter hold register (or FIFO) is empty. CPU can load the next character.
 * ......
 */
#define LSR_RX_READY (1 << 0)
#define LSR_TX_IDLE  (1 << 5)

#define uart_read_reg(reg) (*(UART_REG(reg)))
#define uart_write_reg(reg, v) (*(UART_REG(reg)) = (v))

#define CONSOLE_PORT_NAME       "uart0"
/* Semaphore used to receive messages */
static err_t rx_sem;
static deviceCB_t *serial;

/*
hardware low level drivers implementations corresponding to deviceCB_t
device call back function
    err_t (*rxReady)(size_t size);
    err_t (*txComplete)(void *buffer);
    err_t  (*init)   ();
    err_t  (*open)   ();
    err_t  (*close)  ();
    size_t (*read)   (int pos, void *buffer, size_t size);
    size_t (*write)  (int pos, const void *buffer, size_t size);
    err_t  (*control)(int cmd, void *args);
*/

static err_t uart0_init() {
	uart_write_reg(IER, 0x00);

	uint8_t lcr = uart_read_reg(LCR);
	uart_write_reg(LCR, lcr | (1 << 7));
	uart_write_reg(DLL, 0x03);
	uart_write_reg(DLM, 0x00);

	lcr = 0;
	uart_write_reg(LCR, lcr | (3 << 0));

    /*
	 * enable receive interrupts.
	 */
	uint8_t ier = uart_read_reg(IER);
	uart_write_reg(IER, ier | (1 << 0));    
    return E_DEV_OK;
}

/*
    we can define an empty function or set NULL in corresponding 
    members of deviceCB_t.
    Here we define empty functions.
*/
static err_t uart0_open() {
    return E_DEV_OK;
};

static err_t uart0_close(){
    return E_DEV_OK;
};

static size_t uart0_read(int pos, void *buffer, size_t size){
    return 0;
};

static err_t uart0_control(int cmd, void *args){
    return E_DEV_OK;
};

/**********/

// uart0_putc call by uart0_write
static int uart0_putc(char ch)
{
	while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0);
	return uart_write_reg(THR, ch);
}

/* transmition data for driver write function*/
static size_t uart0_write(int pos, const void *buffer, size_t size) {
    char *s = (char *)buffer;
	while (*s) {
		uart0_putc(*s++);
	}
    return size;
}


/* Receive data callback function rxReady */
static err_t uart0_rxReady(size_t size)
{
    char rxChar = -1;
    if (uart_read_reg(LSR) & LSR_RX_READY)
		rxChar = uart_read_reg(RHR);
    sem_release((uint16_t)rx_sem);
    return rxChar;
}

//upper layer request uart0 
// first send and waiting rxdata
err_t waitRxData(int timeout) {
    return sem_take(rx_sem, timeout);
}


/*
 * handle a uart interrupt, raised because input has arrived, called from exteralInterrrupt().
 */
void uart0_isr(void)
{
    if (uart_read_reg(LSR) & LSR_RX_READY) {
        char rxChar = (char)serial->rxReady(1);
        //process rx print or store in a buffer
        //we can store rxChar in a global buffer
        //then access by pos offset
        //here we only send it back no process anymore
        char buffer[2]={0};
        if (rxChar == '\r')
            buffer[0]='\n';
        else 
            buffer[0]=rxChar;
        serial->write(0,buffer, 1); 
    } else {
        char msg[]="only suppose rx interrupt\n";
        serial->write(0, msg, 25);
    }
}

deviceCB_t * serial_init() {
    rx_sem = createSem(1,1,PRIO);
    if (rx_sem<0) {
        return NULL;
    }

    serial = device_create(Device_Class_Char,CONSOLE_PORT_NAME);
    //set up deviceCB_t data
    serial->init = uart0_init;
    serial->open = uart0_open; // or NULL
    serial->close = uart0_close; // or NULL
    serial->read = uart0_read; // or NULL
    serial->write = uart0_write;
    serial->control = uart0_control; // or NULL
    device_set_rxReady(serial, uart0_rxReady);   

    err_t regResult = device_register(serial, CONSOLE_PORT_NAME, 
                      FLAG_RDWR|FLAG_DEACTIVATE);
    if (regResult != E_DEV_OK) {
        return NULL;
    }

    /* register ISR to ISRTbl*/
	isrRegister(UART0_IRQ, uart0_isr);
    return serial;
}