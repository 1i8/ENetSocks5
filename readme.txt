* READ THOROUGHLY OR YOU CAN END UP IN CONFLICT (either personally - that means with ME, or with understanding how this works) * 

Codename: "Satan's Nuke", development code name: EBotNet. [Project is CLASSIFIED SOFTWARE until I quit]

This project is not to be disclosed as it imposes severe destruction upon any networked ENet server.

Not much help is provided for the project, you need to have programming knowledge yourself to get this to running.

Distributing the project to anyone is only allowed for me (github.com/playingoDEERUX), in case anyone in this group distributes it without my permission,
full src will be open src'ed in order to avoid the offender from making too much of a money-gain.

Using this source to harm our own products in any way will also result in publishing of the source.

Recommended use case for this project is mass botting on real GT, or selling off bot API to others to make profit via that.

On linux, please compile using build.sh.
Also, linux has a socket limit of 1024 - however this implementation of ENet SOCKS5 has the capability (unlike others),
can target up to 32K enet clients per machine, provided you have a proxy pool provider to offer that many IPs.
You DO still have to unlock the socket limit of 1024 on linux-based systems using ulimit -n 65535

* The other necessity is that you use THIS ENet provided by EBotNet, as it has special modifications to go beyond 1024 sockets for
the ENetHosts. For one, it makes use of poll() callback instead of select, to support unlocked connections to max. 64K
(out of which 32K shall be used for ENet [udp], and 32K for the UDP associate sessions triggered using TCP on the SOCKS5 servers,
as you have to be connected all the time to the socks5 server for using UDP-relaying,
for reference see: https://stackoverflow.com/questions/11614670/select-seems-to-segfault-kill
and also see socks5 docs: https://www.rfc-editor.org/rfc/rfc1928

This source (including ENet) is also extremely optimized, to ensure everything runs stable and fast while using this many bots.
You can check-out all modifications yourself.

Made by DEERUX#7072 only - no other people are affiliated with this software.

NOTE: You need OpenSSL library installed & linked!

EDIT: now public on github :)

NOTE 2: If you're just one of those people who want the socks5 implemented with ENet, all you need is SocksTunnel.cpp and the enet include/source files ;)
