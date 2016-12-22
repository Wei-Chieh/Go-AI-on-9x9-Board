/*
 * This code is provied as a sample code of Hw 2 of "Theory of Computer Game".
 * The "genmove" function will randomly output one of the legal move.
 * This code can only be used within the class.
 *
 * 2015 Nov. Hung-Jui Chang
 * */
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <time.h>
#include "Node.h"
#include "String.h"
//#include "myUsage.h"

#define BOARDSIZE        9
#define BOUNDARYSIZE    11
#define COMMANDLENGTH 1000
#define DEFAULTTIME     10
#define DEFAULTKOMI      7

#define MAXGAMELENGTH 1000
#define MAXSTRING       50
#define MAXDIRECTION     4

#define NUMINTERSECTION 81
#define HISTORYLENGTH   200

#define EMPTY            0
#define BLACK            1
#define WHITE            2
#define BOUNDARY         3

#define SELF             1
#define OPPONENT         2

#define NUMGTPCOMMANDS      15

#define LOCALVERSION      1
#define GTPVERSION        2
 
using namespace std;
int _board_size = BOARDSIZE;
int _board_boundary = BOUNDARYSIZE;
double _komi =  DEFAULTKOMI;
const int DirectionX[MAXDIRECTION] = {-1, 0, 1, 0};
const int DirectionY[MAXDIRECTION] = { 0, 1, 0,-1};
const int CornerX[MAXDIRECTION] = { 1, 1, -1, -1 };
const int CornerY[MAXDIRECTION] = { 1, -1, -1, 1 };
const char LabelX[]="0ABCDEFGHJ";

int COUNT = 0;
int PRUNE = 0;
int move_list[81];
//MyUsage usage;

void record(int[BOUNDARYSIZE][BOUNDARYSIZE], int[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int);
double final_score(int[BOUNDARYSIZE][BOUNDARYSIZE]);
int do_move(int[BOUNDARYSIZE][BOUNDARYSIZE], String*[BOUNDARYSIZE][BOUNDARYSIZE], int&, String*[200], int, int);

/*
 * This function reset the board, the board intersections are labeled with 0,
 * the boundary intersections are labeled with 3.
 * */
void reset(int Board[BOUNDARYSIZE][BOUNDARYSIZE]) {
    for (int i = 1 ; i <= BOARDSIZE; ++i) {
		for (int j = 1 ; j <= BOARDSIZE; ++j) {
			Board[i][j] = EMPTY;
		}
    }
    for (int i = 0 ; i < BOUNDARYSIZE; ++i) {
		Board[0][i] = Board[BOUNDARYSIZE-1][i] = Board[i][0] = Board[i][BOUNDARYSIZE-1] = BOUNDARY;
    }
}

/*
 * This function return the total number of liberity of the string of (X, Y) and
 * the string will be label with 'label'.
 * */
int find_liberty(int X, int Y, int label, int Board[BOUNDARYSIZE][BOUNDARYSIZE], int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE]) {
    // Label the current intersection
    ConnectBoard[X][Y] |= label;
    int total_liberty = 0;
    for (int d = 0 ; d < MAXDIRECTION; ++d) {
		// Check this intersection has been visited or not
		if ((ConnectBoard[X+DirectionX[d]][Y+DirectionY[d]] & (1<<label) )!= 0) continue;

		// Check this intersection is not visited yet
		ConnectBoard[X+DirectionX[d]][Y+DirectionY[d]] |=(1<<label);
		// This neighboorhood is empty
		if (Board[X+DirectionX[d]][Y+DirectionY[d]] == EMPTY){
			total_liberty++;
		}
		// This neighboorhood is self stone
		else if (Board[X+DirectionX[d]][Y+DirectionY[d]] == Board[X][Y]) {
			total_liberty += find_liberty(X+DirectionX[d], Y+DirectionY[d], label, Board, ConnectBoard);
		}
    }
    return total_liberty;
}

