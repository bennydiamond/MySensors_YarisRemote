# MySensors_YarisRemote
MySensors device hardwired to a spare Yaris 2008 Keyfob.

Used to expose Door locking/unlocking capabilities on my old car to Home Assistant.

As a bonus, can also toggle "alarm" mode on the car although this isn't planned to be used.

This requires a donor keyfob module to solder onto it. Cheap replacement keyfobs/remotes are readily available on AliExpress. Make sure to order one with the correct Chip variant for your car.

Threw in a SHT21 Temperature and humidity sensor in there because why not!

# Hardware
- Arduino Nano v3 168p
- NRF24L01+
- Toyota Yaris 2008 keyfob bare PCB
- SHT21 Temperature + Humidity Sensor

# Librairies
- [MySensors](https://github.com/mysensors/MySensors)
- [SHT2x](https://github.com/RobTillaart/SHT2x) Because it's non blocking (ish) and I wanted to keep my main code simple with easy to use `wait` function from MySensors

# Hardware design
The design is rather simple. Arduino is 5V. Keyfob module runs on 3V battery. Arduino's GPIO are soldered to poles of the switches on the keyfob through diodes with cathodes facing Arduino's GPIOs. 

Keyfobs switches are held high at resting state and assert to GND when a button is pressed. To simulate a button press, all is required is to transition Arduino's GPIO from a HIGH resting state to a temporary LOW state. Keyfob receives button pressed signal.

NRF24 module is connected as described in MySensors' documentation.
