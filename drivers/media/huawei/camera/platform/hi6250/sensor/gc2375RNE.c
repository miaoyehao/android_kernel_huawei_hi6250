 

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <securec.h>

#include "hwsensor.h"
#include "sensor_commom.h"
#include "hw_csi.h"

#define I2S(i) container_of(i, sensor_t, intf)
#define Sensor2Pdev(s) container_of((s).dev, struct platform_device, dev)
#define POWER_DELAY_0        0//delay 0 ms
#define POWER_DELAY_1        1//delay 1 ms
#define POWER_DELAY_5        5//delay 5 ms

static hwsensor_vtbl_t s_gc2375RNE_vtbl;
static bool power_on_status = false;//false: power off, true:power on
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -save -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715 -specific(-e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 -e715)*/

int gc2375RNE_config(hwsensor_intf_t* si, void  *argp);

static struct platform_device *s_pdev = NULL;
static sensor_t *s_sensor = NULL;

struct sensor_power_setting gc2375RNE_power_up_setting[] = {

    //init MCAM1 PWDN [GPIO58]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_5,
    },

    //enable MIPI [GPIO-010]
    {
        .seq_type     = SENSOR_MIPI_EN,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //disable SCAM1 reset [GPIO182]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable SCAM1 PWDN [GPIO181]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable SCAM0 reset [GPIO17]
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM1 IOVDD 1.80V [LDO33]
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"back-slave-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },

    //MCAM1 AVDD 2.80V [LDO13]
    {
       .seq_type = SENSOR_AVDD,
       .data = (void*)"back-slave-sensor-avdd",
       .config_val = LDO_VOLTAGE_V2P8V,
       .sensor_index = SENSOR_INDEX_INVALID,
       .delay = POWER_DELAY_0,
    },

    //MIPI Swith to SCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM1 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //MCAM1 PWDN [GPIO58]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //MCAM1 RST [GPIO179]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
};

