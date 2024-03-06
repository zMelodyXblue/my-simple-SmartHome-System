/*************************************************************************
	> File Name: device_ctl.c
	> Author: 
	> Mail: 
	> Created Time: Wed 06 Mar 2024 02:13:25 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <device_ctl.h>

int (*change_device_states[_DEVICE_STATE_SUM])(struct device *dev) = {change_device_state_to_off, change_device_state_to_on, change_device_state_to_standby};
int (*add_devices[_DEVICE_TYPE_SUM])(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name) = {add_device_light, add_device_switch, add_device_thermostat};

int change_device_state_to_off(struct device *dev) {
    //take_actions

    dev->state = DEVICE_STATE_OFF;
    return 0;
}

int change_device_state_to_on(struct device *dev) {
    //take_actions

    dev->state = DEVICE_STATE_ON;
    return 0;
}

int change_device_state_to_standby(struct device *dev) {
    //take_actions

    dev->state = DEVICE_STATE_STANDBY;
    return 0;
}


int add_device_light(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name) {
    int flag = -1;
    for (int i = 1; i < MAX_DEVICE_SUM; ++i) {
        if (devlist[i] != NULL) continue;
        flag = 0;
        devlist[i] = (struct device *)calloc(1, sizeof(struct device));
        devlist[i]->device_id = i;
        strncpy(devlist[i]->device_name, dev_name, sizeof(devlist[i]->device_name) - 1);
        devlist[i]->type = DEVICE_LIGHT;
        devlist[i]->state = DEVICE_STATE_OFF;
        //take other actions
        break;
    }
    return flag;
}

int add_device_switch(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name) {
    int flag = -1;
    for (int i = 1; i < MAX_DEVICE_SUM; ++i) {
        if (devlist[i] != NULL) continue;
        flag = 0;
        devlist[i] = (struct device *)calloc(1, sizeof(struct device));
        devlist[i]->device_id = i;
        strncpy(devlist[i]->device_name, dev_name, sizeof(devlist[i]->device_name) - 1);
        devlist[i]->type = DEVICE_SWITCH;
        devlist[i]->state = DEVICE_STATE_OFF;
        //take other actions
        break;
    }
    return flag;
}

int add_device_thermostat(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name) {
    int flag = -1;
    for (int i = 1; i < MAX_DEVICE_SUM; ++i) {
        if (devlist[i] != NULL) continue;
        flag = 0;
        devlist[i] = (struct device *)calloc(1, sizeof(struct device));
        devlist[i]->device_id = i;
        strncpy(devlist[i]->device_name, dev_name, sizeof(devlist[i]->device_name) - 1);
        devlist[i]->type = DEVICE_THERMOSTAT;
        devlist[i]->state = DEVICE_STATE_OFF;
        //take other actions
        break;
    }
    return flag;
}
