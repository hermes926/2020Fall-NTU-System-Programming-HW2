#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	int player_id;
        if(argc > 1)
		player_id = atoi(argv[1]);
	else
		return 0;
        int bid_list[21] = {
	    20, 18, 5, 21, 8, 7, 2, 19, 14, 13,
        	9, 1, 6, 10, 16, 11, 4, 12, 15, 17, 
	};
	int round = 1; 
	while(round <= 10){
	        int bid = bid_list[player_id + round - 2] * 100;
	        printf("%d %d\n", player_id, bid);
	        fflush(stdout);
		fsync(STDOUT_FILENO);
		round ++;
	}
	exit(0);
	return 0;
}