/*
 * This function count the liberties of the given intersection's neighboorhod
 * */
void count_liberty(int X, int Y, int Board[BOUNDARYSIZE][BOUNDARYSIZE], int Liberties[MAXDIRECTION]) {
    int ConnectBoard[BOUNDARYSIZE][BOUNDARYSIZE];
    // Initial the ConnectBoard
    for (int i = 0 ; i < BOUNDARYSIZE; ++i) {
		for (int j = 0 ; j < BOUNDARYSIZE; ++j) {
			ConnectBoard[i][j] = 0;
		}
    }
    // Find the same connect component and its liberity
    for (int d = 0 ; d < MAXDIRECTION; ++d) {
		Liberties[d] = 0;
		if (Board[X+DirectionX[d]][Y+DirectionY[d]] == BLACK ||  
			Board[X+DirectionX[d]][Y+DirectionY[d]] == WHITE    ) {
			Liberties[d] = find_liberty(X+DirectionX[d], Y+DirectionY[d], d, Board, ConnectBoard);
		}
    }
}

/*
 * This function count the number of empty, self, opponent, and boundary intersections of the neighboorhod
 * and saves the type in NeighboorhoodState.
 * */
void count_neighboorhood_state(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn, int* empt, int* self, int* oppo ,int* boun, int NeighboorhoodState[MAXDIRECTION]) {
    for (int d = 0 ; d < MAXDIRECTION; ++d) {
	// check the number of nonempty neighbor
		switch(Board[X+DirectionX[d]][Y+DirectionY[d]]) {
		    case EMPTY:
				(*empt)++; 
				NeighboorhoodState[d] = EMPTY;
				break;
		    case BLACK:
				if (turn == BLACK) {
					(*self)++;
					NeighboorhoodState[d] = SELF;
				}
				else {
					(*oppo)++;
					NeighboorhoodState[d] = OPPONENT;
				}
				break;
		    case WHITE:
				if (turn == WHITE) {
					(*self)++;
					NeighboorhoodState[d] = SELF;
				}
				else {
					(*oppo)++;
					NeighboorhoodState[d] = OPPONENT;
				}
				break;
		    case BOUNDARY:
				(*boun)++;
				NeighboorhoodState[d] = BOUNDARY;
				break;
		}
    }
}

/*
 * This function remove the connect component contains (X, Y) with color "turn" 
 * And return the number of remove stones.
 * */
int remove_piece(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn) {
    int remove_stones = (Board[X][Y] == EMPTY)? 0:1;
    Board[X][Y] = EMPTY;
    for (int d = 0; d < MAXDIRECTION; ++d) {
		if (Board[X+DirectionX[d]][Y+DirectionY[d]] == turn) {
		    remove_stones += remove_piece(Board, X+DirectionX[d], Y+DirectionY[d], turn);
		}
    }
    return remove_stones;
}
/*
 * This function update Board with place turn's piece at (X,Y).
 * Note that this function will not check if (X, Y) is a legal move or not.
 * */
void update_board(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int X, int Y, int turn) {
    int num_neighborhood_self = 0;
    int num_neighborhood_oppo = 0;
    int num_neighborhood_empt = 0;
    int num_neighborhood_boun = 0;
    int Liberties[4];
    int NeighboorhoodState[4];
    count_neighboorhood_state(Board, X, Y, turn,
	    &num_neighborhood_empt,
	    &num_neighborhood_self,
	    &num_neighborhood_oppo,
	    &num_neighborhood_boun, NeighboorhoodState);
    // check if there is opponent piece in the neighboorhood
    if (num_neighborhood_oppo != 0) {
		count_liberty(X, Y, Board, Liberties);
		for (int d = 0 ; d < MAXDIRECTION; ++d) {
			// check if there is opponent component only one liberty
			if (NeighboorhoodState[d] == OPPONENT && Liberties[d] == 1 && Board[X+DirectionX[d]][Y+DirectionY[d]]!=EMPTY) {
				remove_piece(Board, X+DirectionX[d], Y+DirectionY[d], Board[X+DirectionX[d]][Y+DirectionY[d]]);
			}
		}
    }
    Board[X][Y] = turn;
}

