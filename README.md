# Bluepill + W5500 Ethernet Webserver using PlatformIO 

# Description 

This project uses a "W5500 lite" board (also called DEVMO USR-ES1) connected to a standard BluePill board (STM32F103C8).

![[/w5500_modules.png|W5500 Module]]

# Wireup

Refer module datasheet for details. 

* Power pins: All GNDs to Bluepill's GNDs, all VIN3.3 pins to Bluepill's 3.3V 
	* since the module is power-hungry, extra decoupling capacitors on the 3.3V rail are recommended
* SPI: use default SPI1 (SCK = PA5, MISO = PA6, MOSI = PA7) and arbitrary chip select (here CS = PA4)
* nRESET of W5500 module **must** be tied to 3.3V, otherwise the chip won't start
* INT and POWERDOWN are not used

# Expected output

The `Serial` UART output is on PA9 at 115200 baud. 

The sketch should start with detecting the W5500 chip and getting an IP address via DHCP. If its HTTP website (port 80) is visited, the request is printed and some data is sent back.

```
Ethernet WebServer Example
W5500 detected
server is at 192.168.1.237
new client
GET / HTTP/1.1
Host: 192.168.1.237
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: de,en-US;q=0.7,en;q=0.3
Accept-Encoding: gzip, deflate
Connection: keep-alive
Upgrade-Insecure-Requests: 1
Cache-Control: max-age=0

client disconnected
```

![[/webpage.png|Webpage in Firefox]]

# Technical Documents 

* Datasheet W5500 Module: https://www.usriot.com/download/ES1/USR-ES1-EN-V1.0.pdf
* Datasheet W5500 chip: http://wizwiki.net/wiki/lib/exe/fetch.php/products:w5500:w5500_ds_v109e.pdf
* STM32Duino Bluepill pinout: https://stm32duinoforum.com/forum/wiki_subdomain/index_title_Blue_Pill.html