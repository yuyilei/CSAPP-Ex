#include"csapp/csapp.c"
// 展示域名和它相关联的IP之间的映射
int main( int argc , char **argv) {
    struct addrinfo *p , *listp , hints ;
    char buf[MAXLINE] ;
    int rc , flags ;
    if ( argc != 2 ){
        fprintf(stderr,"usage : %s <damain name> \n",argv[0]) ;
        exit(0) ;
    }
    memset(&hints,0,sizeof(struct addrinfo)) ;
    hints.ai_family = AF_INET ; // 限制为IPv4套接字地址
    hints.ai_socktype = SOCK_STREAM ; // 限制为一个套接字地址
    if (( rc = getaddrinfo( argv[1],NULL,&hints,&listp )) != 0 ) {
        fprintf(stderr, "getaddrinfo error: %s\n",gai_strerror(rc)) ; // 将主机名，主机地址，服务名，和端口号的字符传转化为套接字地址
        exit(1) ;
    }
    for ( p = listp ; p ; p = p->ai_next ) {
        Getnameinfo(p->ai_addr,p->ai_addrlen,buf,MAXLINE,NULL,0,flags) ; // 讲一个套接字地址转化为相应的主机和服务名字符串
        printf("%s\n",buf) ;
    }
    Freeaddrinfo(listp) ;
    return 0 ;
}
