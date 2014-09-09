/*
 * C program to print the system statistics like system uptime, 
 * total RAM space, free RAM space, process count, page size
 */

#ifndef FF_MEM_MAN_H
#define FF_MEM_MAN_H

#include <sys/sysinfo.h>    // sysinfo
#include <cstdio>
#include <iostream>
#include <unistd.h>     // sysconf

#include <cstdlib>
#include <pthread.h>

//#define FREE_MEM_UP_LIMIT 1600
//#define FREE_MEM_LOW_LIMIT 1800

#define MAX_RAM_PRESSURE_WAIT 0.5
#define MIN_RAM_PRESSURE_SIGNAL 0.6


/*
struct sys_info {
    sys_info (){}
    long hours;
    long min; 
    long sec;
    long total_ram;
    long free_ram;
    long used_ram;
    int process_count;
    long page_size;
};*/


double get_free_ram(){
  
  struct sysinfo info;
     
  if (sysinfo(&info) != 0)
     printf("sysinfo: error reading system statistics");
  
  
  //std::cerr << "Total Ram: " << info.totalram/1024 << std::endl;
  //std::cerr << "Free Ram: " << info.freeram/1024 << std::endl;
  
  return ((double)(info.freeram * info.mem_unit))/info.totalram;
  
}

/*
sys_info * get_sys_info(){
   struct sysinfo info;
   if (sysinfo(&info) != 0)
        printf("sysinfo: error reading system statistics");
   sys_info * si = new sys_info;
   si->hours= info.uptime/3600;
   si->min = info.uptime%3600/60;
   si->sec =info.uptime%60;
   si->total_ram = info.totalram/1024/1024; // megabyte
   si->used_ram = (info.totalram-info.freeram)/1024/1024;// mega byte
   si->free_ram = (info.freeram)/1024/1024;// mega byte
   si->process_count = info.procs;
   si->page_size = sysconf(_SC_PAGESIZE); //byte
return si;
} 
*/

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void pushwait(){

  pthread_mutex_lock( &count_mutex );
  //sys_info * si = get_sys_info();FREE_MEM_UP_LIMIT
  
  //if (si->free_ram < FREE_MEM_LIMIT)
  if (get_free_ram() < MAX_RAM_PRESSURE_WAIT){
    
    std::cerr << "Free RAM below limit, forcing wait " << get_free_ram() << std::endl;

    pthread_cond_wait( &condition_var, &count_mutex );
  }

  pthread_mutex_unlock( &count_mutex );	 

}
	 
void popsignal(){
  
  pthread_mutex_lock( &count_mutex );
  //sys_info * si = get_sys_info();

  //if (si->free_ram > FREE_MEM_LIMIT)
  if (get_free_ram() > MIN_RAM_PRESSURE_SIGNAL){

    std::cerr << "Free Mem above limit, continuing " << get_free_ram() << std::endl;
    pthread_cond_signal( &condition_var );
    
  }
    pthread_mutex_unlock( &count_mutex );
 
}

#endif /* FF_MEM_MAN_H */
	

/*
int main()
{
    struct sysinfo info;
     
    if (sysinfo(&info) != 0)
        printf("sysinfo: error reading system statistics");
   
    printf("Uptime: %ld:%ld:%ld\n", info.uptime/3600, info.uptime%3600/60, info.uptime%60);
    printf("Total RAM: %ld MB\n", info.totalram/1024/1024);
    printf("Free RAM: %ld MB\n", (info.totalram-info.freeram)/1024/1024);
    printf("Process count: %d\n", info.procs);
    printf("Page size: %ld bytes\n", sysconf(_SC_PAGESIZE));

    return 0;
}
*/
