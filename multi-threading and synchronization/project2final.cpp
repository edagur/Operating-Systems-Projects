#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <mutex>
#include <chrono>
#include <queue>
#include <algorithm>    
using namespace std;

//We assigned default values to the parameters in case the user didn't provide any
double probability=0.5;
int simulation_time=5;
int seed=time(NULL);
int train_id=0;
struct timeval start_time;
struct timeval system_overload_start_time;

//mutexes
pthread_mutex_t train_id_mutex;
pthread_mutex_t AC_outcoming_mutex;
pthread_mutex_t BC_outcoming_mutex;
pthread_mutex_t DE_outcoming_mutex;
pthread_mutex_t DF_outcoming_mutex;
pthread_cond_t AC;
pthread_cond_t BC;
pthread_cond_t DE;
pthread_cond_t DF;
bool clear;

//Train class is defined to be able to use it for log,as parameter arrival time,
// departure time, train id,train length, starting and destination point exist.

class Train{
	public:
	struct timeval arrival_time;
	struct timeval departure_time;
	int train_id;
	int length;
	char starting_point;
	char destination_point;
	Train(struct timeval, struct timeval, int, int, char, char);
};
  
Train::Train(struct timeval t1, struct timeval t2, int a,  int b, char c,  char d){
	arrival_time=t1;
	departure_time=t2;
	train_id=a;
	length=b;
	starting_point=c;
	destination_point=d;
};

//train queues exist for keep and decide which train from which section will be in the tunnel(controller) section
std::queue<Train>SectionACoutcomingqueue;
std::queue<Train>SectionBCoutcomingqueue;
std::queue<Train>SectionDEoutcomingqueue;
std::queue<Train>SectionDFoutcomingqueue;

//section datas
 struct thread_data{
	 public:
	 char start;
	 char dest1;
	 char dest2;
	 pthread_mutex_t *section_mutex;
	 pthread_cond_t *section_cond;
	 queue<Train> *sectionqueue;
};
struct thread_data thread_data_array[4];

//Train Event is defined to be able to use it for log,as parameter event time, event type, train id,waiting trains as string exist.
class Event{
	public:
	std::string event_type;
	struct timeval event_time;
	int train_id;
	std::string trains_waiting;
	Event(std::string, struct timeval, int,std::string);
};
 
Event::Event(std::string a, struct timeval b, int c,std::string d){
	event_type=a;
	event_time=b;
	train_id=c;
	trains_waiting=d;
};
std::queue<Event>events_log;

//for priority on entering the tunnel, it checks which has the priority in the case of a tie.
struct QueueCompare{
    bool operator()(const queue<Train> &t1, const queue<Train> &t2) const{
        int t1value = t1.size();
        int t2value = t2.size();
        char starting1 =t1.front().starting_point;
        char starting2 =t2.front().starting_point;
        if(t1value==t2value&&t1value!=0) return starting1>starting2;
        return t1value < t2value;
    }
};
//To list the trains in arrival time order
struct LogCompare{
    bool operator()(const Train &t1, const Train &t2) const{
        long int t1value = t1.arrival_time.tv_sec;
        long int t2value = t2.arrival_time.tv_sec;
        return t1value > t2value;
    }
};
std::priority_queue<Train, vector<Train>, LogCompare> train_logs;

//Pthread sleep is given.
int pthread_sleep (int seconds){
	pthread_mutex_t mutex;
	pthread_cond_t conditionvar;
	struct timespec timetoexpire;
	if(pthread_mutex_init(&mutex,NULL)){
		return -1;
	}
	if(pthread_cond_init(&conditionvar,NULL)){
		return -1;
	}
	struct timeval tp;
	//When to expire is an absolute time, so get the current time and add it to our delay time
	gettimeofday(&tp, NULL);
	timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;

	pthread_mutex_lock (&mutex);
	int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
	pthread_mutex_unlock (&mutex);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&conditionvar);
	//Upon successful completion, a value of zero shall be returned
	return res;
}

