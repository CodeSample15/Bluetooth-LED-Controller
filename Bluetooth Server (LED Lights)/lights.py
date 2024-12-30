import GATTServer

from ledstrip import strip
import threading

import math
import time
import random

led = strip(led_count=150)

mode = 'off'
running = False

#bunch 'o variables that the run method will use
wave_offset = 0
wave_rate = 5

stars_createStarTime = 0.055
stars_current_time = time.time()
stars_starDecay = 0.8
stars_star_channel_normal_value = 2

rainbow_r = 0.0
rainbow_g = 125.0
rainbow_b = 30.0
rainbow_rUp = True
rainbow_gUp = True
rainbow_bUp = True
rainbow_rocR = 0.1
rainbow_rocG = 0.4
rainbow_rocB = 0.8

#bunch 'o methods that the run method will use
def getCosValue(x):
	return int(255 * ((math.cos(x)+1)/2))
	
def getSinValue(x):
	return int(255 * ((math.sin(x)+1)/2))

#Every pattern programmed into a single, ugly, spaghetti loop
def run():
    global running
    global wave_offset
    global stars_current_time
    global rainbow_r, rainbow_g, rainbow_b, rainbow_rUp, rainbow_gUp, rainbow_bUp

    running = True
    while running:
        if mode == 'off':
            led.off()
        elif mode == 'static':
            led.set_color(GATTServer.yaw, GATTServer.pitch, GATTServer.roll)
        elif mode == 'waves':
            for x in range(0, led.LED_COUNT):
                red = getCosValue((x/wave_rate) + wave_offset)
                green = getSinValue((led.LED_COUNT-x)/wave_rate + wave_offset)

                led.set_pixel(x, [red, green, 30])
            led.update()
            wave_offset += 0.04
        elif mode == 'stars':
            if time.time() - stars_current_time > stars_createStarTime:
                stars_current_time = time.time()
                randomPosition = random.randrange(0, led.LED_COUNT)

                randC = random.randrange(0, 3)
                r = led.getPixelColorRGB(randomPosition).r
                g = led.getPixelColorRGB(randomPosition).g
                b = led.getPixelColorRGB(randomPosition).b

                if randC == 0:
                    r = random.randrange(2, 255)
                elif randC == 1:
                    g = random.randrange(2, 255)
                elif randC == 2:
                    b = random.randrange(2, 255)
                led.set_pixel(randomPosition, [r, g, b])

            for x in range(0, led.LED_COUNT):
                pixelColor = led.getPixelColorRGB(x)

                if pixelColor.r > stars_star_channel_normal_value:
                    setattr(pixelColor, 'r', pixelColor.r-stars_starDecay)
                if pixelColor.g > stars_star_channel_normal_value:
                    setattr(pixelColor, 'g', pixelColor.g-stars_starDecay)
                if pixelColor.b > stars_star_channel_normal_value:
                    setattr(pixelColor, 'b', pixelColor.b-stars_starDecay)

                led.set_pixel(x, [int(pixelColor.r), int(pixelColor.g), int(pixelColor.b)])
            led.update()
        elif mode == 'rainbow':
            if rainbow_rUp:
                rainbow_r += rainbow_rocR
                if rainbow_r > 255:
                    rainbow_r = 255
                    rainbow_rUp = False
            else:
                rainbow_r -= rainbow_rocR
                if rainbow_r < 0:
                    rainbow_r = 0
                    rainbow_rUp = True
            
            if rainbow_gUp:
                rainbow_g += rainbow_rocG
                if rainbow_g > 255:
                    rainbow_g = 255
                    rainbow_gUp = False
            else:
                rainbow_g -= rainbow_rocG
                if rainbow_g < 0:
                    rainbow_g = 0
                    rainbow_gUp = True

            if rainbow_bUp:
                rainbow_b += rainbow_rocB
                if rainbow_b > 255:
                    rainbow_b = 255
                    rainbow_bUp = False
            else:
                rainbow_b -= rainbow_rocB
                if rainbow_b < 0:
                    rainbow_b = 0
                    rainbow_bUp = True

            led.set_color(int(rainbow_r), int(rainbow_g), int(rainbow_b))


thread = threading.Thread(target=run)

def start():
    thread.start()

def stop():
     global running
     if running:
          running = False
          thread.join()
          led.off()