#include </home/cat/Downloads/paho.mqtt.c/src/MQTTAsync.h>  //换成你自己的安装路径，不然回报错
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <iostream>
#include <sys/ioctl.h>
#include <signal.h>
#include <mutex>
#include <condition_variable>

#include "cJSON.h"
#include "ZLogSystem.h"

#define SERVERIP "47.114.90.143"  //换成你自己的服务器
#define Clientid "hs5c1uXVkUt.Smart_home_0.0.1"    //这个随便写
#define Subtopic "/pub"
#define Pubtopic "/topic/qos0"
#define Username "Smart_home_0.0.1"
#define Userpasswd "da7739d5b4e1"

#define BUFFER_SIZ (1024)

std::mutex mtx;
std::condition_variable cv;
LogSystem* logger = LogSystem::getInstance();
cJSON *root;

//消息回调函数
int onMessrecv(void* context,char*topicName,int topicLen,MQTTAsync_message*message){
            int rcv_len;
            //printf("recv topic:%s,payload:%s\n",topicName,(char*)message->payload);
            
            logger->log_Info("MQTT Topic:");
            logger->log_Info(topicName);
            logger->log_Info("\n");
            logger->log_Info("MQTT Payload:");
            logger->log_Info((char*)message->payload);
            logger->log_Info("\n");

            root = cJSON_Parse((char*)message->payload);  

            rcv_len = message->payloadlen;
            MQTTAsync_free(topicName);
            MQTTAsync_free(message);
            cv.notify_all();
            return 1;         
}
void onSubscribe(void* context,MQTTAsync_successData* response){
    logger->log_Info("sub success!\n");
}
//连接mqtt服务器
void onConnect(void *context,MQTTAsync_successData* response)
{
    MQTTAsync client =(MQTTAsync)context;
    int ret;
    MQTTAsync_responseOptions response_opt=MQTTAsync_responseOptions_initializer;
    logger->log_Info("Succeed in connecting to mqtt-server!\n");
    response_opt.onSuccess=onSubscribe;
    ret = MQTTAsync_unsubscribe(client, Pubtopic, NULL);
    ret = MQTTAsync_subscribe(client,Subtopic,1,&response_opt);//订阅/pub主题消息
    if(ret!=MQTTASYNC_SUCCESS){
        printf("fail to sub!\n");
    }
}
void disConnect(void *context,MQTTAsync_failureData* response)
{
    logger->log_Info("Failed to connect mqtt-server!\n");
}
void onSend(void* context,MQTTAsync_successData* response){
    logger->log_Info("send message to mqtt server success!\n");
}


void sigint_handler(int signo) {
    std::cout<<"\nCaught Ctrl+C signal. Executing custom code...\n"<<std::endl;
    // 在这里执行你想要的代码
    std::cout<<"退出MQTT_Client\n"<<std::endl;
    // 退出程序
    exit(0);
}


class MQTT_Thread
{
public:
    bool Init();
    bool handle_Message(std::condition_variable &cv);
    bool publish_Message(char* str_message);
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opt;
    MQTTAsync_message message = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions res_option=MQTTAsync_responseOptions_initializer;
};


bool MQTT_Thread::Init()
{
    int ret;
    conn_opt = MQTTAsync_connectOptions_initializer;//初始化连接选项
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
    conn_opt.username=Username;//设置用户名密码换成自己的
    conn_opt.password=Userpasswd;
    ret=MQTTAsync_connect(client,&conn_opt);
    //因为是异步的，当MQTTAsync_connect返回的时候只是代表底层代码对参数进行了检查
    //当正确返回时，表示底层代码接收了该connect连接命令
    if(ret!=MQTTASYNC_SUCCESS)
    {
        printf("Cannot start a mqttt server connect!\n");
        return -1;
    }
    return true;
}


bool MQTT_Thread::publish_Message(char* str_message)
{
    int ret;
    message.payload=str_message;
    message.payloadlen=strlen(str_message);
    message.qos=1;
    res_option.onSuccess=onSend;
    //printf("read the message is :");
    //发布消息
    ret=MQTTAsync_sendMessage(client,Pubtopic,&message,&res_option);//发送消息
    if(ret!=MQTTASYNC_SUCCESS)
    {
        printf("主线程的参数错误！");
    }
    return true;
}

void find_cJSON_All_Keys(cJSON* obj) {
   cJSON* child = obj->child;
    while (child) {
        if (child->type == cJSON_String) { // 确保是键（在cJSON的对象结构中，键总是字符串类型）
            logger->log_Info("键:");
            logger->log_Info(child->string);
            logger->log_Info("\n");
            if(strcmp(child->string,"key") == 0)
            {
                logger->log_Info("值:");
                logger->log_Info(child->valuestring);
                logger->log_Info("\n");
            }
        }
        child = child->next;
    }
}

bool MQTT_Thread::handle_Message(std::condition_variable &cv)
{
    while(1)
    {
        std::unique_lock<std::mutex> uni_lock(mtx);
        logger->log_Info("wait notify!\n");
        cv.wait(uni_lock);
        if(root == NULL || root->type != cJSON_Object)
        {
            logger->log_Warning("json_value_format wrong\n");
            continue;
        }
        else
        {
            find_cJSON_All_Keys(root);
        }
    }
    return true;
}

int main()
{
    logger->setLogLevel(LogSystem::level_info);
    MQTT_Thread thread1;
    char str_message[12] = "reallygood"; 
    thread1.Init();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    thread1.publish_Message(str_message);
       // 注册信号处理函数

    if (signal(SIGINT, sigint_handler) == SIG_ERR) 
    {
        perror("Error registering signal handler");
        return 1;
    }
    std::thread t1(&MQTT_Thread::handle_Message, std::ref(thread1), std::ref(cv));
    t1.detach();
    std::getchar();
    return 0;
}
 
 