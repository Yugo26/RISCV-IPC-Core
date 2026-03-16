#ifndef __DEVICES_H__
#define __DEVICES_H__
#include "types.h"

enum deviceClass
{
    Device_Class_Char = 0,                           /**< character device */
    Device_Class_Block,                              /**< block device */
    Device_Class_NetIf,                              /**< net interface */
    Device_Class_MTD,                                /**< memory device */
    Device_Class_CAN,                                /**< CAN device */
    Device_Class_RTC,                                /**< RTC device */
    Device_Class_Graphic,                            /**< Graphic device */
    Device_Class_I2CBUS,                             /**< I2C bus device */
    Device_Class_USBDevice,                          /**< USB slave device */
    Device_Class_USBHost,                            /**< USB host bus */
    Device_Class_SPIBUS,                             /**< SPI bus device */
    Device_Class_SPIDevice,                          /**< SPI device */
    Device_Class_SDIO,                               /**< SDIO bus device */
    Device_Class_Timer,                              /**< Timer device */
    Device_Class_Sensor,                             /**< Sensor device */
    Device_Class_Unknown                             /**< unknown device */
};

/**
 * device flags defitions
 */
/* first 4 bits*/
#define FLAG_DEACTIVATE       0x000           /**< device is not initialized */
#define FLAG_RDONLY           0x001           /**< read only */
#define FLAG_WRONLY           0x002           /**< write only */
#define FLAG_RDWR             0x003           /**< read and write */
#define FLAG_REMOVABLE        0x004           /**< removable device */
#define FLAG_STANDALONE       0x008           /**< standalone device */

/* middle 4 bits*/
#define FLAG_ACTIVATED        0x010           /**< device is activated */
#define FLAG_SUSPENDED        0x020           /**< device is suspended */

/* last 4 bits */
#define FLAG_INT_RX           0x100           /**< INT mode on Rx */
#define FLAG_DMA_RX           0x200           /**< DMA mode on Rx */
#define FLAG_INT_TX           0x400           /**< INT mode on Tx */
#define FLAG_DMA_TX           0x800           /**< DMA mode on Tx */


/* open flags */
#define OFLAG_CLOSE           0x00           /**< device is closed */
#define OFLAG_RDONLY          0x01           /**< read only access */
#define OFLAG_WRONLY          0x02           /**< write only access */
#define OFLAG_RDWR            0x03           /**< read and write */
#define OFLAG_OPEN            0x04           /**< device is opened */
#define OFLAG_MASK            0xF0F           /**< mask of open flag */
/**
 * general device commands
 */
#define CTRL_RESUME           0x01            /**< resume device */
#define CTRL_SUSPEND          0x02            /**< suspend device */
#define CTRL_CONFIG           0x03            /**< configure device */
#define CTRL_CLOSE            0x04            /**< close device */


typedef struct device deviceCB_t;

struct device
{
    list_t          node;
    uint8_t         device_id;                  /**< 0 - MAX_DEVICES-1 */
    char            name[10];
    enum deviceClass type;                    /**< device type */
    uint16_t        deviceFlag;              /**< device flag */
    uint16_t        openFlag;                /**< device open flag */

    uint8_t         refCount;                /**< reference count */

    /* device call back function*/
    err_t (*rxReady)(size_t size);
    err_t (*txComplete)(void *buffer);
    /* common device interface */
    err_t  (*init)   ();
    err_t  (*open)   ();
    err_t  (*close)  ();
    size_t (*read)   (int pos, void *buffer, size_t size);
    size_t (*write)  (int pos, const void *buffer, size_t size);
    err_t  (*control)(int cmd, void *args);

    void              *user_data;                /**< device private data */
};

#endif