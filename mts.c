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

FILE* output_file = NULL;


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


//Main function, initializes mutex and convar, creates and collects threads into thread_ids, cleans program before completing.
int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
		return 1;
	}

	output_file = fopen("output.txt", "w");
	if (output_file == NULL) {
		fprintf(stderr, "Error: Can't create output.txt\n");
		return 1;
	}

	if (read_input_file(argv[1]) != 0) {
		fprintf(stderr, "Error reading input file\n");
		fclose(output_file);
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
			fprintf(stderr, "Error creating thread for train %d\n", i);
			free(trains);
			free(thread_ids);
			return 1;
		}
	}

	// Wait for train threads to complete
	
	for (int i=0; i < num_trains; i++) {
		pthread_join(thread_ids[i], NULL);
	}

	fclose(output_file);

	// Clean
	

	pthread_mutex_destroy(&global_mutex);
	pthread_cond_destroy(&train_ready_convar);
	free(trains);
	free(thread_ids);


	return 0;

}



// Reads and stores train data from input file
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



// Gets time at specific points during execution
double get_current_time() {
	struct timeval current;
	gettimeofday(&current, NULL);

	
	double elapsed = (current.tv_sec - start_time.tv_sec) + ((current.tv_usec - start_time.tv_usec) / 1000000.0);
	return elapsed;



}
	

// Formats time stamps, prints to stdout and writes into output.txt file.
void print_time_stamp(const char* format, int train_id, const char* direction) {

	double current = get_current_time();
	int hours = (int)(current/3600);
	int minutes = (int)((current - hours * 3600) / 60);
	double seconds = current - hours * 3600 - minutes * 60;

	printf("%02d:%02d:%04.1f ", hours, minutes, seconds);
	printf(format, train_id, direction);
	printf("\n");

	if (output_file != NULL) {
		fprintf(output_file, "%02d:%02d:%04.1f ", hours, minutes, seconds);
		fprintf(output_file, format, train_id, direction);
		fprintf(output_file, "\n");
		fflush(output_file);
	}
	
}
// Goes through actions train will go through in simulation, checks for scheduling, simulates loading and crossing time with usleep(), uses mutex to ensure 
// safe use of shared variables (track occupied). 
void* train_thread(void* arg) {
	Train* my_train = (Train*)arg;
	

	usleep(my_train->load_time * 100000);
	pthread_mutex_lock(&global_mutex);

	my_train->ready_time = get_current_time();
	my_train->is_ready = 1;
	
	// If direction is E
	const char* dir_str = (my_train->direction == 'E') ? "East" : "West";
	print_time_stamp("Train %2d is ready to go %4s", my_train->id, dir_str);

	pthread_cond_broadcast(&train_ready_convar);
	
	pthread_mutex_unlock(&global_mutex);
	usleep(1000);
	pthread_mutex_lock(&global_mutex);

    	// Waits for permission to cross
    	while (!is_my_turn(my_train)) {
        	pthread_cond_wait(&train_ready_convar, &global_mutex);
    	}

    	// Got permission, marks track as occupied
    	track_occupied = 1;

    	// Updates consecutive count to track starvation prevention
    	if (last_direction == my_train->direction) {
        	consecutive_count++;
    	} else {
        	consecutive_count = 1;
    	}
    		last_direction = my_train->direction;

    	// Mark as no longer waiting
    	my_train->is_ready = 0;

    	print_time_stamp("Train %2d is ON the main track going %4s", my_train->id, dir_str);

    	pthread_mutex_unlock(&global_mutex);
	
    	// Simulate crossing time
    	usleep(my_train->cross_time * 100000);

    	// Finish crossing, update shared state
    	pthread_mutex_lock(&global_mutex);

    	track_occupied = 0;
    	print_time_stamp("Train %2d is OFF the main track after going %4s", my_train->id, dir_str);

    	// Wake up waiting trains
    	pthread_cond_broadcast(&train_ready_convar);

    	pthread_mutex_unlock(&global_mutex);

    	return NULL;
}

	      			      
// Helper function to check if train is next to go
int is_my_turn(Train* me) {
    // Track must be free
    if (track_occupied) {
        return 0;
    }

    // Find highest priority train that should go next
    Train* next_train = get_highest_priority_train();

    // Check if (me) is correct train
    return (next_train != NULL && next_train->id == me->id);
}


// Returns train that has highest priority to cross
Train* get_highest_priority_train() {
    Train* best = NULL;
    int highest_priority = -1;
    
    //Find highest priority level among ready trains
    for (int i = 0; i < num_trains; i++) {
        if (trains[i].is_ready && trains[i].is_high_priority > highest_priority) {
            highest_priority = trains[i].is_high_priority;
        }
    }
    
    if (highest_priority == -1) {
        return NULL;
    }
    
    //Counts trains in each direction at this priority level
    int east_count = 0, west_count = 0;
    
    for (int i = 0; i < num_trains; i++) {
        if (trains[i].is_ready && trains[i].is_high_priority == highest_priority) {
            if (trains[i].direction == 'E') {
                east_count++;
            } else {
                west_count++;
            }
        }
    }
    
    //Apply scheduling rules
    
    //Only trains in one direction - pick earliest ready_time, then lowest ID
    if (east_count > 0 && west_count == 0) {
        best = NULL;
        double earliest = 999999.0;
        for (int i = 0; i < num_trains; i++) {
            if (trains[i].is_ready && 
                trains[i].is_high_priority == highest_priority &&
                trains[i].direction == 'E') {
                if (trains[i].ready_time < earliest || 
                    (trains[i].ready_time == earliest && (best == NULL || trains[i].id < best->id))) {
                    earliest = trains[i].ready_time;
                    best = &trains[i];
                }
            }
        }
        return best;
    }
    
    if (west_count > 0 && east_count == 0) {
        best = NULL;
        double earliest = 999999.0;
        for (int i = 0; i < num_trains; i++) {
            if (trains[i].is_ready && 
                trains[i].is_high_priority == highest_priority &&
                trains[i].direction == 'W') {
                if (trains[i].ready_time < earliest || 
                    (trains[i].ready_time == earliest && (best == NULL || trains[i].id < best->id))) {
                    earliest = trains[i].ready_time;
                    best = &trains[i];
                }
            }
        }
        return best;
    }
    
    // Both directions have trains going - determine target direction
    if (east_count > 0 && west_count > 0) {
        char target_direction;
        
        // Check starvation prevention
        if (consecutive_count >= 2) {
            target_direction = (last_direction == 'E') ? 'W' : 'E';
        } else if (last_direction == '\0') {
            // If no trains have crossed yet - Westbound trains have priority
            target_direction = 'W';
        } else if (last_direction == 'E') {
            target_direction = 'W';
        } else {
            target_direction = 'E';
        }
        
        // Find best train in target direction (earliest ready_time, then lowest ID)
        best = NULL;
        double earliest = 999999.0;
        for (int i = 0; i < num_trains; i++) {
            if (trains[i].is_ready && 
                trains[i].is_high_priority == highest_priority &&
                trains[i].direction == target_direction) {
                if (trains[i].ready_time < earliest || 
                    (trains[i].ready_time == earliest && (best == NULL || trains[i].id < best->id))) {
                    earliest = trains[i].ready_time;
                    best = &trains[i];
                }
            }
        }
        return best;
    }
    
    return NULL;
}
