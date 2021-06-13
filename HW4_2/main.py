THRESHOLD = (0,100)
BINARY_VISIBLE = True
import pyb, sensor, image, time, math
enable_lens_corr = False # turn on for straighter lines...
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()

# All lines also have x1(), y1(), x2(), and y2() methods to get their end-points
# and a line() method to get all the above as one 4 value tuple for draw_line().
low_threshold = (0, 50)
uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)

while(True):
   clock.tick()
   img = sensor.snapshot().binary([THRESHOLD]) if BINARY_VISIBLE else sensor.snapshot()
   #if enable_lens_corr: img.lens_corr(1.😎 # for 2.8mm lens...
   line = img.get_regression([(255,255) if BINARY_VISIBLE else THRESHOLD],roi = (20,0,140,70), robust = True)
   if (line): img.draw_line(line.line(),color =127)


   print(line.rho())
   print_arg=(line.rho())

   uart.write(("%f" % print_arg).encode())
   #uart.write(("%f" % print_args2).encode())
   #uart.write(("%f" % print_args3).encode())
   #uart.write(("%f" % print_args4).encode())