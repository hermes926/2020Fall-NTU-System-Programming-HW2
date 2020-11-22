#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

void err_sys(const char *err_m){
	perror(err_m);
	exit(1);
}

void flush_sync(FILE *pipe, int fd){
	fflush(pipe);
	//fsync(fd);
	return;
}


void create_child(pid_t *pid1, pid_t *pid_2, int pipe1_d[2], int pipe1_u[2], int pipe2_d[2], int pipe2_u[2], char *a1, char *a2, int depth, int player1_id, int player2_id){
	if(pipe(pipe1_d) < 0)
		err_sys("pipe1_down_error\n");
	if(pipe(pipe1_u) < 0)
		err_sys("pipe1_up_error\n");
	if((*pid1 = fork()) < 0)
		err_sys("fork1_error\n");
	else if(*pid1 == 0){
		close(pipe1_d[1]);
		close(pipe1_u[0]);
		if(dup2(pipe1_d[0], STDIN_FILENO) != STDIN_FILENO)
			err_sys("p1d_dup2_error\n");
		if(dup2(pipe1_u[1], STDOUT_FILENO) != STDOUT_FILENO)
			err_sys("p1u_dup2_error\n");
		close(pipe1_d[0]);
		close(pipe1_u[1]);


		if(depth < 2){
			char next_depth[3];
			sprintf(next_depth, "%d\0", depth + 1);
			execl("./host", "./host", a1, a2, next_depth, (char *)0);
		}else{
			char player_id[100];
			sprintf(player_id, "%d\0", player1_id);
			execl("./player", "./player", player_id, (char *)0);
		}

	}
	close(pipe1_d[0]);
	close(pipe1_u[1]);

	if(pipe(pipe2_d) < 0)
		err_sys("pipe2_down_error\n");
	if(pipe(pipe2_u) < 0)
		err_sys("pipe2_up_error\n");
	if((*pid_2 = fork()) < 0)
		err_sys("fork2_error\n");
	else if(*pid_2 == 0){
		close(pipe2_d[1]);
		close(pipe2_u[0]);
		if(dup2(pipe2_d[0], STDIN_FILENO) != STDIN_FILENO)
			err_sys("p2d_dup2_error\n");
		if(dup2(pipe2_u[1], STDOUT_FILENO) != STDOUT_FILENO)
			err_sys("p2u_dup2_error\n");
		close(pipe2_d[0]);
		close(pipe2_u[1]);


		if(depth < 2){
			char next_depth[3];
			sprintf(next_depth, "%d\0", depth + 1);
			execl("./host", "./host", a1, a2, next_depth, (char *)0);
		}else{
			char player_id[100];
			sprintf(player_id, "%d\0", player2_id);
			execl("./player", "./player", player_id, (char *)0);
		}

	}
	close(pipe2_d[0]);
	close(pipe2_u[1]);
	return ;
}




