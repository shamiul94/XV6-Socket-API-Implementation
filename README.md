# XV6-Socket-API-Implementation
## COPY AT YOUR OWN RISK
In this problem we are trying to implement socket API in ```XV6 operating system```. We will create multiple processes and create one socket in each of them. In our case we have reduced our scope to one-to-one connection. That means, every server can be connected to only one client. If we have 64 sockets, we can make highest 32 connections to work (1 server and 1 client busy in each connection). 

```patch_xv6_Socket_1505038_Final_Final``` should be applied on a fresh ```XV6``` copy from ```XV6``` public github repository  by ```git apply patch_xv6_Socket_1505038_Final_Final``` command. This patch will make all the necessary changes on that fresh XV6 copy of yours. Run these commads to run and test - 
```
  make clean
  make
  make qemu
```