bool check_legal(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int move, int ko) {
	if (move == ko) return false;
	int x = move / 10;
	int y = move % 10;
	int i, j, x2, y2;
	int oppo = (turn == BLACK) ? WHITE : BLACK;
	int bound = 0, empty = 0, self = 0, self_n = 0, self_s = 0;
	String* s[4];
	bool exist, c_empty = true;
	if (Board[x][y]) return false;
	for (i = 0; i < MAXDIRECTION; ++i) {
		x2 = x + CornerX[i], y2 = y + CornerY[i];
		if (Board[x2][y2] == BLACK || Board[x2][y2] == WHITE)
			c_empty = false;
	}
	for (i = 0; i < MAXDIRECTION; ++i) {
		x2 = x + DirectionX[i], y2 = y + DirectionY[i];
		if (!Board[x2][y2]) ++empty;
		else if (Board[x2][y2] == BOUNDARY) ++bound;
		else if (Board[x2][y2] == oppo) {
			if (sBoard[x2][y2]->liberty == 1) return true;
		}
		else {
			++self;
			if (sBoard[x2][y2]->liberty == 1) ++self_n;
			exist = false;
			for (j = 0; j < self_s; ++j) {
				if (sBoard[x2][y2] == s[j]) {
					exist = true;
					break;
				}
			}
			if (!exist) {
				s[self_s] = sBoard[x2][y2];
				++self_s;
			}
		}
	}
	if (bound && bound + empty == 4 && c_empty) return false;
	if (empty) return true;
	if (self == self_n) return false;
	if (self + bound == 4 && self_s == 1) return false;
	return true;
}

int gen_legal_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int game_length, int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int MoveList[HISTORYLENGTH], int ko, bool formal) {
	int NextBoard[BOUNDARYSIZE][BOUNDARYSIZE];
	int legal_moves = 0;
	int move;
	for (int x = 1; x <= BOARDSIZE; ++x) {
		for (int y = 1; y <= BOARDSIZE; ++y) {
			if (formal) {
				if (game_length < 4 && (Board[3][3] == 0 || Board[3][7] == 0 || Board[7][3] == 0 || Board[7][7] == 0)) {
					if ((x != 3 && x != 7) || (y != 3 && y != 7)) continue;
				}
			}
			move = x * 10 + y;
			if (check_legal(Board, sBoard, turn, move, ko)) {
				if (!game_length) {
					MoveList[legal_moves] = x * 10 + y;
					++legal_moves;
					continue;
				}
				for (int i = 0; i < BOUNDARYSIZE; ++i) {
					for (int j = 0; j < BOUNDARYSIZE; ++j) {
						NextBoard[i][j] = Board[i][j];
					}
				}
				update_board(NextBoard, x, y, turn);
				bool repeat_move = 0;
				for (int t = 0; t < game_length; ++t) {
					bool repeat_flag = 1;
					for (int i = 1; i <= BOARDSIZE; ++i) {
						for (int j = 1; j <= BOARDSIZE; ++j) {
							if (NextBoard[i][j] != GameRecord[t][i][j]) {
								repeat_flag = 0;
							}
						}
					}
					if (repeat_flag == 1) {
						repeat_move = 1;
						break;
					}
				}
				if (repeat_move == 0) {
					MoveList[legal_moves] = x * 10 + y;
					++legal_moves;
				}
			}
		}
	}
	return legal_moves;
}

/*
 * This function randomly selects one move from the MoveList.
 * */
int rand_pick_move(int num_legal_moves, int MoveList[HISTORYLENGTH]) {
    if (num_legal_moves == 0)
		return 0;
    else {
		int move_id = rand() % num_legal_moves;
		return MoveList[move_id];
    }
}

