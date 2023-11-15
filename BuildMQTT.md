#部署在ARM64开发板上  
https://www.emqx.com/zh/downloads-and-install?product=broker&version=5.3.1&os=Ubuntu20.04&oslabel=Ubuntu+20.04  

##1下载deb包    
```
wget https://www.emqx.com/zh/downloads/broker/5.3.1/emqx-5.3.1-ubuntu20.04-arm64.deb
```  
##2安装  
```
sudo apt install ./emqx-5.3.1-ubuntu20.04-arm64.deb  
```
##3启动
```  
sudo systemctl start emqx  
```