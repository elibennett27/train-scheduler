#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

typedef struct {
	int id;
	char direction;
	int is_high_priority;
	int load_time;
	int cross_time;
	double ready_time;
	int is_ready;

} Train;



pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t train_ready_convar = PTHREAD_COND_INITIALIZER;



// Flags
int track_occupied = 0;
char last_direction = '\0';
int consecutive_count = 0;
struct timeval start_time;

Train* trains = NULL;
int num_trains;



// Function prototypes

int read_input_file(const char* filename);
void* train_thread(void* arg);
double get_current_time();
void print_time_stamp(const char* format, int train_id, const char* direction);
int is_my_turn(Train* me);
Train* get_highest_priority_train();

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
		return 1;
	}
	if (read_input_file(argv[1]) != 0) {
		fprintf(stderr, "Error reading input file\n");
		return 1;
	
	}


	// Initialize mutex and convar
	pthread_mutex_init(&global_mutex, NULL);
	pthread_cond_init(&train_ready_convar, NULL);
	
	// Simulation start time
	gettimeofday(&start_time, NULL);
	
	// Array to hold thread IDs
	pthread_t* thread_ids = malloc(num_trains * sizeof(pthread_t));
	if (thread_ids == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		free(trains);
		return 1;
	}

	// Create threads for each train
	
	for (int i=0; i<num_trains; i++) {
		if (pthread_create(&thread_ids[i], NULL, train_thread, &trains[i]) != 0) {
			fprintf(stderr, "Error creating thread for train", i);
			free(trains);
			free(thread_ids);
			return 1;
		}
	}

	// Wait for train threads to complete
	
	for (int i=0; i < num_trains; i++) {
		pthread_join(thread_ids[i], NULL);
	}

	// Clean
	

	pthread_mutex_destroy(&global_mutex);
	pthread_cond_destroy(&train_ready_convar);
	free(trains);
	free(thread_ids);


	return 0;

}




int read_input_file(const char* filename) {
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
		return -1;
	}

	char direction;
	int load_time;
	int cross_time;
	num_trains = 0;
	while (fscanf(file, " %c %d %d", &direction, &load_time, &cross_time) == 3) {
		num_trains++;
	}
	
	// Get num_trains on first scan and then allocate memory for the array of trains
	trains = malloc(num_trains * sizeof(Train));
	if (trains == NULL) {
		fclose(file);
		return -1;
	}

	rewind(file); // Rewind will go back to the start of the file
	for (int i=0; i<num_trains; i++) {
		// Collect direction, load time, cross time and check if any misinput if so halt the program
		if (fscanf(file, " %c %d %d", &direction, &load_time, &cross_time) != 3) {
			free(trains);
			fclose(file);
			return -1;
		}
		
		trains[i].id = i;
		trains[i].load_time = load_time;
		trains[i].cross_time = cross_time;
		trains[i].is_ready = 0;
		trains[i].ready_time = 0.0;
		
		// Check for high-priority trains, also check for misinput in direction character
		if (direction == 'e' || direction == 'E') {
			trains[i].direction = 'E';
            		trains[i].is_high_priority = (direction == 'E') ? 1 : 0;
        	} else if (direction == 'w' || direction == 'W') {
			trains[i].direction = 'W';
            		trains[i].is_high_priority = (direction == 'W') ? 1 : 0;
		} else {
			fprintf(stderr, "Invalid direction: %c\n", direction);
			free(trains);
			fclose(file);
			return -1;
		}
	
	}

	fclose(file);
	return 0;

}




double get_current_time() {
	struct timeval current;
	gettimeofday(&current, NULL);

	
	double elapsed = (current.tv_sec - start_time.tv_sec) + ((current.tv_usec - start_time.tv_usec) / 1000000.0);
	return elapsed;



}
	


void print_time_stamp(const char* format, int train_id, const char* direction) {

	double current = get_current_time();
	int hours = (int)(current/3600);
	int minutes = (int)((current - hours * 3600) / 60);
	double seconds = current - hours * 3600 - minutes * 60;

	printf("%02d:%02d:%04.1f ", hours, minutes, seconds);
	printf(format, train_id, direction);
	printf("\n");
	
}

void* train_thread(void* arg) {
	Train* my_train = (Train*)arg;
	

	usleep(my_train->load_time * 100000);
	pthread_mutex_lock(&global_mutex);

	my_train->ready_time = get_current_time();
	my_train->is_ready = 1;
	
	// If direction is E
	const char* dir_str = (my_train->direction == 'E') ? "East" : "West";
	print_time_stamp("Train %2d is ready to go %4s", my_train->id, dir_str);
	





	// wake other trains, wait for permission, clean




	pthread_mutex_unlock(&global_mutex);
	return NULL;
}

	      			      
	