int MCS_sim(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String * sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int& string_num, String* strings[200], int turn, int ko, bool pass) {
	int rand_list[81];
	for (int i = 0; i < 81; ++i)
		rand_list[i] = move_list[i];
	int idx, move, x, y, size = 81;
	int next_turn = (turn == BLACK) ? WHITE : BLACK;
	int next_ko;
	while (1) {
		idx = rand() % size;
		move = rand_list[idx];
		x = move / 10;
		y = move % 10;
		if (check_legal(Board, sBoard, turn, move, ko)) break;
		rand_list[idx] = rand_list[--size];
		if (!size) {
			if (pass) return (int)final_score(Board);
			else {
				next_ko = 0;
				return MCS_sim(Board, sBoard, string_num, strings, next_turn, next_ko, true);
			}
		}
	}
	next_ko = do_move(Board, sBoard, string_num, strings, turn, move);
	return MCS_sim(Board, sBoard, string_num, strings, next_turn, next_ko, false);
}

int MCSpure_pick_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String * sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int string_num, String* strings[200], int turn, int num_legal_moves, int MoveList[HISTORYLENGTH], clock_t end_t) {
	if (num_legal_moves == 0) return 0;

	int NextBoard[BOUNDARYSIZE][BOUNDARYSIZE];
	String* NextsBoard[BOUNDARYSIZE][BOUNDARYSIZE];
	String* next_strings[200];
	int next_string_num;
	int ko;

	int scores[HISTORYLENGTH], nums[HISTORYLENGTH];
	int a, b, i, next_turn;
	double Max_score, temp_score;
	int Max_index;
	int score = 0;

	int count = 0;

	for (i = 0; i < num_legal_moves; ++i) scores[i] = nums[i] = 0; 
	while(1) {
		for (i = 0; i < num_legal_moves; ++i) {
			for (a = 0; a < BOUNDARYSIZE; ++a) {
				for (b = 0; b < BOUNDARYSIZE; ++b) {
					NextBoard[a][b] = Board[a][b];
					NextsBoard[a][b] = NULL;
				}
			}
			next_string_num = 0;
			for (a = 0; a < string_num; ++a) {
				if (strings[a] != NULL) {
					next_strings[next_string_num] = new String(strings[a], NextsBoard);
					next_strings[next_string_num]->location = next_string_num;
					++next_string_num;
				}
			}
			ko = do_move(NextBoard, NextsBoard, next_string_num, next_strings, turn, MoveList[i]);
			next_turn = (turn == BLACK) ? WHITE : BLACK;
			score = MCS_sim(NextBoard, NextsBoard, next_string_num, next_strings, next_turn, ko, false) - (int)_komi;
			for (a = 0; a < next_string_num; ++a) {
				if (next_strings[a] != NULL) delete next_strings[a];
			}
			if ((turn == BLACK && score > 0) || (turn == WHITE && score < 0)) ++scores[i];
			++nums[i];
			++count;
			if (clock() > end_t - 30000) break;
		}
		if (clock() > end_t - 30000) break;
	}
	Max_score = 0;
	for (i = 0; i < num_legal_moves; ++i) {
		temp_score = (double)scores[i] / (double)nums[i];
		if (Max_score < temp_score) {
			Max_score = temp_score;
			Max_index = i;
		}
	}
	cerr << count << endl;
	return MoveList[Max_index];
}

int max(vector<Node*>* V, double c) {
	int idx;
	double max_score, score;
	max_score = -10;
	for (int i = 0; i < (int)V->size(); ++i) {
		if ((*V)[i]->pruned) continue;
		score = (*V)[i]->eval(c);
		if (max_score < score) {
			max_score = score;
			idx = i;
		}
	}
	return idx;
}

