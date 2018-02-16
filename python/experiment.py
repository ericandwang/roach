#!/usr/bin/env python
"""
authors: stanbaek, apullin

"""
from lib import command
import time,sys,os,traceback
import serial

# Path to imageproc-settings repo must be added
sys.path.append(os.path.dirname("../../imageproc-settings/"))
sys.path.append(os.path.dirname("../imageproc-settings/"))      # Some projects have a single-directory structure
import shared

from hall_helpers import *

def main():    
    setupSerial()

    # Send robot a WHO_AM_I command, verify communications
    queryRobot()
    #Motor gains format:
    #  [ Kp , Ki , Kd , Kaw , Kff     ,  Kp , Ki , Kd , Kaw , Kff ]
    #    ----------LEFT----------        ---------_RIGHT----------
    #motorgains = [450,0,20,0,100, 0,0,0,0,0] #[600,0,20,0,0, 100,0,0,0,0]
    #thrustGains = [300,100,300,30,0,40]
    motorgains = [160,0,30,0,0, 0,0,0,0,0]
    thrustGains = [170,0,120, 170,0,120]
    #motorgains = [0,0,0,0,0, 0,0,0,0,0]# disable thrusters and tail
    #thrustGains = [0,0,0, 0,0,0]

    xb_send(0, command.SET_THRUST_OPEN_LOOP, pack('6h', *thrustGains))

    duration = 5000
    rightFreq = 0
    leftFreq = 0
    phase = 0
    telemetry = True
    repeat = False

    manParams = manueverParams(0, 0, 0, 0, 0, 0) # JY edits: added for compatibility
    params = hallParams(motorgains, duration, rightFreq, leftFreq, phase, telemetry, repeat)
    setMotorGains(motorgains)

    sj_params = sjParams(300, 800)
    # wj_params = wjParams(-551287, -50000, 1000000, 4941297, 411774)
    wj_params = wjParams(-551287, -40000, 80000, 5353068, 411774)
    wjParams.set(wj_params)


    while True:

        if not(params.repeat):
            settingsMenu(params, sj_params, wj_params)   

        if params.telemetry:
            # Construct filename
            # path     = '/home/duncan/Data/'
            path     = 'Data/'
            name     = 'trial'
            datetime = time.localtime()
            dt_str   = time.strftime('%Y.%m.%d_%H.%M.%S', datetime)
            root     = path + dt_str + '_' + name
            shared.dataFileName = root + '_imudata.txt'
            print "Data file:  ", shared.dataFileName
            print os.curdir

            numSamples = int(ceil(1000 * (params.duration + shared.leadinTime + shared.leadoutTime) / 1000.0))
            eraseFlashMem(numSamples)
            raw_input("Press enter to start run ...") 
            startTelemetrySave(numSamples)

        #Start robot 0: wall jump, 1: single jump, 2: vicon jumps
        exp = [2] 
        stopSignal = [0]

        viconTest = [0,0,0,0,0,0,65*256,78*256]#55*256,70*256]
        xb_send(0, command.INTEGRATED_VICON, pack('8h', *viconTest))
        time.sleep(0.01)

        xb_send(0, command.START_EXPERIMENT, pack('h', *exp))

        '''
        time.sleep(1)
        viconTest = [0,0,0,0,0,0,20*256,20*256]#55*256,70*256]
        xb_send(0, command.INTEGRATED_VICON, pack('8h', *viconTest))
        time.sleep(1)
        viconTest = [0,0,0,0,0,0,40*256,40*256]#55*256,70*256]
        xb_send(0, command.INTEGRATED_VICON, pack('8h', *viconTest))
        time.sleep(1)
        viconTest = [0,0,0,0,0,0,60*256,60*256]#55*256,70*256]
        xb_send(0, command.INTEGRATED_VICON, pack('8h', *viconTest))
        time.sleep(1)
        viconTest = [0,0,0,0,0,0,80*256,80*256]#55*256,70*256]
        xb_send(0, command.INTEGRATED_VICON, pack('8h', *viconTest))
        time.sleep(1)
        '''

        time.sleep(params.duration / 1000.0)
        
        #time.sleep(10)
        xb_send(0, command.STOP_EXPERIMENT, pack('h', *stopSignal))
        time.sleep(0.01)
        xb_send(0, command.STOP_EXPERIMENT, pack('h', *stopSignal))


        '''
        time.sleep(10)
        #viconTest = [0,0,0,0,0,0,256,256]
        #xb_send(0, command.INTEGRATED_VICON, pack('8h', *viconTest))
        time.sleep(20)
        xb_send(0, command.STOP_EXPERIMENT, pack('h', *stopSignal))
        '''


        # temp = [0]
        # xb_send(0, command.RESET_BODY_ANG, "0")
        # xb_send(0, command.PID_START_MOTORS, "0")
        # xb_send(0, command.SET_PITCH_SET, pack('l', *temp))
        # time.sleep(params.duration/1000.0)
        # xb_send(0, command.PID_STOP_MOTORS, "0")

        if params.telemetry and query_yes_no("Save Data?"):
            flashReadback(numSamples, params, manParams)

        repeatMenu(params)

    print "Done"
    
    
#Provide a try-except over the whole main function
# for clean exit. The Xbee module should have better
# provisions for handling a clean exit, but it doesn't.
#TODO: provide a more informative exit here; stack trace, exception type, etc
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print "\nRecieved Ctrl+C, exiting."
        shared.xb.halt()
        shared.ser.close()
    except Exception as args:
        print "\nGeneral exception:",args
        print "\n    ******    TRACEBACK    ******    "
        traceback.print_stack()
        print "    *****************************    \n"
        print "Attempting to exit cleanly..."
        shared.xb.halt()
        shared.ser.close()
        sys.exit()
    except serial.serialutil.SerialException:
        shared.xb.halt()
        shared.ser.close()
