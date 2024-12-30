#a useful wrapper for easily controlling the led light strip
from rpi_ws281x import *

class color:
	def __init__(self, r=0, g=0, b=0):
		self.r = r
		self.g = g
		self.b = b

class strip:
	def __init__(self, led_count=300, led_pin=18, freq=800000, dma=10, brightness=255, invert=False, channel=0):
		self.LED_COUNT = led_count
		self.LED_PIN = led_pin
		self.LED_FREQ_HZ = freq
		self.LED_DMA = dma
		self.LED_BRIGHTNESS = brightness
		self.LED_INVERT = invert
		self.LED_CHANNEL = channel
		
		self.strip = Adafruit_NeoPixel(self.LED_COUNT, self.LED_PIN, self.LED_FREQ_HZ, self.LED_DMA, self.LED_INVERT, self.LED_BRIGHTNESS, self.LED_CHANNEL)
		self.strip.begin()

	def getStrip(self):
		return self.strip

	def getPixelColorRGB(self, pixel):
		return color(self.strip.getPixelColorRGB(pixel).r, self.strip.getPixelColorRGB(pixel).g, self.strip.getPixelColorRGB(pixel).b)

	def pixelCount(self):
		return self.LED_COUNT

	def set_color(self, r, g, b, a=255):
		for pixel in range(0, self.LED_COUNT):
			self.strip.setPixelColor(pixel, Color(r,g,b))
		self.strip.setBrightness(a)
		self.strip.show()
		
	def set_pixel(self, pixel, color):
		self.strip.setPixelColor(pixel, Color(color[0], color[1], color[2]))
		
	def off(self):
		self.set_color(0, 0, 0)
	
	def update(self):
		self.strip.show()