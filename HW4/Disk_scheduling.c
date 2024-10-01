#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define CYLINDERS 5000
#define REQUESTS 1000

// The function that sorts the requests to ascending
int cmp(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void generate_random_requests(int* requires)
{
    srand(time(0));
    for(int i = 0; i < REQUESTS; i++){
        requires[i] = rand() % CYLINDERS;
    }
}

void FCFS(int* requests, int head, int *total_movement, double *average_latency)
{
    *total_movement = 0;
    *average_latency = 0;

    for(int i = 0; i < REQUESTS; i++){
        *total_movement += abs(requests[i] - head);
        head = requests[i];
    }

    *average_latency = (*total_movement / 100.0) * 1.0;
}

void SSTF(int* requests, int head, int *total_movement, double *average_latency)
{
    int visited[REQUESTS] = {0};    // Record the requests which have been visited
    *total_movement = 0;
    *average_latency = 0;

    for(int i = 0; i < REQUESTS; i++){
        int min_distance = CYLINDERS;
        int index = -1;

        for(int j = 0; j < REQUESTS; j++){
            if(!visited[j] && abs(requests[j] - head) < min_distance){
                min_distance = abs(requests[j] - head);
                index = j;    // Record the next selected request
            }
        }

        if (index != -1) {
            visited[index] = 1;    // Declare that the request has been serviced
            *total_movement += abs(requests[index] - head);
            head = requests[index]; // Update head position
        }
    }

    *average_latency = (*total_movement / 100.0) * 1.0; // Calculate average latency
}

void SCAN(int* requests, int head, int *total_movement, double *average_latency)
{
    *total_movement = 0;
    *average_latency = 0;

    // Split the requests into left array and right array
    int left[REQUESTS + 1], right[REQUESTS + 1];
    int left_size = 0, right_size = 0;

    // Determine the initial direction
    char direction[6];
    if (head < requests[0]) {
        strcpy(direction, "right");
    }
    else {
        strcpy(direction, "left");
    }

    // Appending end values which has to be visited before reversing the direction
    if (strcmp(direction, "left") == 0) {
        left[left_size++] = 0;
    }
    else if (strcmp(direction, "right") == 0) {
        right[right_size++] = CYLINDERS - 1;
    }

    for (int i = 0; i < REQUESTS; i++) {
        if (requests[i] < head) {
            left[left_size++] = requests[i];
        }

        if (requests[i] > head) {
            right[right_size++] = requests[i];
        }
    }

    // Sort the left array and the right array
    qsort(left, left_size, sizeof(int), cmp);
    qsort(right, right_size, sizeof(int), cmp);

    int cur_track;	// record the request which is serviced currently

    // Run the while loop two times, one by one scanning right and left of the head
    int run = 2;
    while (run--) {
        if (strcmp(direction, "left") == 0) {
            for (int i = left_size - 1; i >= 0; i--) {
                cur_track = left[i];

                // Increase the total count
                *total_movement += abs(cur_track - head);

                // Accessed track is now the new head
                head = cur_track;
            }
            // set the direction to the right
            strcpy(direction, "right");
        }

        else if (strcmp(direction, "right") == 0) {
            for (int i = 0; i < right_size; i++) {
                cur_track = right[i];

                // Increase the total count
                *total_movement += abs(cur_track - head);

                // Accessed track is now new head
                head = cur_track;
            }
            // set the direction to the left
            strcpy(direction, "left");
        }
    }

    // Calculate average latency
    *average_latency = (*total_movement / 100.0) * 1.0;
}

void CSCAN(int* requests, int head, int *total_movement, double *average_latency) 
{
    *total_movement = 0;
    *average_latency = 0;

    int left[REQUESTS + 1], right[REQUESTS + 1];
    int left_size = 0, right_size = 0;

    // Appending end values which has to be visited before reversing the direction
    left[left_size++] = 0;
    right[right_size++] = CYLINDERS - 1;

    // Tracks on the left of the head will be serviced once the head comes back to the beginning.
    for (int i = 0; i < REQUESTS; i++) {
        if (requests[i] < head) {
            left[left_size++] = requests[i];
        }
        if (requests[i] > head) {
            right[right_size++] = requests[i];
        }
    }

    // Sorting left and right arrays
    qsort(left, left_size, sizeof(int), cmp);
    qsort(right, right_size, sizeof(int), cmp);

    int cur_track;	// record the request which is serviced currently

    // First service the requests on the right side of the head
    for (int i = 0; i < right_size; i++) {
        cur_track = right[i];

        // Increase the total count
        *total_movement += abs(cur_track - head);

        // Update the head
        head = cur_track;
    }

    // Once reached the right end, jump to the beginning
    head = 0;

    // Jumps to the head and adding the distance from right end to left end (+CYLINDERS - 1)
    *total_movement += (CYLINDERS - 1);

    // Now service the requests which are in left array
    for (int i = 0; i < left_size; i++) {
        cur_track = left[i];

        // Increase the total count
        *total_movement += abs(cur_track - head);

        // Update the head
        head = cur_track;
    }

    // Calculate average latency
    *average_latency = (*total_movement / 100.0) * 1.0;
}

void LOOK(int* requests, int head, int* total_movement, double* average_latency) 
{
    *total_movement = 0;
    *average_latency = 0;

    int left[REQUESTS], right[REQUESTS];
    int left_size = 0, right_size = 0;

    // Determine the initial direction
    char direction[6];
    if (head < requests[0]) {
        strcpy(direction, "right");
    }
    else {
        strcpy(direction, "left");
    }

    // Appending values which are currently at left and right direction from the head
    for (int i = 0; i < REQUESTS; i++) {
        if (requests[i] < head) {
            left[left_size++] = requests[i];
        }
        if (requests[i] > head) {
            right[right_size++] = requests[i];
        }
    }

    // Sorting left and right arrays
    qsort(left, left_size, sizeof(int), cmp);
    qsort(right, right_size, sizeof(int), cmp);

    int cur_track;	// record the request which is serviced currently

    // Run the while loop two times, one by one scanning right and left side of the head
    int run = 2;
    while (run--) {
        if (strcmp(direction, "left") == 0) {
            for (int i = left_size - 1; i >= 0; i--) {
                cur_track = left[i];

                // Increase the total count
                *total_movement += abs(cur_track - head);

                // Update the head
                head = cur_track;
            }
            // Reversing the direction
            strcpy(direction, "right");
        }

        else if (strcmp(direction, "right") == 0) {
            for (int i = 0; i < right_size; i++) {
                cur_track = right[i];

                // Increase the total count
                *total_movement += abs(cur_track - head);

                // Update the head
                head = cur_track;
            }
            // Reversing the direction
            strcpy(direction, "left");
        }
    }

    // Calculate average latency
    *average_latency = (*total_movement / 100.0) * 1.0;
}

void CLOOK(int* requests, int head, int* total_movement, double* average_latency) 
{
    *total_movement = 0;
    *average_latency = 0;

    int left[REQUESTS], right[REQUESTS];
    int left_size = 0, right_size = 0;

    // Split the requests into left array and right array
    for (int i = 0; i < REQUESTS; i++) {
        if (requests[i] < head) {
            left[left_size++] = requests[i];
        }
        else if (requests[i] > head) {
            right[right_size++] = requests[i];
        }
    }

    // Sort the left array and the right array
    qsort(left, left_size, sizeof(int), cmp);
    qsort(right, right_size, sizeof(int), cmp);

    int cur_track;	// record the request which is serviced currently

    // First deal with the right array
    for (int i = 0; i < right_size; i++) {
        cur_track = right[i];

        *total_movement += abs(cur_track - head);

        // Update the head
        head = cur_track;
    }

    // Once reach the end of the right array, jump to the first one of the left array
    if (left_size > 0) {
        *total_movement += abs(head - left[0]);
        head = left[0];
    }

    // Deal with the left requests
    for (int i = 0; i < left_size; i++) {
        cur_track = left[i];

        *total_movement += abs(cur_track - head);

        // Update the head
        head = cur_track;
    }

    *average_latency = (*total_movement / 100.0) * 1.0;
}

void OPTIMAL(int* requests, int head, int* total_movement, double* average_latency)
{
	*total_movement = 0;
    *average_latency = 0;

	int sorted_requests[REQUESTS];
	for(int i = 0; i < REQUESTS; i++) {
		sorted_requests[i] = requests[i];
	}

	// Sort the requests, and put the result in the sorted_requests
	qsort(sorted_requests, REQUESTS, sizeof(int), cmp);

	for(int i = 0; i < REQUESTS; i++) {
		*total_movement += abs(sorted_requests[i] - head);
		head = sorted_requests[i];
	}

	*average_latency = (*total_movement / 100.0) * 1.0;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
         printf("Usage: %s <initial_head_position>\n", argv[0]);
         return 1;
    }

    int head =  atoi(argv[1]);          // converts a command-line argument (a string) into an integer.
    if (head < 0 || head > CYLINDERS) {
        printf("Invalid initial head position. Must be between 0 and %d.\n", CYLINDERS - 1);
        return 1;
    }

    int requests[REQUESTS];             // generate the random requests
    generate_random_requests(requests);

	int total_movement = 0;
	double average_latency = 0;

    FCFS(requests, head, &total_movement, &average_latency);
    printf("FCFS: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

	SSTF(requests, head, &total_movement, &average_latency);
	printf("SSTF: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

	SCAN(requests, head, &total_movement, &average_latency);
    printf("SCAN: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

	CSCAN(requests, head, &total_movement, &average_latency);
    printf("CSCAN: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

    LOOK(requests, head, &total_movement, &average_latency);
    printf("LOOK: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

	CLOOK(requests, head, &total_movement, &average_latency);
    printf("CLOOK: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

    OPTIMAL(requests, head, &total_movement, &average_latency);
    printf("OPTIMAL: Total Head Movement = %d, Average Latency = %.2f ms\n", total_movement, average_latency);

    return 0;
}
