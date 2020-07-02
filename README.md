Display controlled via MQTT and json
based on epaper price tag + wemos mini



example json:
{"NR":600,"i":4,"d":[{"x":1,"y":10,"f":0,"c":0,"t":"20:20:15 (10m)"},
{"x":10,"y":50,"f":1,"c":1,"t":"20.9C"},
{"x":10,"y":90,"f":1,"c":1,"t":"Clouds"},
{"x":10,"y":110,"f":0,"c":0,"t":"broken clouds"},
{"x":10,"y":125,"f":0,"c":0,"t":"Gentle breeze"},
{"x":10,"y":150,"f":0,"c":0,"t":"Sun:5:51:44 - 21:56:48"},
{"x":180,"y":30,"f":0,"c":1,"t":"Faible:36"}
]}

NR - next refresh in seconds
i -  icon number to display
d - data array
x,y - coordinates
f - font ( 0 small , 1 big)
c - color (0 black, 1 red)
t - text