//method to write the trains in waiting for tunnel pass
string trains_waiting(int train_id){
	string trains_waiting;
	std::queue<Train>SectionACoutcomingqueuetemp=SectionACoutcomingqueue;
	std::queue<Train>SectionBCoutcomingqueuetemp=SectionBCoutcomingqueue;
	std::queue<Train>SectionDEoutcomingqueuetemp=SectionDEoutcomingqueue;
	std::queue<Train>SectionDFoutcomingqueuetemp=SectionDFoutcomingqueue;

	//trains in AC
	while(!SectionACoutcomingqueuetemp.empty()){
		if(SectionACoutcomingqueuetemp.front().train_id!=train_id){
			trains_waiting+=to_string(SectionACoutcomingqueuetemp.front().train_id)+", ";
		}
		SectionACoutcomingqueuetemp.pop();
	}
	//trains in BC
	while(!SectionBCoutcomingqueuetemp.empty()){
		if(SectionBCoutcomingqueuetemp.front().train_id!=train_id){
			trains_waiting+=to_string(SectionBCoutcomingqueuetemp.front().train_id)+", ";
		}
		SectionBCoutcomingqueuetemp.pop();
	}
	//trains in DE
	while(!SectionDEoutcomingqueuetemp.empty()){
		if(SectionDEoutcomingqueuetemp.front().train_id!=train_id){
			trains_waiting+=to_string(SectionDEoutcomingqueuetemp.front().train_id)+", ";
		}
		SectionDEoutcomingqueuetemp.pop();
	}
	//trains in DF
	while(!SectionDFoutcomingqueuetemp.empty()){
		if(SectionDFoutcomingqueuetemp.front().train_id!=train_id){
			trains_waiting+=to_string(SectionDFoutcomingqueuetemp.front().train_id)+", ";
		}
		SectionDFoutcomingqueuetemp.pop();
	}
	return trains_waiting.substr(0, trains_waiting.size()-2);
}


void *Controller(void *v){
	struct timeval current_time;
	gettimeofday(&current_time,NULL);
	while(current_time.tv_sec<start_time.tv_sec+ simulation_time){// check if total number of trains in 4 sections exceeds the permitted number, 

		// check section queues, based on priority pick a train to pass the tunnel section
		std::queue<Train>max_queue;
		priority_queue<queue<Train>, vector<queue<Train>>, QueueCompare> pq;
		
		
		pthread_mutex_lock(&AC_outcoming_mutex);
		pthread_mutex_lock(&BC_outcoming_mutex);
		pthread_mutex_lock(&DE_outcoming_mutex);
		pthread_mutex_lock(&DF_outcoming_mutex);
		
		
	     	pthread_cond_wait(&AC, &AC_outcoming_mutex);
	     	pthread_cond_wait(&BC, &BC_outcoming_mutex);
	     	pthread_cond_wait(&DE, &DE_outcoming_mutex);
	     	pthread_cond_wait(&DF, &DF_outcoming_mutex);
	           
   
     		pq.push(SectionACoutcomingqueue);
     		pq.push(SectionBCoutcomingqueue);
     		pq.push(SectionDEoutcomingqueue);
     		pq.push(SectionDFoutcomingqueue);
     		
		max_queue=pq.top();
		
		//total trains are calculated by summing all queue sizes since they have all trains exist.
		int total_trains=SectionACoutcomingqueue.size()+SectionBCoutcomingqueue.size()+SectionDEoutcomingqueue.size()+SectionDFoutcomingqueue.size()-1;	
		pthread_mutex_unlock(&AC_outcoming_mutex);
 	     	pthread_mutex_unlock(&BC_outcoming_mutex);
		pthread_mutex_unlock(&DE_outcoming_mutex);
		pthread_mutex_unlock(&DF_outcoming_mutex);  
			
		gettimeofday(&current_time,NULL);
		
		if(!clear&&total_trains>10){
			clear=true;events_log.push(Event("System Overload", current_time, 100000, trains_waiting(10000)));
			gettimeofday(&system_overload_start_time,NULL);
		}
		
		if(clear&&(total_trains+1)==0){
			clear=false;
			events_log.push(Event("Tunnel Cleared", current_time, 100000, "Time taken to clear the tunnel"+to_string(current_time.tv_sec-system_overload_start_time.tv_sec)));
		}
			  
		if(max_queue.size()!=0){ //if there is a train in waiting
		
			Train tunnel_train=max_queue.front();
			gettimeofday(&current_time,NULL);
			events_log.push(Event("Tunnel Passing", current_time, tunnel_train.train_id, trains_waiting(tunnel_train.train_id)));
			int additional_breakdown=0;
			double breakdown_probability= ((double) rand()/(double) RAND_MAX); 
			
			if(breakdown_probability<= 0.10){
				additional_breakdown=4;
				gettimeofday(&current_time,NULL);
				struct timeval breakdown_time=current_time;
				breakdown_time.tv_sec+=rand()%((tunnel_train.length/100)+1);
				events_log.push(Event("Breakdown     ", breakdown_time, tunnel_train.train_id, trains_waiting(tunnel_train.train_id)));
			}
		        
	        switch(tunnel_train.starting_point){
	            case 'A':SectionACoutcomingqueue.pop();break;
	            case 'B':SectionBCoutcomingqueue.pop();break;
	            case 'E':SectionDEoutcomingqueue.pop();break;
	            case 'F':SectionDFoutcomingqueue.pop();break;
            }
		        printf("heyy");
		        
		        
	        pthread_sleep((tunnel_train.length/100)+1+additional_breakdown);
	        gettimeofday(&current_time,NULL);
	        tunnel_train.departure_time.tv_sec=current_time.tv_sec+1; 
	        train_logs.push(tunnel_train);
		}
		
			    
		gettimeofday(&current_time,NULL);
    }
    exit(1);
}



