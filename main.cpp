#include <iostream>
#include <cstdlib>


#include "SetOS.h"
#include "SetLS.h"

#define ELEMS_NUM 200
#define THREAD_NUM 5

typedef void* (*func)(void* args);

SetOS<int>* p_SetOS = nullptr;
SetLS<int>* p_SetLS = nullptr;

void* writeOS(void* args){
	for (int i = 0; i < ELEMS_NUM; i++){
		int num = i + 1;
		p_SetOS->add(num);
	}
 	pthread_exit(0);
}

void* writeOSInverse(void* args){
	for (int i = 0; i < ELEMS_NUM; i++){
		int num = ELEMS_NUM - i;
		p_SetOS->add(num);
	}
 	pthread_exit(0);
}


void* readOS(void* args){
	int* count = reinterpret_cast<int*>(args);
	*count = 0;
	for(int i = 0; i < ELEMS_NUM; i++){
		int num = i + 1;
		if (p_SetOS->contains(num) && p_SetOS->remove(num)){
			++(*count);
		}
	}
	pthread_exit(0);
}

void* writeLS(void* args){
	for (int i = 0; i < ELEMS_NUM; i++){
		int num = i + 1;
		p_SetLS->add(num);
	}
 	pthread_exit(0);
}

void* writeLSInverse(void* args){
	for (int i = 0; i < ELEMS_NUM; i++){
		int num = ELEMS_NUM - i;
		p_SetLS->add(num);
	}
 	pthread_exit(0);
}

void* readLS(void* args){
	int* count = reinterpret_cast<int*>(args);
	*count = 0;
	for(int i = 0; i < ELEMS_NUM; i++){
		int num = i + 1;
		if (p_SetLS->contains(num) && p_SetLS->remove(num)){
			++(*count);		
		}
	}
	pthread_exit(0);
}

bool many_to_one(func p_funcRead, func p_funcWrite){
	p_SetOS = new SetOS<int>();
	p_SetLS = new SetLS<int>();
	
	pthread_t read_thread;
	pthread_t write_threads[THREAD_NUM];
	int count = 0;

	for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&write_threads[i], NULL, p_funcWrite, NULL) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}

    for(int i = 0; i < THREAD_NUM; i++){
    	pthread_join(write_threads[i], NULL);	
    }
    
    if (pthread_create(&read_thread, NULL, p_funcRead, &count) != 0)
		syslog(LOG_INFO, "E:Creating thread");
  	
  	pthread_join(read_thread, NULL);
	
    delete p_SetOS;
    delete p_SetLS;
    
    return (count == ELEMS_NUM);
}

bool one_to_many(func p_funcRead, func p_funcWrite){
    p_SetOS = new SetOS<int>();
	p_SetLS = new SetLS<int>();

	pthread_t write_thread;
	pthread_t read_threads[THREAD_NUM];
	int count[THREAD_NUM];
    if (pthread_create(&write_thread, NULL, p_funcWrite, NULL) != 0)
    	syslog(LOG_INFO, "E:Creating thread");
    pthread_join(write_thread, NULL);

    for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&read_threads[i], NULL, p_funcRead, &count[i]) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}

    for(int i = 0; i < THREAD_NUM; i++){
    	pthread_join(read_threads[i], NULL);	
    }

	int sum = 0;
    for(int i = 0; i < THREAD_NUM; i++){
    	sum += count[i];
    }
    
    delete p_SetOS;
    delete p_SetLS;
    
    return (sum == ELEMS_NUM);
}

bool many_to_many(func p_funcRead, func p_funcWrite){
	p_SetOS = new SetOS<int>();
	p_SetLS = new SetLS<int>();

	pthread_t read_threads[THREAD_NUM];
	pthread_t write_threads[THREAD_NUM];
    
	int count[THREAD_NUM];

	for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&write_threads[i], NULL, p_funcWrite, NULL) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}

    for(int i = 0; i < THREAD_NUM; i++){
    	pthread_join(write_threads[i], NULL);	
    }
    
    for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&read_threads[i], NULL, p_funcRead, &count[i]) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}

    for(int i = 0; i < THREAD_NUM; i++){
    	pthread_join(read_threads[i], NULL);	
    }

	int sum = 0;
    for(int i = 0; i < THREAD_NUM; i++){
    	sum += count[i];
    }

    delete p_SetOS;
    delete p_SetLS;   
    
    return (sum == ELEMS_NUM);
}

