1.	温度传感器DS18B20
//ds18b20初始化
unsigned char DS18B20Initial(void)
{
    unsigned char status = 0x00;
    unsigned int count = 0;
    unsigned char flag = 1;
    DS18B20OutputInitial();					//设置端口为输出 P0DIR |= 0x80;
    DS18B20IO = 1;      						//数据总线复位
    DS18B20Delay(260);  						//延时260ms
    DS18B20IO = 0;      						//单片机将数据总线拉低
    DS18B20Delay(700);  						//精确延时大于480us小于960us
    DS18B20IO = 1;      						//拉高总线
    DS18B20InputInitial();						//设置端口为输入 P0DIR &= 0x7f;
    while((DS18B20IO != 0)&&(flag == 1))		
    {                                      	//等待约60ms左右
        count++;
        DS18B20Delay(10);
        if(count > 8000)flag = 0;
        status = DS18B20IO;
    }
    DS18B20OutputInitial();
    DS18B20IO = 1;
    DS18B20Delay(100);
    return status;       //返回初始化状态
}
//温度读取函数，带1位小数位
float floatReadDS18B20(void) 
{
    unsigned char V1,V2;   				//定义高低8位缓冲
    unsigned int temp;     					//定义温度缓冲寄存器
    float value;
    DS18B20Initial();
    DS18B20Write(0xcc);    				//跳过读序列号的操作
    DS18B20Write(0x44);    				//启动温度转换
    DS18B20Initial();
    DS18B20Write(0xcc);    				//跳过读序列号的操作 
    DS18B20Write(0xbe);    				//读取温度寄存器
    V1 = DS18B20Read();    				//低位
    V2 = DS18B20Read();    				//高位 
    temp=V2*0xFF+V1;
    value = temp*0.0625;
    return value;}

2.	温湿度传感器DHT11
void DHT11Read(void)    //温湿写入
{     
    unsigned char i;         
    for(i=0;i<8;i++)    
    {
        unsigned char flag=2; 
        while((!pin)&&flag++);
        Delay_10us();
        Delay_10us();
        Delay_10us();
        unsigned char temp=0;
        if(pin)unsigned char temp=1;
        flag=2;
        while((pin)&&unsigned flag++);   
        if(flag==1)break;    
        unsigned char dht11data<<=1;
        dht11data|=temp; 
    }    
}

void DHT11Initial(void)   //传感器启动函数
{
    pin=0;
    Delay_ms(20);  	//>18ms
    pin=1; 
    P0DIR &= ~0x20; 	//重新配置I/O口方向
    Delay_10us();
    Delay_10us();                        
    Delay_10us();
    Delay_10us();  
    if(!pin) 
    {
        unsigned char flag=2; 
        while((!pin)&&flag++);
        flag=2;
        while((pin)&&flag++); 
        DHT11Read();
        unsigned char data_H_temp=dht11data;
        pin=1;   
        humidity_decade=data_H_temp/10; 
        humidity_unit=data_H_temp%10;        
    } 
    else 				//没有成功读取，返回0
    {
        humidity_decade=0; 
        humidity_unit=0;  
    } 
    P0DIR |= 0x20; 	//IO口需要重新配置 
}
3.	ZigBee网络通信代码
//任务初始化
void SampleApp_Init( uint8 task_id )
{ 
  SampleApp_TaskID = task_id;
  SampleApp_NwkState = DEV_INIT;
  SampleApp_TransID = 0;
  MT_UartInit();                  							//串口初始化
  MT_UartRegisterTaskID(task_id); 								//注册串口任务
  P0SEL &= 0x5f;                  							//P0_5,P0_7配置成通用I/O
  SampleApp_P2P_DstAddr.addrMode = (afAddrMode_t)Addr16Bit; 	//点播 
  SampleApp_P2P_DstAddr.endPoint = SAMPLEAPP_ENDPOINT; 
  SampleApp_P2P_DstAddr.addr.shortAddr = 0x0000;            	//发给协调器
  SampleApp_epDesc.endPoint = SAMPLEAPP_ENDPOINT;
  SampleApp_epDesc.task_id = &SampleApp_TaskID;
  SampleApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&SampleApp_SimpleDesc;
  SampleApp_epDesc.latencyReq = noLatencyReqs;
  afRegister( &SampleApp_epDesc );
  RegisterForKeys( SampleApp_TaskID );
  SampleApp_Group.ID = 0x0001;
  osal_memcpy( SampleApp_Group.name, "Group 1", 7 );
  aps_AddGroup( SAMPLEAPP_ENDPOINT, &SampleApp_Group );}