// for all secitons: to generate a train probability clearence of tunnel is checked
// then train is generated.

void *Section(void *threadarg){
	struct thread_data *section_data;
	section_data=(struct thread_data *) threadarg;
	struct timeval current_time;
	gettimeofday(&current_time,NULL);

	while(current_time.tv_sec<start_time.tv_sec+ simulation_time){ 
		Train newtrain=Train(start_time, start_time, 2,2,'b','b');  //generic
		Train incomingtrain(start_time, start_time, 1,1,'a','a');  //generic
		bool cleartemp=true;
		
		double rand_train= ((double) rand()/(double) RAND_MAX); // generate a random number to decide if a train arrives
		if(rand_train<= probability&&!clear){ // generate a train
			pthread_mutex_lock(&train_id_mutex);	//we need a unique train_id for each train
			train_id+=1;
			
			int train_length;
			char destination;
			double rand_length= ((double) rand()/(double) RAND_MAX);  // generate a random number to decide train length
			if(rand_length<= 0.7) {
				train_length=100;
			} 
			else{
				train_length=200;
			}
			double rand_line= ((double) rand()/(double) RAND_MAX); // generate a r.n. to decide which section to join
			if(rand_line<= 0.5){
				destination=section_data->dest1;
			} 
			else {
				destination=section_data->dest2;
			}
			newtrain=Train(current_time, current_time, train_id,  train_length, section_data->start,  destination);
			cleartemp=clear;
			pthread_mutex_unlock(&train_id_mutex);
		}
		
		gettimeofday(&current_time,NULL);
		pthread_sleep (1);
		
		pthread_mutex_lock(&(*section_data->section_mutex));
			
		if(rand_train<= probability&&!cleartemp){
		    	(*section_data->sectionqueue).push(newtrain);
					
		} 
		pthread_cond_signal(&(*section_data->section_cond) );
		
		pthread_mutex_unlock(&(*section_data->section_mutex));
		gettimeofday(&current_time,NULL);
	} 
	exit(1);
}

// log thread keeps infos about train parameters as train.log and event details as control-center.log,
// write these values to a file.
void *Log(void *v){
	struct timeval current_time;
	gettimeofday(&current_time,NULL);
	bool writtenonce;
	
	while(current_time.tv_sec<=start_time.tv_sec+ simulation_time){

		if(!writtenonce&&current_time.tv_sec==start_time.tv_sec+ simulation_time){
			writtenonce=true;
		
			FILE *fp2;
			fp2=fopen("train.log", "a");
			while (!train_logs.empty()) {
				Train p2(train_logs.top());
				struct tm *nowtm; //for arrival time conversion
				char tmbuf[64];
				nowtm = localtime(&p2.arrival_time.tv_sec);
				strftime(tmbuf, sizeof tmbuf, "%H:%M:%S", nowtm);
				struct tm *nowtm2; //for departure time conversion
				char tmbuf2[64];
				nowtm2 = localtime(&p2.departure_time.tv_sec);
				strftime(tmbuf2, sizeof tmbuf2, "%H:%M:%S", nowtm2);
				fprintf(fp2,"%d                    %c                     %c                 %d            %s            %s \n", p2.train_id, p2.starting_point, p2.destination_point, p2.length , tmbuf, tmbuf2);
				train_logs.pop();
			}
			fclose(fp2);

			FILE *fp4;
			fp4=fopen("control_center.log", "a");
			while (!events_log.empty()) {
				Event e1(events_log.front());
				struct tm *nowtm3; //for event time conversion
				char tmbuf3[64];
				nowtm3 = localtime(&e1.event_time.tv_sec);
				strftime(tmbuf3, sizeof tmbuf3, "%H:%M:%S", nowtm3);
				fprintf(fp4,"%s                      %s                     %d               %s\n", e1.event_type.c_str(), tmbuf3, e1.train_id, e1.trains_waiting.c_str());
				events_log.pop();
			}
				fclose(fp4);
	
		}
		gettimeofday(&current_time,NULL);
	}
	exit(2);
}

