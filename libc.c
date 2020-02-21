/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

int write(int fd, char* buffer, int length){
	int result;
  __asm__ __volatile__ (
   
   "movl $4,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (fd), "c" (buffer), "d" (length)
   
    
  );
  if(result < 0){ 
    errno = -result;
    return -1;

  }
  return result;
  
}

int read(int fd, char* buffer, int count){
	int result;
  __asm__ __volatile__ (
   
   "movl $3,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (fd), "c" (buffer), "d" (count)
   
    
  );
  if(result < 0){ 
    errno = -result;
    return -1;

  }
  return result;
  
}

int gettime(){
	int result;
  __asm__ __volatile__ (
     
   "movl $10,%%eax;"
   "int $0x80;"
   : "=a" (result)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
   return result;
  
}

int getpid(){
	int result;
  __asm__ __volatile__ (
     
   "movl $20,%%eax;"
   "int $0x80;"
   : "=a" (result)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
   return result;
}

int fork(){
	int result;
	__asm__ __volatile__ (
     
   "movl $2,%%eax;"
   "int $0x80;"
   : "=a" (result)
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
   return result;
}
  

int clone (void ( *function)( void ), void * stack)  {
    int result;
	__asm__ __volatile__ (
     
   "movl $19,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (function), "c" (stack)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
  return result;
}
  
  
  
void exit(){
	int result;
	__asm__ __volatile__ (
     
   "movl $1,%%eax;"
   "int $0x80;"
   : "=a" (result)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }

}

int get_stats(int pid, struct stats *st){
	int result;
	__asm__ __volatile__ (
     
   "movl $35,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (pid), "c" (st)
      
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
   return result;

  }



void* sbrk(int increment){
	int result;
	__asm__ __volatile__ (
     
   "movl $15,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (increment)
      
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
   return result;

}
  



//SEMAPHORES


int sem_init (int n_sem, unsigned int value)  {
    int result;
	__asm__ __volatile__ (
     
   "movl $21,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (n_sem), "c" (value)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
  return result;
}


int sem_wait (int n_sem)  {
    int result;
	__asm__ __volatile__ (
     
   "movl $22,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (n_sem)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
  return result;
}


int sem_signal (int n_sem)  {
    int result;
	__asm__ __volatile__ (
     
   "movl $23,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (n_sem)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
  return result;
}


int sem_destroy (int n_sem)  {
    int result;
	__asm__ __volatile__ (
     
   "movl $24,%%eax;"
   "int $0x80;"
   : "=a" (result)
   : "b" (n_sem)
   
  );
  if(result < 0){
    errno = -result;
    result = -1;
  }
  return result;
}


char* error_list[] = {
  
  "THIS IS THE TOP OF THE WORLD"
  "Operation not permitted",
  "No such file or directory",
  "No such process",
  "Interrupted system call",
  "I/O error",
  "No such device or address",
  "Arg list too long",
  "Exec format error",
  "Bad file number",
  "No child processes",
  "Try again",
  "Out of memory",
  "Permission denied",
  "Bad address",
  "Block device required",
  "Device or resource busy",
  "File exists",
  "Cross-device link",
  "No such device",
  "Not a directory",
  "Is a directory",
  "Invalid argument",
  "File table overflow",
  "Too many open files",
  "Not a typewriter",
  "Text file busy",
  "File too large",
  "No space left on device",
  "Illegal seek",
  "Read-only file system",
  "Too many links",
  "Broken pipe",
  "Math argument out of domain of func",
  "Math result not representable",
  "Resource deadlock would occur",
  "File name too long",
  "No record locks available",
  "Function not implemented",
  "Directory not empty",
  "Too many symbolic links encountered",
  "Operation would block",
  "No message of desired type",
  "Identifier removed",
  "Channel number out of range",
  "Level 2 not synchronized",
  "Level 3 halted",
  "Level 3 reset",
  "Link number out of range",
  "Protocol driver not attached",
  "No CSI structure available",
  "Level 2 halted",
  "Invalid exchange",
  "Invalid request descriptor",
  "Exchange full",
  "No anode",
  "Invalid request code",
  "Invalid slot",
  "Bad font file format",
  "Device not a stream",
  "No data available",
  "Timer expired",
  "Out of streams resources",
  "Machine is not on the network",
  "Package not installed",
  "Object is remote",
  "Link has been severed",
  "Advertise error",
  "Srmount error",
  "Communication error on send",
  "Protocol error",
  "Multihop attempted",
  "RFS specific error",
  "Not a data message",
  "Value too large for defined data type",
  "Name not unique on network",
  "File descriptor in bad state",
  "Remote address changed",
  "Can not access a needed shared library",
  "Accessing a corrupted shared library",
  ".lib section in a.out corrupted",
  "Attempting to link in too many shared libraries",
  "Cannot exec a shared library directly",
  "Illegal byte sequence",
  "Interrupted system call should be restarted",
  "Streams pipe error",
  "Too many users",
  "Socket operation on non-socket",
  "Destination address required",
  "Message too long",
  "Protocol wrong type for socket",
  "Protocol not available",
  "Protocol not supported",
  "Socket type not supported",
  "Operation not supported on transport endpoint",
  "Protocol family not supported",
  "Address family not supported by protocol",
  "Address already in use",
  "Cannot assign requested address",
  "Network is down",
  "Network is unreachable",
  "Network dropped connection because of reset",
  "Software caused connection abort",
  "Connection reset by peer",
  "No buffer space available",
  "Transport endpoint is already connected",
  "Transport endpoint is not connected",
  "Cannot send after transport endpoint shutdown",
  "Too many references: cannot splice",
  "Connection timed out",
  "Connection refused",
  "Host is down",
  "No route to host",
  "Operation already in progress",
  "Operation now in progress",
  "Stale NFS file handle",
  "Structure needs cleaning",
  "Not a XENIX named type file",
  "No XENIX semaphores available",
  "Is a named type file",
  "Remote I/O error",
  "Quota exceeded",
  "No medium found",
  "Wrong medium type"
  
  
  };
  
  int max_error = 123;
  
  void perror() {
      char mssg[512] ;
      itoa(errno,mssg);
      /*if(errno < 0 || errno > 131){
          mssg = "Uknown error.";
      }
      else{
          mssg = error_list[errno];
      }
      if(s != NULL){
          write(1, s, strlen(s));
      }*/
  
      write(1,mssg,strlen(mssg));
      //write(1, '\n', 1);
  
}
