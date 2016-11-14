/*
 * Name: tail_ctrl.c
 * Desc: Control code for pitch control of hopping robot
 * Date: 6/10/2015
 * Author: DWH
 */
#include "tail_ctrl.h"
#include "led.h"
#include "mpu6000.h"
#include "timer.h"
#include "ports.h"
#include "pid-ip2.5.h"
#include "experiment.h"
#define ABS(my_val) ((my_val) < 0) ? -(my_val) : (my_val)


#define GYRO_LSB2_DEG (0.061035156) // +-2000 deg/s in 16 bits

static volatile unsigned char interrupt_count = 0;
static int interval[NUM_VELS];
long set_pt = 0;

long body_angle[3];

extern pidPos pidObjs[NUM_PIDS];

char pitchControlFlag;
long pitchSetpoint;


void tailCtrlSetup(){
    int i;
    for(i=0; i<NUM_VELS; i++){
        interval[i] = 2; //2 ms duration for in
    }
    body_angle[0]=0;
    body_angle[1]=0;
    body_angle[2]=0;
    initPIDObjPos( &(pidObjs[0]), 0,0,0,0,0);
    // initPIDObjPos( &(pidObjs[2]), -500,0,-500,0,0);
    initPIDObjPos( &(pidObjs[2]), 0,0,0,0,0);
    initPIDObjPos( &(pidObjs[3]), 0,0,0,0,0); //100,100
    SetupTimer5();
    EnableIntT5;
    pitchControlFlag = 0;
    pitchSetpoint = 0;

    pidObjs[0].timeFlag = 0;
    pidObjs[0].mode = 0;
    pidSetInput(0, 0);
    pidObjs[0].p_input = pidObjs[0].p_state;
    pidObjs[2].p_input = 0;
    pidObjs[3].p_input = 0;
    pidObjs[2].v_input = 0;
    pidObjs[3].v_input = 0;
    pidOn(0);
}

void setPitchControlFlag(char state){
    pitchControlFlag = state;
}


void setPitchSetpoint(long setpoint){
    pitchSetpoint = setpoint;
}

void resetBodyAngle(){
    // mpuRunCalib(0,100); //re-offset gyro, assumes stationary
    body_angle[0] = 0;
    body_angle[1] = 0;
    body_angle[2] = 0;
}

///////        Tail control ISR          ////////
//////  Installed to Timer5 @ 1000hz  ////////
void __attribute__((interrupt, no_auto_psv)) _T5Interrupt(void) {
    interrupt_count++;
      
    if(interrupt_count <= 5) {
        // Do signal processing on gyro
        int gdata[3];
        mpuGetGyro(gdata);
        // body_angle += gdata[2]*GYRO_LSB2_DEG*0.001;
        if(ABS(gdata[2] - 51)>80){body_angle[0] += gdata[2] - 51;}
        if(ABS(gdata[1] + 10)>80){body_angle[1] += gdata[1] + 10;}
        if(ABS(gdata[0] - 15)>80){body_angle[2] += gdata[0] - 15;}

    }
    if(interrupt_count == 5) 
    {
        interrupt_count = 0;
        expFlow();
        if(pitchControlFlag == 0){
            //LED_2 = 0;
            // Control/motor off
            pidObjs[0].onoff = 0;
        } else {
            //LED_2 = 1;
            // Control pitch
            pidObjs[0].p_input = pitchSetpoint;
            pidObjs[2].p_input = 0;
            pidObjs[3].p_input = 0;
        }
    }

    _T5IF = 0;
}


void SetupTimer5() {
    ///// Timer 5 setup, Steering ISR, 300Hz /////
    // period value = Fcy/(prescale*Ftimer)
    unsigned int T5CON1value, T5PERvalue;
    // prescale 1:64
    T5CON1value = T5_ON & T5_IDLE_CON & T5_GATE_OFF & T5_PS_1_64 & T5_SOURCE_INT;
    T5PERvalue = 625; //1Khz
    OpenTimer5(T5CON1value, T5PERvalue);
}