struct sensor_power_setting gc2375RNE_power_down_setting[] = {
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //MIPI Swith to MCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM1 AVDD 2.80V
    {
       .seq_type = SENSOR_AVDD,
       .data = (void*)"back-slave-sensor-avdd",
       .config_val = LDO_VOLTAGE_V2P8V,
       .sensor_index = SENSOR_INDEX_INVALID,
       .delay = POWER_DELAY_1,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"back-slave-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //disable SCAM0 reset
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 PWDN
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 reset
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable MIPI [GPIO-010]
    {
        .seq_type     = SENSOR_MIPI_EN,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

};

struct sensor_power_setting gc2375RNE_v4_power_up_setting[] = {

    //init MCAM1 PWDN [GPIO58]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_5,
    },

    //MCAM0 AFVDD & enable MIPI 3V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //disable SCAM1 reset [GPIO182]
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable SCAM1 PWDN [GPIO181]
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //disable SCAM0 reset [GPIO17]
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM1 IOVDD 1.80V [LDO33]
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"back-slave-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },

    //MCAM1 AVDD 2.80V [LDO13]
    {
       .seq_type = SENSOR_AVDD,
       .data = (void*)"back-slave-sensor-avdd",
       .config_val = LDO_VOLTAGE_V2P8V,
       .sensor_index = SENSOR_INDEX_INVALID,
       .delay = POWER_DELAY_0,
    },

    //MIPI Swith to SCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },

    //MCAM1 CLK
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //MCAM1 PWDN [GPIO58]
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //MCAM1 RST [GPIO179]
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
};

struct sensor_power_setting gc2375RNE_v4_power_down_setting[] = {
    {
        .seq_type = SENSOR_PWDN,
        .config_val = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    {
        .seq_type = SENSOR_RST,
        .config_val = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    {
        .seq_type = SENSOR_MCLK,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //MIPI Swith to MCAM [GPIO-009]
    {
        .seq_type     = SENSOR_MIPI_SW,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
    //MCAM1 AVDD 2.80V
    {
       .seq_type = SENSOR_AVDD,
       .data = (void*)"back-slave-sensor-avdd",
       .config_val = LDO_VOLTAGE_V2P8V,
       .sensor_index = SENSOR_INDEX_INVALID,
       .delay = POWER_DELAY_1,
    },
    //MCAM1 IOVDD 1.80V
    {
        .seq_type = SENSOR_IOVDD,
        .data = (void*)"back-slave-sensor-iovdd",
        .config_val = LDO_VOLTAGE_1P8V,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay = POWER_DELAY_1,
    },
    //disable SCAM0 reset
    {
        .seq_type     = SENSOR_SUSPEND,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 PWDN
    {
        .seq_type     = SENSOR_PWDN2,
        .config_val   = SENSOR_GPIO_HIGH,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },
    //disable SCAM1 reset
    {
        .seq_type     = SENSOR_SUSPEND2,
        .config_val   = SENSOR_GPIO_LOW,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = 0,
    },

    //MCAM0 AFVDD & disable MIPI 3V [LDO25]
    {
        .seq_type     = SENSOR_VCM_AVDD,
        .data         = (void*)"cameravcm-vcc",
        .config_val   = LDO_VOLTAGE_V3PV,
        .sensor_index = SENSOR_INDEX_INVALID,
        .delay        = POWER_DELAY_1,
    },
};

atomic_t volatile gc2375RNE_powered = ATOMIC_INIT(0);
struct mutex gc2375RNE_power_lock;
static sensor_t s_gc2375RNE =
{
    .intf = { .vtbl = &s_gc2375RNE_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(gc2375RNE_power_up_setting),
            .power_setting = gc2375RNE_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(gc2375RNE_power_down_setting),
            .power_setting = gc2375RNE_power_down_setting,
    },
     .p_atpowercnt = &gc2375RNE_powered,
};

static sensor_t s_gc2375RNE_v4 =
{
    .intf = { .vtbl = &s_gc2375RNE_vtbl, },
    .power_setting_array = {
            .size = ARRAY_SIZE(gc2375RNE_v4_power_up_setting),
            .power_setting = gc2375RNE_v4_power_up_setting,
     },
    .power_down_setting_array = {
            .size = ARRAY_SIZE(gc2375RNE_v4_power_down_setting),
            .power_setting = gc2375RNE_v4_power_down_setting,
    },
     .p_atpowercnt = &gc2375RNE_powered,
};

static const struct of_device_id s_gc2375RNE_dt_match[] =
{
    {
        .compatible = "huawei,gc2375RNE",
        .data = &s_gc2375RNE.intf,
    },
    {
        .compatible = "huawei,gc2375RNE_v4",
        .data = &s_gc2375RNE_v4.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_gc2375RNE_dt_match);

static struct platform_driver s_gc2375RNE_driver =
{
    .driver =
    {
        .name = "huawei,gc2375RNE",
        .owner = THIS_MODULE,
        .of_match_table = s_gc2375RNE_dt_match,
    },
};

char const* gc2375RNE_get_name(hwsensor_intf_t* si)
{
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return NULL;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return NULL;
    }
    return sensor->board_info->name;
}

int gc2375RNE_power_up(hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }
    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);

    ret = hw_sensor_power_up_config(sensor->dev, sensor->board_info);
    if (0 == ret ){
        cam_info("%s. power up config success.", __func__);
    }else{
        cam_err("%s. power up config fail.", __func__);
        return ret;
    }
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_on(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_up(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power up sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power up sensor fail.", __func__);
    }
    return ret;
}

int
gc2375RNE_power_down(
        hwsensor_intf_t* si)
{
    int ret = 0;
    sensor_t* sensor = NULL;

    if (!si){
        cam_err("%s. si is null.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info->name is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("enter %s. index = %d name = %s", __func__, sensor->board_info->sensor_index, sensor->board_info->name);
    if (hw_is_fpga_board()) {
        ret = do_sensor_power_off(sensor->board_info->sensor_index, sensor->board_info->name);
    } else {
        ret = hw_sensor_power_down(sensor);
    }
    if (0 == ret )
    {
        cam_info("%s. power down sensor success.", __func__);
    }
    else
    {
        cam_err("%s. power down sensor fail.", __func__);
    }
    hw_sensor_power_down_config(sensor->board_info);
    return ret;
}

int gc2375RNE_csi_enable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

int gc2375RNE_csi_disable(hwsensor_intf_t* si)
{
    (void)si;
    return 0;
}

static int gc2375RNE_match_id(
        hwsensor_intf_t* si, void * data)
{
    sensor_t* sensor = NULL;
    struct sensor_cfg_data *cdata = NULL;
    int error = 0;
    cam_info("%s enter.", __func__);
    if(NULL == si || NULL == data)
    {
        cam_err("%s. si or data is NULL.", __func__);
        return -1;
    }

    sensor = I2S(si);
    if (NULL == sensor || NULL == sensor->board_info || NULL == sensor->board_info->name) {
        cam_err("%s. sensor or board_info is NULL.", __func__);
        return -1;
    }
    cdata = (struct sensor_cfg_data *)data;

    error = memset_s(cdata->cfg.name, DEVICE_NAME_SIZE, 0, DEVICE_NAME_SIZE);
    if (error != EOK) {
        cam_err("%s. failed to memset_s write_data.", __func__);
    }
    strncpy_s(cdata->cfg.name, DEVICE_NAME_SIZE, sensor->board_info->name, DEVICE_NAME_SIZE - 1);
    cdata->data = sensor->board_info->sensor_index;

    return 0;
}

static hwsensor_vtbl_t s_gc2375RNE_vtbl =
{
    .get_name = gc2375RNE_get_name,
    .config = gc2375RNE_config,
    .power_up = gc2375RNE_power_up,
    .power_down = gc2375RNE_power_down,
    .match_id = gc2375RNE_match_id,
    .csi_enable = gc2375RNE_csi_enable,
    .csi_disable = gc2375RNE_csi_disable,
};

int gc2375RNE_config(hwsensor_intf_t* si, void  *argp)
{
    struct sensor_cfg_data *data = NULL;

    int ret = 0;

    if (NULL == si || NULL == argp){
        cam_err("%s : si or argp is null", __func__);
        return -1;
    }

    data = (struct sensor_cfg_data *)argp;
    cam_debug("gc2375RNE cfgtype = %d",data->cfgtype);
    switch(data->cfgtype){
        case SEN_CONFIG_POWER_ON:
            mutex_lock(&gc2375RNE_power_lock);
            if(false == power_on_status){
                if(NULL == si->vtbl->power_up){
                    cam_err("%s. si->vtbl->power_up is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_up(si);
                    if (0 == ret) {
                        power_on_status = true;
                    } else {
                        cam_err("%s. power up fail.", __func__);
                    }
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&gc2375RNE_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_POWER_OFF:
            mutex_lock(&gc2375RNE_power_lock);
            if(true == power_on_status){
                if(NULL == si->vtbl->power_down){
                    cam_err("%s. si->vtbl->power_down is null.", __func__);
                    ret=-EINVAL;
                }else{
                    ret = si->vtbl->power_down(si);
                    if (0 != ret) {
                        cam_err("%s. power_down fail.", __func__);
                    }
                    power_on_status = false;
                }
            }
            /*lint -e455 -esym(455,*)*/
            mutex_unlock(&gc2375RNE_power_lock);
            /*lint -e455 +esym(455,*)*/
            break;
        case SEN_CONFIG_WRITE_REG:
            break;
        case SEN_CONFIG_READ_REG:
            break;
        case SEN_CONFIG_WRITE_REG_SETTINGS:
            break;
        case SEN_CONFIG_READ_REG_SETTINGS:
            break;
        case SEN_CONFIG_ENABLE_CSI:
            break;
        case SEN_CONFIG_DISABLE_CSI:
            break;
        case SEN_CONFIG_MATCH_ID:
            if(NULL == si->vtbl->match_id){
                cam_err("%s. si->vtbl->match_id is null.", __func__);
                ret=-EINVAL;
            }else{
                ret = si->vtbl->match_id(si,argp);
            }
            break;
        case SEN_CONFIG_RESET_HOLD:
            cam_warn("%s cfgtype(%d) does not support", __func__, data->cfgtype);
            break;
        case SEN_CONFIG_RESET_RELEASE:
            cam_warn("%s cfgtype(%d) does not support", __func__, data->cfgtype);
            break;
        default:
            cam_err("%s cfgtype(%d) is error", __func__, data->cfgtype);
            break;
    }

    return ret;
}

static int32_t gc2375RNE_platform_probe(struct platform_device* pdev)
{
    int rc = 0;
    const struct of_device_id *id = NULL;
    hwsensor_intf_t *intf = NULL;
    sensor_t *sensor = NULL;
    struct device_node *np = NULL;

    if (NULL == pdev){
        cam_err("%s pdev is null.\n", __func__);
        return -1;
    }

    cam_notice("enter %s",__func__);

    np = pdev->dev.of_node;
    if (NULL == np) {
        cam_err("%s of_node is NULL", __func__);
        return -ENODEV;
    }

    id = of_match_node(s_gc2375RNE_dt_match, np);
    if (!id) {
        cam_err("%s none id matched", __func__);
        return -ENODEV;
    }

    intf = (hwsensor_intf_t*)id->data;
    if (NULL == intf) {
        cam_err("%s intf is NULL", __func__);
        return -ENODEV;
    }

    sensor = I2S(intf);
    if(NULL == sensor){
        cam_err("%s sensor is NULL rc %d", __func__, rc);
        return -ENODEV;
    }
    rc = hw_sensor_get_dt_data(pdev, sensor);
    if (rc < 0) {
        cam_err("%s no dt data rc %d", __func__, rc);
        return -ENODEV;
    }

    mutex_init(&gc2375RNE_power_lock);
    sensor->dev = &pdev->dev;
    rc = hwsensor_register(pdev, intf);
    if (0 != rc){
        cam_err("%s hwsensor_register fail.\n", __func__);
        goto gc2375RNE_sensor_probe_fail;
    }
    s_pdev = pdev;
    rc = rpmsg_sensor_register(pdev, (void*)sensor);
    if (0 != rc){
        cam_err("%s rpmsg_sensor_register fail.\n", __func__);
        hwsensor_unregister(s_pdev);
        goto gc2375RNE_sensor_probe_fail;
    }
    s_sensor = sensor;

gc2375RNE_sensor_probe_fail:
    return rc;
}

static int __init gc2375RNE_init_module(void)
{
    cam_info("Enter: %s", __func__);
    return platform_driver_probe(&s_gc2375RNE_driver,
            gc2375RNE_platform_probe);
}

static void __exit gc2375RNE_exit_module(void)
{
    if( NULL != s_sensor)
    {
        rpmsg_sensor_unregister((void*)s_sensor);
        s_sensor = NULL;
    }
    if (NULL != s_pdev) {
        hwsensor_unregister(s_pdev);
        s_pdev = NULL;
    }
    platform_driver_unregister(&s_gc2375RNE_driver);
}
/*lint -restore*/
/*lint -e826 -e31 -e485 -e785 -e731 -e846 -e514 -e866 -e30 -e84 -e838 -e64 -e528 -e753 -e749 +esym(826, 31, 485, 785, 731, 846, 514, 866, 30, 84, 838, 64, 528, 753, 749, 715*)*/
/*lint -e528 -esym(528,*)*/
module_init(gc2375RNE_init_module);
module_exit(gc2375RNE_exit_module);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_DESCRIPTION("gc2375RNE");
MODULE_LICENSE("GPL v2");
/*lint -e753 +esym(753,*)*/
