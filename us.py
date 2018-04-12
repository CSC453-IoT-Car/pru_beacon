from Adafruit_BBIO import GPIO as io
import time
import datetime

outpin = "P8_17"
inpin = "P8_19"

io.setup(outpin, io.OUT)
io.setup(inpin, io.IN)

start = 0
end = 0
while True:
    io.output(outpin, io.HIGH)
    time.sleep(.001)
    io.output(outpin, io.LOW)
    to = datetime.datetime.now().microsecond + 3500
    while(not io.input(inpin) and datetime.datetime.now().microsecond < to):pass
    if datetime.datetime.now().microsecond >= to : 
        time.sleep(2)
        print "to"
        continue
    start = datetime.datetime.now().microsecond
    to = start + 3500
    while(io.input(inpin) and datetime.datetime.now().microsecond < to):pass
    end = datetime.datetime.now().microsecond
    print start, end, end-start
    time.sleep(2)