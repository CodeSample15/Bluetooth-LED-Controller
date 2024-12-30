import lights

import asyncio
from typing import Any, Dict

from bless import (
    BlessServer,
    BlessGATTCharacteristic,
    GATTCharacteristicProperties,
    GATTAttributePermissions,
)

server = None
stop_event = asyncio.Event()

LOOP_DELAY = 0.1

yaw = 0
pitch = 0
roll = 0

mode = 'off'

def read_request(characteristic: BlessGATTCharacteristic, **kwargs) -> bytearray:
    #print(f"Reading {characteristic.value}")
    return characteristic.value


def write_request(characteristic: BlessGATTCharacteristic, value: Any, **kwargs):
    characteristic.value = value
    #print(f"Char value set to {characteristic.value}")
    
async def run(loop):
    global device_connected, yaw, pitch, roll, server

    stop_event.clear()

    gatt: Dict = {
        "f6351508-a501-4022-acba-7232431a7fca": {
            "57ead45a-9ac2-4810-98c3-63191c97cece": { #yaw
                "Properties": (
                    GATTCharacteristicProperties.read
                    | GATTCharacteristicProperties.write
                    | GATTCharacteristicProperties.indicate
                ),
                "Permissions": (
                    GATTAttributePermissions.readable
                    | GATTAttributePermissions.writeable   
				),
                "Value": 0,
            },
            "0c712b41-f2ec-462d-b60e-5defdc405db2": { #pitch
                "Properties": (
                    GATTCharacteristicProperties.read
                    | GATTCharacteristicProperties.write
                    | GATTCharacteristicProperties.indicate
                ),
                "Permissions": (
                    GATTAttributePermissions.readable
                    | GATTAttributePermissions.writeable
                ),
                "Value": 0,
            },
            "c4b5dc3e-0bcd-4f34-93be-3afbabc6eed8": { #roll
                "Properties": (
                    GATTCharacteristicProperties.read
                    | GATTCharacteristicProperties.write
                    | GATTCharacteristicProperties.indicate
                ),
                "Permissions": (
                    GATTAttributePermissions.readable
                    | GATTAttributePermissions.writeable
                ),
                "Value": 0,
			},
            "4722c037-0957-4d33-b8a4-69378fd2ef9a": { #led mode
                "Properties": (
                    GATTCharacteristicProperties.read
                    | GATTCharacteristicProperties.write
                    | GATTCharacteristicProperties.indicate
                ),
                "Permissions": (
                    GATTAttributePermissions.readable
                    | GATTAttributePermissions.writeable
                ),
                "Value": 0,
            }
        }
    }
      
    my_service_name = "LukeRaspiZero"
    server = BlessServer(name=my_service_name, loop=loop)
    server.read_request_func = read_request
    server.write_request_func = write_request

    print("Starting server...")

    await server.add_gatt(gatt)
    await server.start()
	
    print("Server started successfully!")
    print("Advertising")
	
    while True:
        #read data
        yaw = int.from_bytes(server.get_characteristic("57ead45a-9ac2-4810-98c3-63191c97cece").value, "little", signed=True)
        pitch = int.from_bytes(server.get_characteristic("0c712b41-f2ec-462d-b60e-5defdc405db2").value, "little", signed=True)
        roll = int.from_bytes(server.get_characteristic("c4b5dc3e-0bcd-4f34-93be-3afbabc6eed8").value, "little", signed=True)
		
        mode = int.from_bytes(server.get_characteristic("4722c037-0957-4d33-b8a4-69378fd2ef9a").value, "little", signed=True)
        if mode == 0:
            lights.mode = 'off'
        elif mode == 1:
            lights.mode = 'static'
        elif mode == 2:
            lights.mode = 'waves'
        elif mode == 3:
            lights.mode = 'stars'
        elif mode == 4:
            lights.mode = 'rainbow'

        await asyncio.sleep(LOOP_DELAY)

async def stop():
    await server.stop()
    print("GATT server stopped")

def run_server():
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(run(loop))
    except KeyboardInterrupt:
        print("Interruped, cancelling tasks")

if __name__ == '__main__':
    run_server()