int main(int argc, char *argv[])
{
	int chooseoption = 0;

	//Command line inputs for simulation time, seed and probability
	while ((chooseoption = getopt(argc, argv, "-d:-p:-s:")) != -1) {
		switch (chooseoption) {
			 case 'd':
			 seed=atoi(optarg);
			 break;
			 case 'p':
			 probability = atof(optarg);
			 break;
			 case 's':
			 simulation_time = atoi(optarg);
			 break;
			 default:
			 exit(1);
		}
	}
	//data for different sections
	thread_data_array[0].start='A';
	thread_data_array[0].dest1='E';
	thread_data_array[0].dest2='F';
	thread_data_array[0].section_mutex=&AC_outcoming_mutex;
	thread_data_array[0].section_cond=&AC;
	thread_data_array[0].sectionqueue=&SectionACoutcomingqueue;

	thread_data_array[1].start='B';
	thread_data_array[1].dest1='E';
	thread_data_array[1].dest2='F';
	thread_data_array[1].section_mutex=&BC_outcoming_mutex;
	thread_data_array[1].section_cond=&BC;
	thread_data_array[1].sectionqueue=&SectionBCoutcomingqueue;

	thread_data_array[2].start='E';
	thread_data_array[2].dest1='A';
	thread_data_array[2].dest2='B';
	thread_data_array[2].section_mutex=&DE_outcoming_mutex;
	thread_data_array[2].section_cond=&DE;
	thread_data_array[2].sectionqueue=&SectionDEoutcomingqueue;

	thread_data_array[3].start='F';
	thread_data_array[3].dest1='A';
	thread_data_array[3].dest2='B';
	thread_data_array[3].section_mutex=&DF_outcoming_mutex;
	thread_data_array[3].section_cond=&DF;
	thread_data_array[3].sectionqueue=&SectionDFoutcomingqueue;
 
	printf("Manual:\nproject2 [options]\n options:\n-s              The total simulation time in seconds\n-p              Probability\n-d              Seed for debugging\n");
    
	FILE *fp;                   //Train.logs file initialized
	fp=fopen("train.log","w");
	fprintf(fp,"Simulation Arguments: p= %f, s= %d, d=%d  \n", probability, simulation_time, seed);
	fprintf(fp,"TrainID          Starting Point    DestinationPoint       Length(m)      Arrival Tİme       Departure Time\n");
	fclose(fp);
	
	FILE *fp3;                  //Control_center.log file initialized
	fp3=fopen("control_center.log","w");
	fprintf(fp3,"Event         			   Event Time                 Train_id           Trains Waiting Passage \n");
	fclose(fp3);

	gettimeofday(&start_time,NULL);

	//Create section threads
	pthread_t sections[4];
	pthread_create(&sections[0], NULL, Section, (void *) &thread_data_array[0]); //AC
	pthread_create(&sections[1], NULL, Section, (void *) &thread_data_array[1]); //BC
	pthread_create(&sections[2], NULL, Section, (void *) &thread_data_array[2]); //DE
	pthread_create(&sections[3], NULL, Section, (void *) &thread_data_array[3]); //DF

	//Create controller thread
	pthread_t controllerthread;
	pthread_create(&controllerthread, NULL, Controller, NULL);

	//Create log thread
	pthread_t logthread;
	pthread_create(&logthread, NULL, Log, NULL);

	/*Wait for simulation to end*/

	//Join threads
	for(int t=0;t<4;t=t++){
		pthread_join(sections[t],NULL);
	}
	pthread_join(controllerthread,NULL);
	pthread_join(logthread,NULL);

	//Destroy attributes, mutexes, conditional variables, threads
	pthread_mutex_destroy(&train_id_mutex);
	pthread_mutex_destroy(&AC_outcoming_mutex);
	pthread_mutex_destroy(&BC_outcoming_mutex);
	pthread_mutex_destroy(&DE_outcoming_mutex);
	pthread_mutex_destroy(&DF_outcoming_mutex);

	pthread_cond_destroy(&AC);
	pthread_cond_destroy(&BC);
	pthread_cond_destroy(&DE);
	pthread_cond_destroy(&DF);

	pthread_exit(NULL);
	exit(1);
}


