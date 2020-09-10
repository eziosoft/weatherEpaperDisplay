Reuse of not working price tag (SES imagotag Vusion 2.6 BWR) display as weather display.

Parts:
* price tag
* esp8266 - Wemos Mini
* HT7333 Low Power Consumption LDO 3.3V
* 1S protected lipo battery
* 1S usb charger

Goals:
* be able to run on battery for some time
* be able to display text and lines
* automatic firmware update
* controlled via MQTT to be easly integrated with NodeRed


---

[PCB and connections](files/teardown.png)

---





Display controlled via MQTT and json
based on epaper price tag + wemos mini



example json:

    {"NR":600,"i":4,"ic":1,"d":[
        {"x":1,"y":10,"f":0,"c":0,"t":"20:20:15 (10m)"},
        {"x":10,"y":50,"f":1,"c":1,"t":"20.9C"},
        {"x":10,"y":90,"f":1,"c":1,"t":"Clouds"},
        {"x":10,"y":110,"f":0,"c":0,"t":"broken clouds"},
        {"x":10,"y":125,"f":0,"c":0,"t":"Gentle breeze"},
        {"x":10,"y":150,"f":0,"c":0,"t":"Sun:5:51:44 - 21:56:48"},
        {"x":180,"y":30,"f":0,"c":1,"t":"Faible:36"}
    ],
    "l":[
        {"x1":0,"y1":0,"x2":100,"y2":100, "c":0},
        {"x1":0,"y1":0,"x2":110,"y2":120, "c":0},
        {"x1":0,"y1":0,"x2":120,"y2":130, "c":0},
        {"x1":0,"y1":0,"x2":130,"y2":140, "c":0},
        {"x1":0,"y1":0,"x2":140,"y2":150, "c":0}
    ]}

* NR - next refresh in seconds
* i -  icon number to display
* d - data array
* x,y - coordinates
* f - font ( 0 small , 1 big)
* c - color (0 black, 1 red)
* t - text
* l - lines array



example script NodeRed:

    const minute= 60;
    var NR = 30*minute;
    var long = 60*minute;

    // NR = 10000;


    var d = new Date();
    var h = d.getHours();
    var m = d.getMinutes();

    if(30-m>0)
    {
        NR = (30-m)*minute;
    }else {
        NR=(60-m)*minute;
    }

    switch(h)
    {
        case 0: NR=long; break;
        case 1: NR=long; break;
        case 2: NR=long; break;
        case 3: NR=long; break;
        case 4: NR=long; break;
        case 5: NR=long; break;
    default:
        break;
    }




    var scale =['Calm', 'Light air', 'Light breeze', 'Gentle breeze', 'Moderate breeze', 'Fresh breeze', 'Strong breeze', 'High wind', 'Gale', 'Strong gale', 'Storm', 'Violent Storm', 'Hurricane'];
    var wind = msToBeaufort(msg.payload.windspeed);

    var icon =msg.payload.icon;

    var s = `{"NR":${NR},"i":${parseInt(icon.substring(0, 2))},"d":[{"x":1,"y":10,"f":0,"c":0,"t":"`+startTime()+` (${NR/minute}m)"},
    {"x":10,"y":50,"f":1,"c":1,"t":"${msg.payload.tempc}C"},
    {"x":10,"y":90,"f":1,"c":1,"t":"${msg.payload.weather}"},
    {"x":10,"y":110,"f":0,"c":0,"t":"${msg.payload.detail}"},
    {"x":10,"y":125,"f":0,"c":0,"t":"${scale[wind]}"},
    {"x":10,"y":150,"f":0,"c":0,"t":"Sun:${time(msg.payload.sunrise)} - ${time(msg.payload.sunset)}"},
    {"x":180,"y":30,"f":0,"c":1,"t":"${global.get("air")}"}
    ]}`;


    msg.payload=s; 

    node.status({text:"NR:"+NR/minute+"minutes"});


    return msg;


    function msToBeaufort(ms) {
        return Math.ceil(Math.cbrt(Math.pow(ms/0.836, 2)));
    }

    function financial(x) {
    return Number.parseFloat(x).toFixed(2);
    }

    function state(x) {
    if(x) return "OK";
    else return "!!";
        
    }

    function startTime() {
        var date = new Date();
        var hours = date.getHours();
        // Minutes part from the timestamp
        var minutes = "0" + date.getMinutes();
        // Seconds part from the timestamp
        var seconds = "0" + date.getSeconds();

        // Will display time in 10:30:23 format
        var formattedTime = hours + ':' + minutes.substr(-2) + ':' + seconds.substr(-2);
        return formattedTime;
    }

    function time(t) {
        var date = new Date(t * 1000 );
        var hours = date.getHours();
        // Minutes part from the timestamp
        var minutes = "0" + date.getMinutes();
        // Seconds part from the timestamp
        var seconds = "0" + date.getSeconds();

        // Will display time in 10:30:23 format
        var formattedTime = hours + ':' + minutes.substr(-2) + ':' + seconds.substr(-2);
        return formattedTime;
    }