int UCT_pick_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int string_num, String* strings[200], int turn, int num_legal_moves, int MoveList[HISTORYLENGTH], clock_t end_t) {
	if (num_legal_moves == 0) return 0;
	COUNT = 0;
	PRUNE = 0;
	Node root(Board, sBoard, string_num, strings, turn, num_legal_moves, MoveList, NULL);
	while (clock() < end_t - 100000) {
		root.prune(1, 0.5);
		root.grow(end_t - 100000);
	}
	int idx = max(&root.children, 0);
	cerr << "sim: " << COUNT << " prune: " << PRUNE << endl;
	return MoveList[idx];
}

int do_move(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int& string_num, String* strings[200], int turn, int move) {
	int x = move / 10, y = move % 10;
	int i, j, x2, y2, ko = 0, liberty = 0;
	int oppo = (turn == BLACK) ? WHITE : BLACK;
	String *s[4], *o[4], *e[4];
	int s_num = 0, o_num = 0, e_num = 0;
	bool exist;
	for (i = 0; i < 4; ++i) {
		exist = false;
		x2 = x + DirectionX[i], y2 = y + DirectionY[i];
		if (Board[x2][y2] == turn) {
			for (j = 0; j < s_num; ++j) {
				if (sBoard[x2][y2] == s[j]) {
					exist = true;
					break;
				}
			}
			if (!exist) {
				s[s_num] = sBoard[x2][y2];
				++s_num;
			}
		}
		else if (Board[x2][y2] == oppo) {
			if (sBoard[x2][y2]->liberty == 1) {
				++liberty;
				for (j = 0; j < e_num; ++j) {
					if (sBoard[x2][y2] == e[j]) {
						exist = true;
						break;
					}
				}
				if (!exist) {
					e[e_num] = sBoard[x2][y2];
					++e_num;
				}
			}
			else {
				for (j = 0; j < o_num; ++j) {
					if (sBoard[x2][y2] == o[j]) {
						exist = true;
						break;
					}
				}
				if (!exist) {
					o[o_num] = sBoard[x2][y2];
					++o_num;
				}
			}
		}
		else if (Board[x2][y2] == EMPTY) ++liberty;
	}
	for (i = 0; i < o_num; ++i) --o[i]->liberty;
	if (e_num == 1 && s_num == 0 && e[0]->size == 1) ko = e[0]->member[0][0] * 10 + e[0]->member[0][1];
	for (i = 0; i < e_num; ++i) e[i]->remove(Board, sBoard, strings);
	Board[x][y] = turn;
	if (s_num == 0) {
		sBoard[x][y] = new String();
		sBoard[x][y]->push_back(x, y);
		sBoard[x][y]->liberty = liberty;
		strings[string_num] = sBoard[x][y];
		sBoard[x][y]->location = string_num;
		++string_num;
	}
	else if (s_num == 1) {
		s[0]->add(Board, sBoard, x, y);
		sBoard[x][y] = s[0];
	}
	else {
		sBoard[x][y] = s[0];
		s[0]->push_back(x, y);
		for (i = 1; i < s_num; ++i) s[0]->merge(s[i], sBoard, strings);
		s[0]->count_liberty(Board);
	}
	return ko;
}

/* 
 * This function records the current game baord with current
 * game length "game_length"
 * */
void record(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int game_length) {
	for (int i = 0 ; i < BOUNDARYSIZE; ++i) {
		for (int j = 0 ; j < BOUNDARYSIZE; ++j) {
			GameRecord[game_length][i][j] = Board[i][j];
		}
	}
}

void construct_string(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE], int x, int y, String* s) {
	int x2, y2;
	s->push_back(x, y);
	sBoard[x][y] = s;
	for (int i = 0; i < MAXDIRECTION; ++i) {
		x2 = x + DirectionX[i], y2 = y + DirectionY[i];
		if (Board[x2][y2] == Board[x][y] && sBoard[x2][y2] == NULL)
			construct_string(Board, sBoard, x2, y2, s);
	}
}

