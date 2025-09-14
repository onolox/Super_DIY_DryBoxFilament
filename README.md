## This is a project for a super powerful homemade DIY filament drying box!

The boxes sold on the market have a serious problem: besides being very expensive, they have no thermal insulation, which leads to enormous energy waste.
Furthermore, there is no need for a large flow of air entering and exiting the box, just a small inlet and outlet hole. What is good to have is a large flow of air circulating internally.
With that in mind, I decided to make my own box with things I had at home.


### List of items for the box:

- Cardboard box (the size depends on what you want; in my case, it fit four rolls);
- Two 2–3 cm Styrofoam boards (for the interior lining and the lid);
- Aluminum foil;
- Wood glue;
- 4 coolers;
- Ceramic socket for the ceramic lamps
- 100W heating lamp
- 1.5mm wire
- Old CPU heat sink
- Thermal paste


### Electronics list:

- RP2040
- DHT22 sensor
- 12v, 1A power supply
- Step-Down to 5V
- 2.4 ILI9341 LCD screen
- DIY or purchased dimmer
- Buzzer
- SD card

The box should be lined with Styrofoam and aluminum foil. For the rounded corners, I used square pieces cut with a hot wire from a bench power source. 
For the lid, I used the leftover pieces to make the inner outline. 
There is a hole at the bottom where air is sucked in and one at the top.
The lamp must be “glued” to the heat sink with plenty of thermal paste. The lamp can be used without anything, but this will greatly hinder heat dissipation. On its own, it operates at 450°C, but with the heat sink at 150°C, it has an extremely long service life.
The box can of course be improved by adding more lamps to speed up the initial heating.
For the dimmer, I used this project: [Arduino Dimmer](https://www.instructables.com/Arduino-controlled-light-dimmer-The-circuit/ “Arduino Dimmer”), but it can be purchased from china. However, I find it very expensive; for the price of the store, you can buy 10x in components.

The really difficult part that took me many months was the software, which is where the gold is. I used PlatformI0 in VS Code. It uses a PID control. I think the code is self-explanatory and there are comments to help.

#### Here are some explanations about the software:

It uses PID, but for PLA it uses a custom algorithm for the first hour.
It's important to note that you will need to adjust the PID values for your box, because of this i made it read the values kd, ki and kp from the SD-card for quick testing, one value per line. The file is PID.txt.



The run data is also saved to the SD for you to analyze.

The interface uses LVGL.

### Project photos:

|||
| ------------ | ------------ |
|<img src="/Images_Files/1.jpg" width="400px" /> |<img src="/Images_Files/2.jpg" width="400px" /> |
|<img src="/Images_Files/3.jpg" width="400px" /> |<img src="/Images_Files/4.jpg" width="400px" /> |
|<img src="/Images_Files/5.jpg" width="400px" /> |<img src="/Images_Files/6.jpg" width="400px" /> |
|<img src="/Images_Files/7.jpg" width="400px" /> |<img src="/Images_Files/8.jpg" width="400px" /> |
|<img src="/Images_Files/screen running.jpg" width="400px" /> |<img src="/Images_Files/rp2040 YD black.webp" width="400px" /> |

### Tip for securing the screws: Drill the holes and tap the screw, then apply super glue to the holes to reinforce them:

<img src="/Images_Files/Mounting holes.jpg" width="400px" />

### Photo of the box with the electronics:

<img src="/Images_Files/Board 1.jpg" width="400px" />

📁 [Fusion 360 design of the box where the electronics are located to use as a base.](Images_Files/DIY%20Filament%20Dry%20Box.f3d)


# Here you will find everything you need to create your own filament drying box. Improve it, share it.

