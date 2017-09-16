#include"../csapp/csapp.c" 
int main(int argc, char ** argv){
    struct in_addr inaddr ; 
    int rc ; 
    if ( argc != 2 ){
        fprintf(stderr,"usage: %s <dotted-decimal>\n",argv[0]) ; 
        exit(0) ; 
    } 
    rc = inet_pton(AF_INET,argv[1],&inaddr) ; 
    if ( rc == 0 ) 
        app_error("inet_pton error: invaild dotted-decimal address") ; // 非法的点十进制  
    else if ( rc < 0 ) 
        unix_error("inet_pton error") ; // 出错  
    printf("0x%x\n",ntohl(inaddr.s_addr)) ; 
}