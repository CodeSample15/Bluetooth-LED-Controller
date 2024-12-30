import GATTServer
import lights
import asyncio

def main():
    #start lights
    lights.start()

    #run GATT server
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(GATTServer.run(loop))
    except KeyboardInterrupt:
        print("Interruped, cancelling tasks")

    loop.run_until_complete(GATTServer.stop())
    lights.stop()

if __name__ == '__main__':
    main()