//终端节点采集数据发送代码
void SampleApp_Send_P2P_Message( void )
{
  byte humidity[3], strTemp[11];
  byte endDeviceID[3];
  char cTemp[4];
  float fTemp;
  fTemp = floatReadDS18B20();
  sprintf(cTemp, "%.01f", fTemp);
  endDeviceID[0] = SAMPLEAPP_DEVICEID/10 + 48;
  endDeviceID[1] = SAMPLEAPP_DEVICEID%10 + 48;
  endDeviceID[2] = '\0';
  DHT11Initial();             //获取湿度
  humidity[0] = humidity_decade+0x30;
  humidity[1] = humidity_unit+0x30;
  humidity[2] = '\0';
  //将数据整合后方便发给协调器显示
  osal_memcpy(strTemp, endDeviceID, 2);
  osal_memcpy(&strTemp[2], " ", 1);
  osal_memcpy(&strTemp[3], cTemp, 4);
  osal_memcpy(&strTemp[7], " ", 1);
  osal_memcpy(&strTemp[8], humidity, 3);
  //获得的温湿度通过串口输出到电脑显示
  HalUARTWrite(0, strTemp, 10);
  HalUARTWrite(0, "\n",1);
  if ( AF_DataRequest( &SampleApp_P2P_DstAddr, &SampleApp_epDesc,
                       SAMPLEAPP_P2P_CLUSTERID,
                       10,
                       strTemp,
                       &SampleApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
  }
}
//协调器接受代码
void SampleApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case SAMPLEAPP_P2P_CLUSTERID:
      //HalUARTWrite(0, "T&H:", 4);       					//提示接收到数据
      HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength); 	//输出接收到的数据
      HalUARTWrite(0, "\n", 1);         					//回车换行
      break;    
  }}

4.	STM32 数据WIFI上传代码 
#include "esp8266.h"
int is_con_slave=0;//标记是否连接到从机
int rn_count = 0;//对接收消息中的换行符计数，收到3个\r\n即为正常结束
char wifi_feedback[200];//用于存放接收数据的数组
int data_i=0;//接收数据数组的索引
int time_wait=3;//接收数据等待的时间(3秒)
int receive_state=0;//为0表示还没有收到数据  为1表示正在接收数据 为2表示接收数据的过程正常结束
u8 fac_us=72000000/8000000;	
u16 fac_ms=72000000/8000000*1000;
	
void Net_Init()
{
	//等待模块自身启动完成
	delay_ms(1000);
	
	//关闭命令回显
	printf("ATE0\r\n");
	if(Re(10)==1);//Close--show---OK
	else{return ;}WiFi-Init--ERROR
	delay_ms(1000);

	//作为主机连接WiFi热点
	printf("AT+CWJAP=\"Wifi-ap\",\"protject-update\"\r\n");
	
	if(Re(10)==1);//WiFi--Access--OK
	else{return ;}//WIFI-------ERROR
	delay_ms(1000);
	
		
	//作为主机-设置AP热点供从机接入
	printf("AT+CWSAP=\"lift-project\",\"labcat127\",1,4,4\r\n");
	
	if(Re(10)==1);//Ad hoc--Init--OK
	else{return ;}//Adhoc-Init-ERROR
	delay_ms(1000);

	//设置主机AP模式下的IP
	printf("AT+CIPAP=\"192.168.4.1\"\r\n");
	
	if(Re(10)==1);//Ad hoc--Init--OK
	else{return ;}//Adhoc-Init-ERROR
	delay_ms(1000);

	//主机设置为多连接
	printf("AT+CIPMUX=1\r\n");
	
	if(Re(10)==1);//Ad hoc--Init--OK
	else{ return ;}//Adhoc-Init-ERROR
	delay_ms(1000);
	
	//主机开启服务器模式，端口为8088
	printf("AT+CIPSERVER=1,8088\r\n");
	
	if(Re(10)==1);//Server--Init--OK
	else{ return ;}//Server-Init-ERROR
	delay_ms(1000);
		
	//测试与SensorWeb服务器的链接
	printf("AT+PING=\"115.28.147.177\"\r\n");
	if(Re(10)==1);//Ping--Server--OK
	else{return ;}//PingServer-ERROR
	delay_ms(1000);
	
	//与服务器建立连接
	printf("AT+CIPSTART=0,\"TCP\",\"115.28.147.177\",80\r\n");
	if(Re(10)==1);//Connect-Sever-OK
	else{return ;}//Con-Sever--ERROR
	delay_ms(1000);
	
	//WiFi模块初始化完成，测试全部成功
	
	//开启串口接受中断--接收从机的连接信息
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

		
}

