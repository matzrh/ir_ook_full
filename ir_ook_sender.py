#!/usr/bin/python3
from os import listdir
import sys
import signal
import termios
import re
import serial
import time
import logging

from homie.device_base import Device_Base
from homie.node.node_state import Node_State
from homie.node.node_switch import Node_Switch

logger = logging.getLogger("IrOokSender")
logger.setLevel(logging.INFO)
fh = logging.FileHandler("/var/log/ir_ook.log")
logger.addHandler(fh)


class ConnectToUSB:
    __theUsbDevice=None
    remotes=[]
    serialLock=False
    QUERYBYTE = 0xff

    def __init__(self):
        self.connect()

    def __del__(self):
        if(self.__theUsbDevice):
            self.__theUsbDevice.close()

    def connect(self):
        devPath='/dev'
        usbRegex=re.compile('tty\.?(usbserial|USB)')  #try out different USB char devices
        remRegex=re.compile('(bool|enum)\:(.+?)\:(.+,)?') #the device announces itself in the given format bool: Switch, enum: States
        usbDevices = [f for f in listdir(devPath) if usbRegex.match(f)]
        while self.serialLock:
            pass
        self.serialLock=True
        for usbPort in usbDevices:
            serialUSB=serial.serial_for_url(
                devPath + '/' + usbPort,baudrate=115200,
                timeout=3,
                do_not_open=True)
            logger.debug('testing ' + serialUSB.name)
            try:
                serialUSB.open()
            except serial.serialException:
                continue
            serialUSB.reset_input_buffer()
            serialUSB.write([self.QUERYBYTE])
            serialUSB.flush()
            time.sleep(2)
            response=serialUSB.read(serialUSB.in_waiting)
            for line in response.decode().splitlines():
                remoteMatch=remRegex.match(line)
                if not remoteMatch:
                    continue
                newRemote={'type':remoteMatch.group(1),
                           'name':remoteMatch.group(2)}
                if newRemote['type']=='enum':
                    newRemote['commands']=remoteMatch.group(3).strip(',').split(',')
                self.remotes.append(newRemote)
            if len(self.remotes):  #if remotes are found, stop iterations and return
                self.__theUsbDevice=serialUSB
                self.serialLock=False
                break
            serialUSB.close()
            #end of for loop (usbport iteration)
        self.serialLock=False
        return

    def getSerialName(self):
        if not self.__theUsbDevice:
            return ''
        return self.__theUsbDevice.name

    def getSerialFNo(self):
        if not self.__theUsbDevice:
            return None
        return self.__theUsbDevice.fileno()

    def checkConnect(self):  #check if connection is still working
        try:
            while self.serialLock:
                pass
            self.serialLock=True
            self.__theUsbDevice.reset_input_buffer()
            self.__theUsbDevice.write([self.QUERYBYTE])
            self.__theUsbDevice.flush()
            line=self.__theUsbDevice.read_until()
            if ':' in line.decode():
               return True
        except (serial.SerialException, termios.error) as e:
            logger.warning("Serial Exception in Check")
            self.__theUsbDevice=None
            self.remotes=[]
            return False
        finally:
            self.serialLock=False
        logger.warning("timeout after check")
        return False

    def send(self, byte):
        try:
            while self.serialLock:
                pass
            self.serialLock=True
            self.__theUsbDevice.reset_input_buffer()
            self.__theUsbDevice.write([byte])
            self.__theUsbDevice.flush()
            line=self.__theUsbDevice.read_until().decode()
            logger.info(line)
        except (serial.SerialException, AttributeError) as e:
            return "serial Error"
        finally:
            self.serialLock=False
        return ('Protocol' in line)


class Remotes:
    remotes = None

    def __init__(self,connection):
        if(connection.remotes == []):
            connection.connect()
        if(connection.remotes == []):
            return None
        self.connection=connection
        self.remotes=connection.remotes
        step1=[len(r['commands']) if r['type']=='enum' else 2 for r in self.remotes]
        self.indexer=[sum(step1[0:n]) for n in range(0,6)]

    def getCode(self,protocolIndex,commandIndex):
        return self.indexer[protocolIndex]+commandIndex

    def sendCode(self,protocolIndex,commandIndex):
        if(not self.connection):
            return False
        return self.connection.send(self.getCode(protocolIndex,commandIndex))

    def getRemoteName(self,protocolIndex):
        if protocolIndex >= len(self.remotes):
            return None
        return self.remotes[protocolIndex]['name']

    def getCommandName(self,protocolIndex, commandIndex):
        if protocolIndex >= len(self.remotes):
            return None
        cmds=self.remotes[protocolIndex]['commands'] if self.remotes[protocolIndex]['type']=='enum' else ['off','on']
        if commandIndex >= len(cmds):
            return None
        return cmds[commandIndex]