bool many_to_many_both(func p_funcRead, func p_funcWrite){
	p_SetOS = new SetOS<int>();
	p_SetLS = new SetLS<int>();

	pthread_t read_threads[THREAD_NUM];
	pthread_t write_threads[THREAD_NUM];
	pthread_t clean_thread;
	int count[THREAD_NUM];

	for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&write_threads[i], NULL, p_funcWrite, NULL) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}
  	
    for(int i = 0; i < THREAD_NUM; i++){
        pthread_join(write_threads[i], NULL);	
    }

  	for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&read_threads[i], NULL, p_funcRead, &count[i]) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}
  	
  	for(int i = 0; i < THREAD_NUM; i++){
    	pthread_join(read_threads[i], NULL);	
    }

    int rest_count = 0;
    if (pthread_create(&clean_thread, NULL, p_funcRead, &rest_count) != 0)
    	syslog(LOG_INFO, "E:Creating thread");
    pthread_join(clean_thread, NULL);

	int sum = 0;
    for(int i = 0; i < THREAD_NUM; i++){
    	sum += count[i];
    }

    delete p_SetOS;
    delete p_SetLS;

    return (sum == ELEMS_NUM);
 
}

bool many_to_many_different_numbers(func p_funcRead,func p_funcWrite, func p_funcWriteInverse){
	p_SetOS = new SetOS<int>();
	p_SetLS = new SetLS<int>();

	pthread_t read_threads[THREAD_NUM];
	pthread_t write_threads[THREAD_NUM];
	pthread_t clean_thread;
	int count[THREAD_NUM];

	for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&write_threads[i], NULL, i % 2 == 0? p_funcWrite: p_funcWriteInverse, NULL) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}
  	
    for(int i = 0; i < THREAD_NUM; i++){
        pthread_join(write_threads[i], NULL);	
    }

  	for(int i = 0; i < THREAD_NUM; i++){
		if (pthread_create(&read_threads[i], NULL, p_funcRead, &count[i]) != 0)
    		syslog(LOG_INFO, "E:Creating thread");
  	}
  	
  	for(int i = 0; i < THREAD_NUM; i++){
    	pthread_join(read_threads[i], NULL);	
    }

    int rest_count = 0;
    if (pthread_create(&clean_thread, NULL, p_funcRead, &rest_count) != 0)
    	syslog(LOG_INFO, "E:Creating thread");
    pthread_join(clean_thread, NULL);

	int sum = 0;
    for(int i = 0; i < THREAD_NUM; i++){
    	sum += count[i];
    }

    delete p_SetOS;
    delete p_SetLS;

    return (sum == ELEMS_NUM);
 
}

void print_result(bool res){
	res ? std::cout << "OK" : std::cout << "WRONG";
	std::cout << std:: endl;
}

int main()
{
    //LS
   	std::cout << "LS: many_to_one: ";
    print_result(many_to_one(&readLS, &writeLS));

    std::cout << "LS: one_to_many: ";
    print_result(one_to_many(&readLS, &writeLS));

	std::cout << "LS: many_to_many: ";
    print_result(many_to_many(&readLS, &writeLS));

    
    //OS
   	std::cout << "OS: many_to_one: ";
    print_result(many_to_one(&readOS, &writeOS));

    std::cout << "OS: one_to_many: ";
    print_result(one_to_many(&readOS, &writeOS));

    std::cout << "OS: many_to_many: ";
    print_result(many_to_many(&readOS, &writeOS));



    std::cout << "LS: many_to_many_both: ";
    print_result(many_to_many_both(&readLS, &writeLS));
        
    std::cout << "OS: many_to_many_both: ";
    print_result(many_to_many_both(&readOS, &writeOS));
    
    std::cout << "LS: many_to_many_both write different numbers: ";
    print_result(many_to_many_different_numbers(&readLS, &writeLS, &writeLSInverse));
    
    std::cout << "OS: many_to_many_both write different numbers: ";
    print_result(many_to_many_different_numbers(&readOS, &writeOS, &writeOSInverse));

    
    return 0;
}