//用于处理wifi返回的数据
int Re(int seconds)
{
	u32 temp;
	u8 data;
	u16 nms=1000;
	int times=seconds;
	data_i=0;
	
	while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET );//等待第一个字节
	
	while(times--)
	{
		SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
		SysTick->VAL =0x00;							//清空计数器
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
		do
		{
			if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
			{
				data = USART_ReceiveData(USART3);
				wifi_feedback[data_i]=data ;
				//OK
				if( wifi_feedback[data_i] == 'K' && data_i>=1 && 
					wifi_feedback[data_i-1] == 'O' )
				{
					SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
					SysTick->VAL =0X00;       					//清空计数器
					//返回成功信息
					return 1;
				}
				//ERROR
				if( wifi_feedback[data_i] == 'R' && data_i>=4 && 
					wifi_feedback[data_i-1] == 'O' &&
					wifi_feedback[data_i-2] == 'R' &&
					wifi_feedback[data_i-3] == 'R' &&
					wifi_feedback[data_i-4] == 'E' 
				)
				{
					SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
					SysTick->VAL =0X00;       					//清空计数器
					//返回失败信息
					return 0;
				}
				
				//接收成功-重新装载倒计时秒数
				times=seconds;
				data_i++;
			}
			
			
			temp=SysTick->CTRL;
		}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
		SysTick->VAL =0X00;       					//清空计数器	
	}
	return 0;
}


void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	u32 temp;
	u8 data;
	u16 nms=1000;
	int times=10;
	data_i=0;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//失能接收中断--准备处理连续字符
		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
		while(times--)
		{
			SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
			SysTick->VAL =0x00;							//清空计数器
			SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
			do
			{
				if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
				{
					data = USART_ReceiveData(USART3);
					wifi_feedback[data_i]=data ;
					//CONNECT
					if( wifi_feedback[data_i] == 'T' && data_i>=6 && 
						wifi_feedback[data_i-1] == 'C' &&
						wifi_feedback[data_i-2] == 'E' &&
						wifi_feedback[data_i-3] == 'N' &&
						wifi_feedback[data_i-4] == 'N' &&
						wifi_feedback[data_i-5] == 'O' &&
						wifi_feedback[data_i-6] == 'C' )
					{
						SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
						SysTick->VAL =0X00;       					//清空计数器	
						//返回成功信息
						is_con_slave=1;
						//WIFI-从机连接成功
						return ;
					}
					//ERROR
					if( wifi_feedback[data_i] == 'R' && data_i>=4 && 
						wifi_feedback[data_i-1] == 'O' &&
						wifi_feedback[data_i-2] == 'R' &&
						wifi_feedback[data_i-3] == 'R' &&
						wifi_feedback[data_i-4] == 'E' 
					)
					{
						SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
						SysTick->VAL =0X00;       					//清空计数器	
						//返回失败信息
						is_con_slave=0;
						//WIFI-从机连接失败
						//连接从机失败--重新使能接受中断--准备下次连接
						USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
						return ;
					}
					
					//接收成功-重新装载倒计时秒数
					times=10;
					data_i++;
				}
				
				
				temp=SysTick->CTRL;
			}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
			SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
			SysTick->VAL =0X00;       					//清空计数器	
		}
		
     } 
	is_con_slave=0;
	//连接从机失败--重新使能接受中断--准备下次连接
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
} 

int Receive()
{
	time_wait=6;
	//receive_state=0;
	data_i=0;
	rn_count = 0;
	while(receive_state==0);//等待数据发来
	while(receive_state==1)//从接收第一个字节的数据开始，超过6秒钟则认为接受失败
	{
		delay_ms(1000);
		time_wait--;
		if(time_wait==0){return 0;}
		if(receive_state==2){receive_state=0;return 1;}
	}
	return 1;
}

