#1、Emqx服务器部署在ARM64开发板上  
https://www.emqx.com/zh/downloads-and-install?product=broker&version=5.3.1&os=Ubuntu20.04&oslabel=Ubuntu+20.04  

###1下载deb包    
```
wget https://www.emqx.com/zh/downloads/broker/5.3.1/emqx-5.3.1-ubuntu20.04-arm64.deb
```  
###2安装  
```
sudo apt install ./emqx-5.3.1-ubuntu20.04-arm64.deb  
```
###3启动
```  
sudo systemctl start emqx  
```  
#2、paho.mqtt.c部署在ARM64开发板上    
1、下载  
```
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
mkdir build
cd build
cmake ..
make 
make install
```
2、可能出现问题  
```
error while loading shared libraries: libpaho-mqtt3a.so.1: cannot open shared object file: No such file or directory
```  
3、解决方法：复制下载路径下的libpaho-mqtt3a.so.1到/usr/lib路径下  
```
sudo cp /home/cat/Downloads/paho.mqtt.c/build/src/libpaho-mqtt3a.so.1 /usr/lib
```  
4、编译命令  
```
gcc mqtt.c -o mqtt -I /usr/local/include/ -lpaho-mqtt3a
```  
5、测试代码  
```
#include </home/cat/Downloads/paho.mqtt.c/src/MQTTAsync.h>  //换成你自己的安装路径，不然回报错
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define  SERVERIP "47.114.90.143"  //换成你自己的服务器
#define Clientid "hs5c1uXVkUt.Smart_home_0.0.1"    //这个随便写
#define Subtopic "/pub"
#define Pubtopic "/sensor/dht11/0"
int isConnected=0;
//消息回调函数
int onMessrecv(void* context,char*topicName,int topicLen,MQTTAsync_message*message){
            printf("recv name:\ntopic:%s,payload:%s\n",topicName,(char*)message->payload);
            MQTTAsync_free(topicName);
            MQTTAsync_free(message);
            return 1;         
}
void onSubscribe(void* context,MQTTAsync_successData* response){
    printf("sub success!");
}
//连接mqtt服务器
void onConnect(void *context,MQTTAsync_successData* response)
{
    MQTTAsync client =(MQTTAsync)context;
    int ret;
    MQTTAsync_responseOptions response_opt=MQTTAsync_responseOptions_initializer;
    printf("Succeed in connecting to mqtt-server!\n");
    response_opt.onSuccess=onSubscribe;
    ret=MQTTAsync_subscribe(client,Subtopic,1,&response_opt);//订阅/pub主题消息
    if(ret!=MQTTASYNC_SUCCESS){
        printf("fail to sub!\n");
    }
    isConnected=1;
}
void disConnect(void *context,MQTTAsync_failureData* response)
{
    printf("Failed to connect  mqtt-server!\n");
}
void onSend(void* context,MQTTAsync_successData* response){
    printf("send message to mqtt server success!\n");
}
int main()
{
    MQTTAsync client;
    int ret;
    MQTTAsync_connectOptions conn_opt = MQTTAsync_connectOptions_initializer;//初始化连接选项
    ret=MQTTAsync_create(&client,SERVERIP,Clientid,MQTTCLIENT_PERSISTENCE_NONE,NULL);
    if(ret!=MQTTASYNC_SUCCESS)
    {
        printf("Cannot create mqtt client!\n");
        return -1;
    }
    //初始化接收消息回调
    ret=MQTTAsync_setCallbacks(client,NULL,NULL,onMessrecv,NULL);
    if(ret!=MQTTASYNC_SUCCESS){
        printf("cannnot set call back function!\n");
        return  -1;
    }
    conn_opt.onSuccess=onConnect;
    conn_opt.onFailure=disConnect;
    conn_opt.automaticReconnect=1;
    conn_opt.context=client;
    conn_opt.cleansession=0;
    conn_opt.username="Smart_home_0.0.1&hs5c1uXVkUt";//设置用户名密码换成自己的
    conn_opt.password="da7739d5b4e1af2588fb80640aa29588fe892e273abd2dd934875766ecd80c20";
    ret=MQTTAsync_connect(client,&conn_opt);
    //因为是异步的，当MQTTAsync_connect返回的时候只是代表底层代码对参数进行了检查
    //当正确返回时，表示底层代码接收了该connect连接命令
    if(ret!=MQTTASYNC_SUCCESS)
    {
        printf("Cannot start a mqttt server connect!\n");
        return -1;
    }
    while (1){
        //1表示成功连接到我们的服务器
        if(isConnected==1){
            MQTTAsync_message message=MQTTAsync_message_initializer;
            MQTTAsync_responseOptions res_option=MQTTAsync_responseOptions_initializer;
            char buffer[200];
            static int temp=16;
            static int humid=58;
            sprintf(buffer," temp:%d,humid：%d ",temp++,humid++);
            message.payload=buffer;
            message.payloadlen=strlen(buffer);
            message.qos=1;
            res_option.onSuccess=onSend;
            printf("read the message is :");
            //发布消息
            ret=MQTTAsync_sendMessage(client,Pubtopic,&message,&res_option);//发送消息
            if(ret!=MQTTASYNC_SUCCESS){
                printf("参数错误！");
            }
                   sleep(10);  
                   if(temp>30|humid>90){
                       temp=16;
                       humid=58;
                   }
        }
    }
    return 0;
}
```  
