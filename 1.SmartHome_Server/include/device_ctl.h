/*************************************************************************
	> File Name: device_ctl.h
	> Author: 
	> Mail: 
	> Created Time: Wed 06 Mar 2024 02:07:58 PM CST
 ************************************************************************/

#ifndef _DEVICE_CTL_H
#define _DEVICE_CTL_H
#include "datatype.h"

#define MAX_DEVICE_SUM 20

int change_device_state_to_off(struct device *dev);
int change_device_state_to_on(struct device *dev);
int change_device_state_to_standby(struct device *dev);
int (*change_device_states[_DEVICE_STATE_SUM])(struct device *dev);

int add_device_light(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
int add_device_switch(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
int add_device_thermostat(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
int (*add_devices[_DEVICE_TYPE_SUM])(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
#endif