int find_string(int Board[BOUNDARYSIZE][BOUNDARYSIZE], String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE], String* strings[200]) {
	int i, j, num = 0;
	String* s;
	for (i = 1; i <= BOARDSIZE; ++i) {
		for (j = 0; j <= BOARDSIZE; ++j)
			sBoard[i][j] = NULL;
	}
	for (i = 1; i <= BOARDSIZE; ++i) {
		for (j = 1; j <= BOARDSIZE; ++j) {
			if (Board[i][j] && sBoard[i][j] == NULL) {
				s = new String();
				strings[num] = s;
				s->location = num;
				++num;
				construct_string(Board, sBoard, i, j, s);
				s->count_liberty(Board);
			}
		}
	}
	return num;
}
/* 
 * This function randomly generate one legal move (x, y) with return value x*10+y,
 * if there is no legal move the function will return 0.
 * */
int genmove(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int turn, int time_limit, int game_length, int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]) {
    clock_t start_t, end_t;
    // record start time
    start_t = clock();
    // calculate the time bound
	end_t = start_t + CLOCKS_PER_SEC * time_limit;

	//usage.setTimeUsage();

	String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE];
	String* strings[200];
	int string_num = find_string(Board, sBoard, strings);
	//cerr << (double)(clock() - start_t) / CLOCKS_PER_SEC << endl;

    int MoveList[HISTORYLENGTH];
    int num_legal_moves = 0;
    int return_move = 0;

	num_legal_moves = gen_legal_move(Board, sBoard, turn, game_length, GameRecord, MoveList, 0, true);

    //return_move = rand_pick_move(num_legal_moves, MoveList);
	//return_move = MCSpure_pick_move(Board, sBoard, string_num, strings, turn, num_legal_moves, MoveList, end_t);
	return_move = UCT_pick_move(Board, sBoard, string_num, strings, turn, num_legal_moves, MoveList, end_t);
	update_board(Board, return_move / 10, return_move % 10, turn);

	for (int i = 0; i < string_num; ++i) {
		if (strings[i] != NULL) delete strings[i];
	}
	//usage.report(true, true);
	//cerr << (double)(clock() - start_t) / CLOCKS_PER_SEC << endl;
    return return_move;
}
/*
 * This function counts the number of points remains in the board by Black's view
 * */
double final_score(int Board[BOUNDARYSIZE][BOUNDARYSIZE]) {
    int black, white;
    black = white = 0;
    int is_black, is_white;
    for (int i = 1 ; i <= BOARDSIZE; ++i) {
		for (int j = 1; j <= BOARDSIZE; ++j) {
			switch(Board[i][j]) {
				case EMPTY:
					is_black = is_white = 0;
					for(int d = 0 ; d < MAXDIRECTION; ++d) {
						if (Board[i+DirectionX[d]][j+DirectionY[d]] == BLACK) is_black = 1;
						if (Board[i+DirectionX[d]][j+DirectionY[d]] == WHITE) is_white = 1;
					}
					if (is_black + is_white == 1) {
						black += is_black;
						white += is_white;
					}
					break;
				case WHITE:
					white++;
					break;
				case BLACK:
					black++;
					break;
			}
		}
    }
    return black - white;
}
/* 
 * Following are commands for Go Text Protocol (GTP)
 *
 * */
const char *KnownCommands[]={
    "protocol_version",
    "name",
    "version",
    "known_command",
    "list_commands",
    "quit",
    "boardsize",
    "clear_board",
    "komi",
    "play",
    "genmove",
    "undo",
    "quit",
    "showboard",
    "final_score"
};

