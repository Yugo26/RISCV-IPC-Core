#include "os.h"

static deviceCB_t      deviceList;

void deviceList_init() {
    list_init((list_t*)&deviceList);
}

err_t device_register(deviceCB_t *dev, const char *name, uint16_t flags)
{
    if (device_find(name) != NULL)
        return E_DEV_FAIL;
 
    dev->deviceFlag = flags;
    dev->refCount = 0;
    dev->openFlag = 0;
    memcpy(dev->name,name,sizeof(dev->name));
    dev->name[sizeof(dev->name)-1]='\0';
    list_init((list_t*)&dev->node);
    reg_t lock_status = baseLock();
    list_insert_after((list_t*)&deviceList, (list_t*)dev);
    baseUnLock(lock_status);
    return E_DEV_OK;
}

/**
 * This function removes a previously registered device driver
 */
err_t device_unregister(deviceCB_t *dev)
{
    //remove from deviceList
    reg_t lock_status = baseLock();
    list_remove((list_t*)dev);
    baseUnLock(lock_status);
    return E_DEV_OK;
}


/**
 * This function finds a device driver by specified name.
 */
deviceCB_t * device_find(const char *name)
{
    list_t *pdevNode = deviceList.node.next;
    reg_t lock_status = baseLock();
    while(pdevNode != (list_t*)&deviceList){
        deviceCB_t *pdev = (deviceCB_t*)pdevNode;
        if (strcmp(pdev->name, name)==0) {
            return pdev;
        }
        pdevNode = pdevNode->next;
    }
    baseUnLock(lock_status);
    return NULL;
}

deviceCB_t * device_create(int type, char *name)
{
    int size;
    deviceCB_t *device;

    size = sizeof(deviceCB_t);
    device = (deviceCB_t*)malloc(size);
    if (device)
    {
        memset(device, 0x0, size);
        device->type = (enum deviceClass)type;
    }

    return device;
}

/**
 * This function destroy the specific device object.
 *
 * @param dev, the specific device object.
 */
void device_free(deviceCB_t *dev)
{
    //remove from deviceList;
    device_unregister(dev);
    /* release this device object */
    free(dev);
}


/**
 * This function will initialize the specified device
 * @return the result
 */
err_t device_init(deviceCB_t *dev)
{
    err_t result = E_DEV_OK;
    /* get device_init handler */
    if (dev->init)
    {
        if (!(dev->deviceFlag & FLAG_ACTIVATED))
        {
            result = dev->init();
            if (result != E_DEV_OK)
                DEBUG("To initialize device:%s failed. The error code is %d\n",
                           dev->name, result);
            else
                dev->deviceFlag |= FLAG_ACTIVATED;
        }
    }
    return result;
}

/**
 * This function will open a device
 */
err_t device_open(deviceCB_t *dev, uint16_t oflag)
{
    err_t result = E_DEV_OK;

    /* if device is not initialized, initialize it. */
    if (!(dev->deviceFlag & FLAG_ACTIVATED))
    {
        if (dev->init)
        {
            result = dev->init();
            if (result != E_DEV_OK)
            {
                DEBUG("To initialize device:%s failed. The error code is %d\n",
                            dev->name, result);
                    return result;
            }
        }

        dev->deviceFlag |= FLAG_ACTIVATED;
    }

    /* device is a stand alone(only one task ref) device and opened */
    if ((dev->deviceFlag & FLAG_STANDALONE) &&
        (dev->openFlag & OFLAG_OPEN))
        return E_DEV_BUSY;

    /* call device_open interface */
    if (dev->open)
        result = dev->open(oflag);
    else
        /* set open flag */
        dev->openFlag = (oflag & OFLAG_MASK);

    /* set open flag  if initial OK*/
    if (result == E_DEV_OK)
    {
        dev->openFlag |= OFLAG_OPEN;

        dev->refCount++;
        if (dev->refCount==0) //if too big to zero
            DEBUG("too many task ref\n");
        result = E_DEV_FAIL;
    }

    return result;
}

/**
 * This function will close a device
 */
err_t device_close(deviceCB_t *dev)
{
    err_t result=E_DEV_OK;

    if (dev->refCount == 0)
        return E_DEV_FAIL;

    dev->refCount--;

    if (dev->refCount != 0)
        return E_DEV_OK;

    if (dev->close)
        /* call device_close interface */
        result = dev->close();

    /* set open flag */
    if (result == E_DEV_OK)
        dev->openFlag = OFLAG_CLOSE;

    return result;
}

/**
 * This function will read some data from a device.
* return the actually read size on successful, otherwise negative returned.
 */
size_t device_read(deviceCB_t *dev,
                         uint16_t    pos,
                         void       *buffer,
                         size_t   size)
{
    if (dev->refCount == 0)
        return 0;
    if (dev->read)
        return dev->read(pos, buffer, size);
    return E_DEV_FAIL;
}

/**
 * This function will write some data to a device.
 * return the actually written size on successful, otherwise negative returned.
 */
size_t device_write(deviceCB_t *dev,
                          uint16_t    pos,
                          const void *buffer,
                          size_t   size)
{
 
    if (dev->refCount == 0)
        return 0;

    if (dev->write)
        /* call device_write interface */
        return dev->write(pos, buffer, size);
    return E_DEV_FAIL;
}

/**
 * This function will perform a variety of control functions on devices.
 */
err_t device_control(deviceCB_t *dev, int cmd, void *arg)
{
    /* call device_write interface */
    if (dev->control)
        return dev->control(cmd, arg);
    return E_DEV_FAIL;
}

/**
 * This function will set the reception indication callback function. This callback function
 * is invoked when this device receives data.
 */
err_t
device_set_rxReady(deviceCB_t *dev,
                          err_t (*rx_ind)(size_t size))
{
    dev->rxReady = rx_ind;
    return E_DEV_OK;
}

/**
 * This function will set the indication callback function when device has
 * written data to physical hardware.
 */
err_t
device_set_txComplete(deviceCB_t *dev,
                          err_t (*tx_done)(void *buffer))
{
    dev->txComplete = tx_done;
    return E_DEV_OK;
}