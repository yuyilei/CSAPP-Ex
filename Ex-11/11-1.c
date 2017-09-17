#include"../csapp/csapp.c" 
int main(int argc, char **argv){
    struct in_addr inaddr ; 
    uint32_t addr ; 
    char buf[MAXBUF] ; 
    if ( argc != 2 ){
        fprintf(stderr,"usage: %s <hex number>\n",argv[0]) ; 
        exit(0) ; 
    } 
    sscanf(argv[1],"%x",&addr) ; // 将字符读入16进制 
    inaddr.s_addr = htonl(addr) ; 
    if ( !inet_ntop(AF_INET,&inaddr,buf,MAXBUF) )  // AF_INET表示32位，AF_INET6表示128位
        unix_error("inet_ntop") ; 
    printf("%s\n",buf) ; 
    exit(0) ; 
}