int main(int argc, char* argv[]){
	int host_id = -1, key = -1, depth = -1;
	if(argc >= 4){
		host_id = atoi(argv[1]);
		key = atoi(argv[2]);
		depth = atoi(argv[3]);
	}
	pid_t cpid_1, cpid_2;
	int pipe1_u[2], pipe1_d[2], pipe2_u[2], pipe2_d[2];

	if(depth < 2)
		create_child(&cpid_1, &cpid_2, pipe1_d, pipe1_u, pipe2_d, pipe2_u, argv[1], argv[2], depth, 0, 0);
	if(depth == 0){
		char FIFO_w_name[20] = {"fifo_0.tmp\0"};
		char FIFO_r_name[20];
		sprintf(FIFO_r_name, "fifo_%d.tmp\0", host_id);
		FILE *FIFO_read = fopen(FIFO_r_name, "r");
		FILE *FIFO_write = fopen(FIFO_w_name, "w");
		if(FIFO_read == NULL || FIFO_write == NULL)
			err_sys("FIFO error!!");

		FILE *pipe1_w = fdopen(pipe1_d[1], "w");
		FILE *pipe1_r = fdopen(pipe1_u[0], "r");
		FILE *pipe2_w = fdopen(pipe2_d[1], "w");
		FILE *pipe2_r = fdopen(pipe2_u[0], "r");

		if(pipe1_w == 0 || pipe1_r == 0 || pipe2_w == 0 || pipe2_w == 0)
			err_sys("depth = 0, pipe error!!!");
		int player[8];
		fscanf(FIFO_read, "%d %d %d %d %d %d %d %d", &player[0], &player[1], &player[2], &player[3], &player[4], &player[5], &player[6], &player[7]);		     
		while(player[0] != -1){
			int rank[8][3];
			for(int i = 0; i < 8; i ++){
				rank[i][0] = player[i];
				rank[i][1] = 0;
				rank[i][2] = 0;
			}
			fprintf(pipe1_w, "%d %d %d %d\n", player[0], player[1], player[2], player[3]);
			fprintf(pipe2_w, "%d %d %d %d\n", player[4], player[5], player[6], player[7]);
			flush_sync(pipe1_w, pipe1_d[1]);
			flush_sync(pipe2_w, pipe2_d[1]);

			int p1_id, p2_id, p1_bid, p2_bid;
			for(int i = 0; i < 10; i ++){
				fscanf(pipe1_r, "%d %d", &p1_id, &p1_bid);
				fscanf(pipe2_r, "%d %d", &p2_id, &p2_bid);
				int w_id = (p1_bid > p2_bid) ? p1_id : p2_id;
				for(int j = 0; j < 8; j++){
					if(w_id == rank[j][0]){
						rank[j][1] ++;
						break;
					}
				}
			}
			int ranks = 1, people = 0;
			for(int point = 10; point >= 0; point--){
				people = 0;
				for(int j = 0; j < 8; j++){
					if(rank[j][1] == point){
						rank[j][2] = ranks;
						people++;
					}
				}
				ranks += people;
				if(ranks >= 9)
					break;
			}
			fprintf(FIFO_write, "%d\n", key);
			for(int i = 0; i < 8; i++)
				fprintf(FIFO_write, "%d %d\n", rank[i][0], rank[i][2]);
			int FIFO_write_fd = fileno(FIFO_write); 
			flush_sync(FIFO_write, FIFO_write_fd);
			fscanf(FIFO_read, "%d %d %d %d %d %d %d %d", &player[0], &player[1], &player[2], &player[3], &player[4], &player[5], &player[6], &player[7]);		     
		}
		fprintf(pipe1_w, "-1 -1 -1 -1\n");
		fprintf(pipe2_w, "-1 -1 -1 -1\n");
		flush_sync(pipe1_w, pipe1_d[1]);
		flush_sync(pipe2_w, pipe2_d[1]);
		wait(NULL);
		wait(NULL);
		fclose(pipe1_w);
		fclose(pipe2_w);
		fclose(pipe1_r);
		fclose(pipe2_r);
		exit(0);
		//wait(NULL);
		//wait(NULL);
	}else if(depth == 1){
		FILE *pipe1_w = fdopen(pipe1_d[1], "w");
		FILE *pipe1_r = fdopen(pipe1_u[0], "r");
		FILE *pipe2_w = fdopen(pipe2_d[1], "w");
		FILE *pipe2_r = fdopen(pipe2_u[0], "r");

		if(pipe1_w == 0 || pipe1_r == 0 || pipe2_w == 0 || pipe2_w == 0)
			err_sys("depth = 1, pipe error!!!");
		int player[4];
		scanf("%d %d %d %d", &player[0], &player[1], &player[2], &player[3]);
		while(player[0] != -1){

			fprintf(pipe1_w, "%d %d\n", player[0], player[1]);
			fprintf(pipe2_w, "%d %d\n", player[2], player[3]);
			flush_sync(pipe1_w, pipe1_d[1]);
			flush_sync(pipe2_w, pipe2_d[1]);

			int p1_id, p2_id, p1_bid, p2_bid;
			for(int i = 0; i < 10; i ++){
				fscanf(pipe1_r, "%d %d", &p1_id, &p1_bid);
				fscanf(pipe2_r, "%d %d", &p2_id, &p2_bid);
				int w_id = (p1_bid > p2_bid) ? p1_id : p2_id;
				int w_bid = (p1_bid > p2_bid) ? p1_bid : p2_bid;
				printf("%d %d\n", w_id, w_bid);
				flush_sync(stdout, STDOUT_FILENO);
			}
			scanf("%d %d %d %d", &player[0], &player[1], &player[2], &player[3]);
			//printf("%d\n", player[0]);
		}
		fprintf(pipe1_w, "-1 -1\n");
		fprintf(pipe2_w, "-1 -1\n");
		flush_sync(pipe1_w, pipe1_d[1]);
		flush_sync(pipe2_w, pipe2_d[1]);
		wait(NULL);
		wait(NULL);
		fclose(pipe1_w);
		fclose(pipe2_w);
		fclose(pipe1_r);
		fclose(pipe2_r);
		exit(0);
	}else{
		int player[2];
		scanf("%d %d", &player[0], &player[1]);
		while(player[0] != -1){
			create_child(&cpid_1, &cpid_2, pipe1_d, pipe1_u, pipe2_d, pipe2_u, argv[1], argv[2], depth, player[0], player[1]);
			FILE *pipe1_w = fdopen(pipe1_d[1], "w");
			FILE *pipe1_r = fdopen(pipe1_u[0], "r");
			FILE *pipe2_w = fdopen(pipe2_d[1], "w");
			FILE *pipe2_r = fdopen(pipe2_u[0], "r");

			if(pipe1_w == 0 || pipe1_r == 0 || pipe2_w == 0 || pipe2_w == 0)
				err_sys("depth = 2, pipe error!!!");
			

			int p1_id, p2_id, p1_bid, p2_bid;
			for(int i = 0; i < 10; i ++){
				fscanf(pipe1_r, "%d %d", &p1_id, &p1_bid);
				fscanf(pipe2_r, "%d %d", &p2_id, &p2_bid);
				int w_id = (p1_bid > p2_bid) ? p1_id : p2_id;
				int w_bid = (p1_bid > p2_bid) ? p1_bid : p2_bid;
				printf("%d %d\n", w_id, w_bid);
				flush_sync(stdout, STDOUT_FILENO);
			}
			wait(NULL);
			wait(NULL);
			fclose(pipe1_w);
			fclose(pipe2_w);
			fclose(pipe1_r);
			fclose(pipe2_r);
			scanf("%d %d", &player[0], &player[1]);
		}
		exit(0);
	}
	exit(0);
	return 0;
}
