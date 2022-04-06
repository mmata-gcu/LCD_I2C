# LCD_I2C
Uses an I2C LCD screen (Grove LCD RGB colour background, 2 display lines) to show the assigned IP address and the status of the ROS Master node.
This is a small C program intended to be run at start up in the robot's main computer running Linux (before ROS is launched). The main computer used needs to provide I2C pins (for instance a Nvidia Jetson TX1, Raspberry Pi, etc).
It is intended to facilitate the headless use of the robot on a DHCP WiFi, and/or to select one particular robot if several of them are around. Knowing the robot's IP allows remote connections to it, or directly joining the robot's ROS master.

It displays the assigned IP on 1st line of the LCD screen, and a message in the 2nd line (plus a background colour code) to indicate the ROS status:
- red background = no IP assigned
- blue background = IP assigned (will be displayed on the first line), ROS Master not running in the robot
- green background = IP assigned (will be displayed on the first line), ROS Master ready

As an example, In particular, this is the connection of the LCD to the Jetson TX1 I2C bus 1. Pins at TX1 connector J21 are:
-  1: 3.3V for SDA and SCL pull-up resistors
-  2: 5V to supply the LCD screen
-  6: GND
- 27: SDA
- 28: SCL

To set up the program:
- get the **i2c** linux library: _sudo apt-get install libi2c-dev i2c-tools_
- compile the program under the **lcd_i2c** folder, using _cmake ._ and _make_
- the _lcd_i2c_ executable file needs to be run with sudo to access the I2C; it is convenient to grant I2C access to your default user (see next)

To provide I2C access for the default user (_my_user_ in this example, please update it to match yours):
- create file /etc/udev/rules.d/50-i2c.rules: _sudo gedt /etc/udev/rules.d/50-i2c.rules_
- write inside it (and save it): _SUBSYSTEM=="i2c-dev", GROUP="i2cuser", MODE="0660"_
- create users group "i2cuser": _sudo groupadd i2cuser_
- add user 'ubuntu' to that group: _sudo adduser my_user i2cuser_

Note: changes won't be effective until restarting the computer

To autostart the _lcd_i2c_ program when the computer boots up, open the _'Startup applications'_ application (or equivalent for your Linux version) and create a new entry, selecting the executable file