class Device_Multi_Remotes(Device_Base):
    mqtt_settings = {
    'MQTT_BROKER' : 'localhost',
    'MQTT_PORT' : 1883,
    'MQTT_USERNAME' : None,
    'MQTT_PASSWORD' : None,
    }


    def __init__(self,remotes):

        if(not remotes):
            return None

        super().__init__(
            device_id='ir-ook-remotes',
            name='IR and OOK Remote Controller',
            mqtt_settings=self.mqtt_settings)
        if remotes.remotes:
            self.setRemotes(remotes)
        self.start()

    def setRemotes(self,remotes):
        #remove nodes first...
        self.nodes = {}
        self.remotes=remotes
        for rem in remotes.remotes:
            nodeName=rem['name']
            nodeId=self.generateNodeId(nodeName)
            if rem['type']== 'enum':
                self.add_node(
                    Node_State(
                        self,
                        id=nodeId,
                        name=nodeName,
                        state_values=",".join(rem['commands']),
                        set_state=self.enumSetterFabric(nodeName)
                    )
                )
            if rem['type'] == 'bool':
                self.add_node(
                    Node_Switch(
                        self,
                        id=nodeId,
                        name=nodeName,
                        set_switch=self.boolSetterFabric(nodeName)
                    )
                )
        

    @staticmethod
    def generateNodeId(name):
        return re.sub('[^a-z0-9\-]','',name.lower())[:15]

    def update_state(self, rName, state):
        type=next((r['type'] for r in self.remotes.remotes if r['name']==renName), None)
        if not type:
            logger.warning("{} not found in remotes".format(renName))
            return
        nodeId=self.generateNodeId(rName)
        if(type == 'enum'):
            self.get_node(nodeId).update_state(state)
        if(type == 'bool'):
            self.get_node(nodeId).update_switch(state)
        logger.info("updated {} to {}".format(renName, state))
        

    def enumSetterFabric(self,renName):
        try:
            pInd=[r['name'] for r in self.remotes.remotes].index(renName)
        except ValueError:
            logger.warning("{} not found in remotes".format(renName))
            return None
        commands=self.remotes.remotes[pInd]['commands']
        
        def stateSetter(state):
            try:
                cInd=commands.index(state)
            except ValueError:
                logger.warning("{} not found in commands".format(state))
                return
            self.remotes.sendCode(pInd,cInd)
            logger.info("State of {} Set {}".format(renName,state))
            
        logger.info("produced stateSetter for remote " + renName)
        return stateSetter

    def boolSetterFabric(self,renName):
        try:
            pInd=[r['name'] for r in self.remotes.remotes].index(renName)
        except ValueError:
            return None
        
        def switchSetter(onoff):
            self.remotes.sendCode(pInd,1 if onoff=='ON' else 0)
            logger.info("Switch {} Set {}".format(renName,onoff))

        logger.info("produced switchSetter for remote " + renName)
        return switchSetter
            

def daemonLoop():
    global exitDaemon
    print("Starting Daemon") #to syslog
    connector = ConnectToUSB()
    logger.info("device is on " + connector.getSerialName())
    logger.debug(connector.remotes)
    remotes=Remotes(connector)
    homieFabric=Device_Multi_Remotes(remotes)
    if homieFabric.state != "ready":
        print(homieFabric.state)
        sys.exit("could not connect to mqtt broker")
    while not exitDaemon:
        time.sleep(5)
        signal.signal(signal.SIGINT, shutdown)
        signal.signal(signal.SIGTERM, shutdown)
        if not (connector.getSerialName() and connector.checkConnect()):
            logger.debug("Locked" if connector.serialLock else "Free")
            logger.info("trying to connect...")
            connector.connect()
            logger.debug("Locked" if connector.serialLock else "Free")
            logger.debug(connector.remotes)
            if(connector.remotes==[]):
                homieFabric.close()
                logger.info("could not connect")
            else:
                remotes=Remotes(connector)
                homieFabric.setRemotes(remotes)
                homieFabric.state="ready"
                homieFabric.subscribe_topics()
        else:
            logger.debug("still connected")
    homieFabric.close()
    del homieFabric
    del remotes
    del connector
    print("Daemon stopped")
    sys.exit(0)


def shutdown(signum, frame):
    global exitDaemon
    exitDaemon=True

exitDaemon=False
if __name__ == '__main__':
    daemonLoop()
else:
    print(__name__)
