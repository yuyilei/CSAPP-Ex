#include"csapp/csapp.c"
void doit(int fd) ;
void read_requesthdrs(rio_t *rp) ;
int parse_uri(char *uri , char *filename , char *cgiargs ) ;
void serve_static(int fd , char *filename , int filesize ) ;
void get_filetype(char *filename , char *filetype ) ;
void serve_dynamic(int fd , char *filename , char *cgiargs ) ;
void clienterror(int fd , char  *cause , char *errnum , char *shortmsg , char *longmsg ) ;

int main(int argc , char **argv ) {
	int listenfd , connfd ;
	char hostname[MAXLINE] , port[MAXLINE] ;
	socklen_t clientlen ;
	struct sockaddr_storage clientaddr ;

	if ( argc != 2 ) {
		fprintf(stderr , "usage : %s <port> \n",argv[0]) ;
		exit(1) ;
	}

    listenfd = Open_listenfd(argv[1]) ;
    while (1) {
        clientlen = sizeof(clientaddr) ;
        connfd = Accept(listenfd, (SA *)&clientaddr , & clientlen ) ;
        Getnameinfo((SA*) &clientaddr , clientlen , hostname , MAXLINE , port , MAXLINE , 0 ) ;
        printf("Accpet connection from (%s ,%s)\n",hostname,port) ;
        doit(connfd) ;
        Close(connfd) ;
    }
    return 0 ;
}
// 处理http事物,只接受GET
void doit(int fd ) {
    int is_static ;
    struct stat sbuf ;
    char buf[MAXLINE] , method[MAXLINE] , uri[MAXLINE] , version[MAXLINE] ;
    char filename[MAXLINE] , cgiargs[MAXLINE] ;
    rio_t rio ;

    // read request line and headers
    Rio_readinitb(&rio,fd) ;
    Rio_readlineb(&rio, buf , MAXLINE ) ;
    printf("Request headers :\n") ;
    printf("%s",buf) ;
    sscanf(buf,"%s %s %s",method,uri,version) ;
    if (strcasecmp(method,"GET")) {
        clienterror(fd, method , "501" , "Not implemented","Tiny does not implement this method") ;
        return ;
    }
    read_requesthdrs(&rio) ;

    // parse URI from GET request
    is_static = parse_uri(uri, filename , cgiargs ) ;
    if ( stat(filename, &sbuf) < 0 ) {
        clienterror(fd,filename ,"404", "Not found" , "Tiny could not find this file") ;
        return ;
    }
    if ( is_static ) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode )) {
            clienterror(fd,filename,"403","Forbidden","Tiny could not read this file") ;
            return ;
        }
        serve_static(fd,filename,sbuf.st_size) ;
    }
    else {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode )) {
            clienterror(fd,filename,"403","Forbidden","Tiny cound not  run the CGI program") ;
            return ;
       }
        serve_dynamic(fd,filename,cgiargs) ;
	}
}

// 发送http相应到客户端
void clienterror( int fd, char  *cause , char *errnum , char *shortmsg , char *longmsg ) {
    char buf[MAXLINE] , body[MAXLINE] ;

    // build the http response ' s body
    sprintf(body,"<html><title>Tiny Error</title>") ;
    sprintf(body,"%s<body bgcolor=""ffffff"">/r/n",body) ;
    sprintf(body,"%s%s: %s\r\n",body,errnum,shortmsg) ;
    sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause) ;
    sprintf(body,"%s<hr><em>The Tiny Web server</em>\r\n",body) ;

    // print the http response
    sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum,shortmsg) ;
    Rio_writen(fd,buf,strlen(buf)) ;
    sprintf(buf,"Content-type: text/html\r\n") ;
    Rio_writen(fd,buf,strlen(buf)) ;
    sprintf(buf,"Content-length: %d\r\n\r\n",(int)strlen(body)) ;
    Rio_writen(fd,buf,strlen(buf)) ;
    Rio_writen(fd,body,strlen(buf))  ;
}

// 读取报文信息
void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE] ;
    Rio_readlineb(rp,buf,MAXLINE) ;
    while (strcmp(buf,"\r\n")) {
        Rio_readlineb(rp,buf,MAXLINE) ;
        printf("%s",buf) ;
    }
}

int parse_uri(char *uri , char *filename , char *cgiargs ) {
    char *ptr ;
    if ( !strstr(uri, "cgi-bin")) {
        strcpy(cgiargs,"") ;
        strcpy(filename,".") ;
        strcat(filename,uri) ;
        if ( uri[strlen(uri)-1] == '/')
            strcat(filename,"home.html") ;
        return 1 ;
    }
    ptr = index(uri,'?') ;
    if (ptr) {  // 抽取CGI参数
        strcpy(cgiargs,ptr+1) ;
        *ptr = '\0' ;
    }
    else
        strcpy(cgiargs,"") ;
    strcpy(filename,".") ;
    strcat(filename,uri) ;
    return 0 ;
}

// 返回http响应,包含静态文件
void serve_static(int fd, char *filename , int filesize) {
    int srcfd ;
    char *srcp , filetype[MAXLINE] , buf[MAXLINE] ;

// send response headers to client
    get_filetype(filename,filetype) ;
    sprintf(buf,"HTTP/1.0 200 OK\r\n") ;
    sprintf(buf,"%sServer: Tiny Web Server\r\n",buf) ;
    sprintf(buf,"%sConnection: close \r\n",buf) ;
    sprintf(buf,"%sContent-length: %d\r\n",buf,filesize) ;
    sprintf(buf,"%sConten-type: %s\r\n",buf,filetype) ;
    Rio_writen(fd,buf,strlen(buf)) ;
    printf("Response headers:\n") ;
    printf("%s",buf) ;

// send response body to client
    srcfd = Open(filename,O_RDONLY,0) ; // 以读的方式打开filename , 获取filename的描述符
    srcp = Mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0) ; // 将srcfd的前file size个字节映射到从srcfd开始的私有只读内存区域
    Close(srcfd) ; // 文件映射到内存，就不需要描述符了，所以关闭文件
    Rio_writen(fd,srcp,filesize) ;  // 将从srcfd开始的file size个字节复制到客户端已连接的描述符
    Munmap(srcp,filesize) ; // 释放映射的虚拟内存区域
}

void get_filetype(char *filename , char *filetype ) {
    if ( strstr(filename,".html")) {
        strcpy(filetype,"text/html") ;
    }
    else if (strstr(filename,".gif")) {
        strcpy(filetype,"image/gif") ;
    }
    else if (strstr(filename,".png")) {
        strcpy(filetype,"image/png") ;
    }
    else if (strstr(filename,".jpg")){
        strcpy(filetype,"image/jpg") ;
    }
    else
        strcpy(filetype,"text/plain") ;
}

// 派生子进程在上下文中运行CGI程序
void serve_dynamic(int fd , char *filename , char *cgiargs ){
	char buf[MAXLINE] , *emptylist[] = { NULL } ;

// return first part of http response
	sprintf(buf,"HTTP/1.0 200 OK\r\n") ;
	Rio_writen(fd ,buf , strlen(buf) ) ;
	sprintf(buf,"Server: Tiny Web Server\r\n") ;
	Rio_writen(fd, buf, strlen(buf) ) ;

	if ( Fork() == 0 ) { // child
		setenv("QUERY_STRING",cgiargs, 1 ) ;
		Dup2(fd ,STDOUT_FILENO) ;  // redirect stdout to client , 重定向子进程的标准输出到已连接的的文件链接符
		Execve(filename, emptylist,environ) ; // RUN CGI
	}
    Wait(NULL) ; // parent
}