void gtp_final_score(int Board[BOUNDARYSIZE][BOUNDARYSIZE]) {
    double result;
    result = final_score(Board);
    result -= _komi;
    cout << "= ";
    if (result > 0.0) { // Black win
		cout << "B+" << result << endl << endl<< endl;;
    }
    if (result < 0.0) { // White win
		cout << "W+" << -result << endl << endl<< endl;;
    }
    else { // draw
		cout << "0" << endl << endl<< endl;;
    }
}
void gtp_undo(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int game_length, int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]) {
    if (game_length!=0) {
		for (int i = 1; i <= BOARDSIZE; ++i) {
			for (int j = 1; j <= BOARDSIZE; ++j) {
			Board[i][j] = GameRecord[game_length][i][j];
			}
		}
    }
    cout << "= " << endl << endl;
}
void gtp_showboard(int Board[BOUNDARYSIZE][BOUNDARYSIZE]) {
    for (int i = 1; i <=BOARDSIZE; ++i) {
		cout << "#";
		cout <<10-i;
		for (int j = 1; j <=BOARDSIZE; ++j) {
			switch(Board[i][j]) {
				case EMPTY: cout << " .";break;
				case BLACK: cout << " X";break;
				case WHITE: cout << " O";break;
			}
		}
		cout << endl;
    }
    cout << "#  ";
    for (int i = 1; i <=BOARDSIZE; ++i) 
		cout << LabelX[i] <<" ";
    cout << endl;
    cout << endl;

}
void gtp_protocol_version() {
    cout <<"= 2"<<endl<< endl;
}
void gtp_name() {
    cout <<"= TCG-Go99" << endl<< endl;
}
void gtp_version() {
    cout << "= 1.02" << endl << endl;
}
void gtp_list_commands(){
    cout <<"= ";
    for (int i = 0 ; i < NUMGTPCOMMANDS; ++i) {
		cout <<KnownCommands[i] << endl;
    }
    cout << endl;
}
void gtp_known_command(const char Input[]) {
    for (int i = 0 ; i < NUMGTPCOMMANDS; ++i) {
		if (strcmp(Input, KnownCommands[i])==0) {
			cout << "= true" << endl<< endl;
			return;
		}
    }
    cout << "= false" << endl<< endl;
}
void gtp_boardsize(int size) {
    if (size!=9) {
		cout << "? unacceptable size" << endl<< endl;
    }
    else {
		_board_size = size;
		cout << "= "<<endl<<endl;
    }
}
void gtp_clear_board(int Board[BOUNDARYSIZE][BOUNDARYSIZE], int NumCapture[]) {
    reset(Board);
    NumCapture[BLACK] = NumCapture[WHITE] = 0;
    cout << "= "<<endl<<endl;
}
void gtp_komi(double komi) {
    _komi = komi;
    cout << "= "<<endl<<endl;
}
void gtp_play(char Color[], char Move[], int Board[BOUNDARYSIZE][BOUNDARYSIZE], int game_length, int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]) {
    int turn, move_i, move_j;
    if (Color[0] =='b' || Color[0] == 'B')
		turn = BLACK;
    else
	turn = WHITE;
    if (strcmp(Move, "PASS") == 0 || strcmp(Move, "pass")==0) {
		record(Board, GameRecord, game_length+1);
    }
    else {
		// [ABCDEFGHJ][1-9], there is no I in the index.
		Move[0] = toupper(Move[0]);
		move_j = Move[0]-'A'+1;
		if (move_j == 10) move_j = 9;
		move_i = 10-(Move[1]-'0');
		update_board(Board, move_i, move_j, turn);
		record(Board, GameRecord, game_length+1);
    }
    cout << "= "<<endl<<endl;
}
void gtp_genmove(int Board[BOUNDARYSIZE][BOUNDARYSIZE], char Color[], int time_limit, int game_length, int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]){
	clock_t start_t = clock();
	int turn = (Color[0]=='b'||Color[0]=='B')?BLACK:WHITE;
    int move = genmove(Board, turn, time_limit, game_length, GameRecord);
    int move_i, move_j;
    record(Board, GameRecord, game_length+1);
    if (move==0) {
		cout << "= PASS" << endl<< endl<< endl;
    }
    else {
		move_i = (move%100)/10;
		move_j = (move%10);
//		cerr << "#turn("<<game_length<<"): (move, move_i,move_j)" << turn << ": " << move<< " " << move_i << " " << move_j << endl;
		cout << "= " << LabelX[move_j]<<10-move_i<<endl<< endl<<flush;
    }
	cerr << "time: " << (double)(clock() - start_t) / CLOCKS_PER_SEC << " s" << endl;
}
/*
 * This main function is used of the gtp protocol
 * */
