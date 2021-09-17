#include "myfun.h"

void package_init_s(Package_S* psp,uint8_t cmd,uint8_t flag){
    psp->head=HEAD;
    psp->tail=TAIL;
    psp->cmd=cmd;
    psp->flag=flag;
    memset(psp->info,0,sizeof(psp->info));
}

void package_init_m(Package_M* pmp,uint8_t cmd,uint8_t flag){
    pmp->head=HEAD;
    pmp->tail=TAIL;
    pmp->cmd=cmd;
    pmp->flag=flag;
    pmp->hw_len=0;
    memset(pmp->hw_dat,0,sizeof(pmp->hw_dat));
    pmp->sw_len=0;
    memset(pmp->sw_dat,0,sizeof(pmp->sw_dat));
    pmp->fc_len=0;
    memset(pmp->fc_dat,0,sizeof(pmp->fc_dat));
    pmp->checksum=0;    
}

void package_init_l(Package_L* plp,uint8_t cmd,uint8_t flag){
    plp->head=HEAD;
    plp->tail=TAIL;
    plp->cmd=cmd;
    plp->flag=flag;
    plp->len=0;
    memset(plp->data,0,sizeof(plp->data));   
    plp->checksum=0;
}


int get_sock(){
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock==-1){perror("socket error\n");exit(-1);}
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in local;
	memset(&local,0,sizeof(local));
    local.sin_family=AF_INET;
	local.sin_port=htons(_PORT);
	local.sin_addr.s_addr=inet_addr(_IP);
	if(bind(sock,(struct sockaddr*)&local,sizeof(local))==-1){perror("bind error\n"); exit(-2);}
	if(listen(sock,1000)==-1){perror("listen error\n");exit(-3);}
	return sock;
}

int seek_sock(){
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(_PORT);
    server_addr.sin_addr.s_addr=inet_addr(_IP);
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1){perror("socket error\n");return -1;}
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){perror("connect error");return -1;}
    printf("[Daemon]\tDaemon client connected\n");
    return sockfd;
}

void get_checksum(int* buf, int size){
    int cnt=0;
    int check_sum=0;
    while(cnt<size/4){
        check_sum^=buf[cnt++];
    }
    buf[cnt-2]=check_sum;
}

int do_checksum(int* buf, int size){
    int cnt=0;
    int check_sum=0;
    while(cnt<size/4){
        check_sum^=buf[cnt++];
        //printf("buf %x,checksum %x\n",buf[cnt],check_sum);
    }
    if(check_sum==0) return 0;
    else return -1;
}

int transferd(int rfd,int wfd){
    int read_ret;
    int sumd=0;
    char rbuf[2048];
    while((read_ret=read(rfd,rbuf,sizeof(rbuf)))!=0){
        if(read_ret==-1) {perror("read error\n");return -1;}
        int write_ret=write(wfd,rbuf,read_ret);
        if(write_ret!=read_ret) {perror("write error\n");return -1;}
        sumd+=write_ret;
    }
    return sumd;
}

void writelog(const char* log)
{
    time_t tDate;
    struct tm* eventTime;
    time(&tDate);//得到系统当前时间
    //将time_数据类型转换为struct tm结构
    eventTime = localtime(&tDate);
    //年，以1900年作为起始值。如果当前年为1991，则year变量=1
    int year = eventTime->tm_year + 1900;
    //月：tm_mon从0开始计算
    int month = eventTime->tm_mon + 1;
    //日：
    int day = eventTime->tm_mday;
    //小时
    int hour = eventTime->tm_hour;
    //分钟
    int minute = eventTime->tm_min;
    //秒
    int second = eventTime->tm_sec;
 
    char sDate[16];
    sprintf(sDate, "%04d-%02d-%02d ", year,month,day);
    char sTime[16];
    sprintf(sTime, "%02d:%02d:%02d", hour, minute, second);
    char s[1024];
    sprintf(s, "%s %s %s\n", sDate, sTime, log);
 
    FILE* fp = fopen("my.log", "a+");
    if (fp==NULL)
    {
        printf("log write error :%s", strerror(errno));
    }
    else
    {
        fputs(s, fp);
        fclose(fp);
    }
    return;
}