void gtp_main(int display) {
    char Input[COMMANDLENGTH]="";
    char Command[COMMANDLENGTH]="";
    char Parameter[COMMANDLENGTH]="";
    char Move[4]="";
    char Color[6]="";
    int ivalue;
    double dvalue;
    int Board[BOUNDARYSIZE][BOUNDARYSIZE]={{0}};
    int NumCapture[3]={0};// 1:Black, 2: White
    int time_limit = DEFAULTTIME;
    int GameRecord[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE]={{{0}}};
    int game_length = 0;
    if (display==1) {
		gtp_list_commands();
		gtp_showboard(Board);
    }
    while (fgets(Input, sizeof(Input), stdin) != 0) {
		sscanf(Input, "%s", Command);
		if (Command[0]== '#')
			continue;
	
		if (strcmp(Command, "protocol_version")==0) {
			gtp_protocol_version();
		}
		else if (strcmp(Command, "name")==0) {
			gtp_name();
		}
		else if (strcmp(Command, "version")==0) {
			gtp_version();
		}
		else if (strcmp(Command, "list_commands")==0) {
			gtp_list_commands();
		}
		else if (strcmp(Command, "known_command")==0) {
			sscanf(Input, "known_command %s", Parameter);
			gtp_known_command(Parameter);
		}
		else if (strcmp(Command, "boardsize")==0) {
			sscanf(Input, "boardsize %d", &ivalue);
			gtp_boardsize(ivalue);
		}
		else if (strcmp(Command, "clear_board")==0) {
			gtp_clear_board(Board, NumCapture);
			game_length = 0;
		}
		else if (strcmp(Command, "komi")==0) {
			sscanf(Input, "komi %lf", &dvalue);
			gtp_komi(dvalue);
		}
		else if (strcmp(Command, "play")==0) {
			sscanf(Input, "play %s %s", Color, Move);
			gtp_play(Color, Move, Board, game_length, GameRecord);
			game_length++;
			if (display==1) {
				gtp_showboard(Board);
			}
		}
		else if (strcmp(Command, "genmove")==0) {
			sscanf(Input, "genmove %s", Color);
			gtp_genmove(Board, Color, time_limit, game_length, GameRecord);
			game_length++;
			if (display==1) {
				gtp_showboard(Board);
			}
		}
		else if (strcmp(Command, "quit")==0) {
			break;
		}
		else if (strcmp(Command, "showboard")==0) {
			gtp_showboard(Board);
		}
		else if (strcmp(Command, "undo")==0) {
			game_length--;
			gtp_undo(Board, game_length, GameRecord);
			if (display==1) {
				gtp_showboard(Board);
			}
		}
		else if (strcmp(Command, "final_score")==0) {
			if (display==1) {
				gtp_showboard(Board);
			}
			gtp_final_score(Board);
		}
    }
}
int main(int argc, char* argv[]) {
//    int type = GTPVERSION;// 1: local version, 2: gtp version
    int type = GTPVERSION;// 1: local version, 2: gtp version
    int display = 0; // 1: display, 2 nodisplay
    if (argc > 1) {
		if (strcmp(argv[1], "-display")==0) {
			display = 1;
		}
		if (strcmp(argv[1], "-nodisplay")==0) {
			display = 0;
		}
    }
	srand((unsigned int)time(NULL));
	for (int i = 0; i < 81; ++i)
		move_list[i] = (i / 9) * 10 + (i % 9) + 11;
    gtp_main(display);
    return 0;
}
