#include <stdlib.h>
#include <vector>
#include <math.h>
#include "String.h"

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

class Node;
int max(vector<Node*> *, double);
int MCS_sim(int[BOUNDARYSIZE][BOUNDARYSIZE], String*[BOUNDARYSIZE][BOUNDARYSIZE], int&, String*[200], int, int, bool);
int gen_legal_move(int[BOUNDARYSIZE][BOUNDARYSIZE], String*[BOUNDARYSIZE][BOUNDARYSIZE], int, int, int[MAXGAMELENGTH][BOUNDARYSIZE][BOUNDARYSIZE], int[HISTORYLENGTH], int, bool);
int do_move(int[BOUNDARYSIZE][BOUNDARYSIZE], String*[BOUNDARYSIZE][BOUNDARYSIZE], int&, String*[200], int, int);

class Node {
public:
	Node(int board[BOUNDARYSIZE][BOUNDARYSIZE], String* sboard[BOUNDARYSIZE][BOUNDARYSIZE], int s_num, String* S[200], int t, int move, Node* n, int l)
		: score_square(0), score_num(0), score_sum(0), all_num(0), turn(t), parent(n), children(0), pruned(false), layer(l + 1) {
		int i, j;
		for (i = 0; i < BOUNDARYSIZE; ++i) {
			for (j = 0; j < BOUNDARYSIZE; ++j) {
				Board[i][j] = board[i][j];
				sBoard[i][j] = NULL;
			}
		}
		string_num = 0;
		for (i = 0; i < s_num; ++i) {
			if (S[i] != NULL) {
				strings[string_num] = new String(S[i], sBoard);
				strings[string_num]->location = string_num;
				++string_num;
			}
		}
		ko = do_move(Board, sBoard, string_num, strings, turn, move);
	}

	Node(int board[BOUNDARYSIZE][BOUNDARYSIZE], String* sboard[BOUNDARYSIZE][BOUNDARYSIZE], int s_num, String* S[200], int t, int num_legal_moves, int MoveList[HISTORYLENGTH], Node* n)
		: score_square(0), score_num(0), score_sum(0), all_num(0), parent(n), children(0), pruned(false), string_num(0), layer(0) {
		for (int i = 0; i < num_legal_moves; ++i) {
			children.push_back(new Node(board, sboard, s_num, S, t, MoveList[i], this, layer));
			children[i]->sim();
		}
	}

	~Node() {
		for (int i = 0; i < string_num; ++i) {
			if (strings[i] != NULL) {
				delete strings[i];
			}
		}
		//cerr << layer << " ";
		for (int i = 0; i < (int)children.size(); ++i) delete children[i];
	}
	
	void sim() {
		double square = 0, sum = 0, score;
		int num = 0;
		int NextBoard[BOUNDARYSIZE][BOUNDARYSIZE];
		String* NextsBoard[BOUNDARYSIZE][BOUNDARYSIZE];
		String* next_strings[200];
		int next_string_num;
		int a, b;
		for (int i = 0; i < 4; ++i) {
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
			//cerr << string_num << " " << ko << " " << turn << endl;
			score = (double)MCS_sim(NextBoard, NextsBoard, next_string_num, next_strings, turn, ko, false) / 81;
			for (int i = 0; i < next_string_num; ++i) {
				if (next_strings[i] != NULL) delete next_strings[i];
			}
			square += (score * score);
			sum += score;
			++num;
			extern int COUNT;
			++COUNT;
		}
		update(square, sum, num, false);
	}

	void update(double square, double sum, int num, bool prune) {
		score_square += square;
		score_sum += sum;
		score_num += num;
		if (!prune) all_num += num;
		if (parent != NULL) parent->update(square, sum, num, prune);
	}

	bool grow(clock_t end_t) {
		if (children.empty()) {
			int next_turn = (turn == BLACK) ? WHITE : BLACK;
			int MoveList[HISTORYLENGTH];
			int num_legal_moves = gen_legal_move(Board, sBoard, next_turn, 0, NULL, MoveList, ko, false);
			//cerr << num_legal_moves << " ";
			if (num_legal_moves == 0) return false;
			for (int i = 0; i < num_legal_moves; ++i) {
				children.push_back(new Node(Board, sBoard, string_num, strings, next_turn, MoveList[i], this, layer));
				children[i]->sim();
				if (clock() > end_t) return true;
			}
		}
		else {
			if (!children[max(&children, 0.1)]->grow(end_t)) {
				if (parent == NULL) {
					for (int i = 0; i < (int)children.size(); ++i) {
						children[i]->sim();
						if (clock() > end_t) return true;
					}
				}
				return false;
			}
		}
		return true;
	}

	double eval(double c) {
		double score = score_sum / score_num;
		if (turn == WHITE) score = -score;
		return score + c * sqrt(log10((double)parent->score_num) / (double)score_num);
	}

	void prune(double rd, double se) {
		int n = (int)children.size(), i;
		if (n == 0 || all_num < 300) return;
		double* m = new double[n];
		double* d = new double[n];
		double max_l = -10, score_l;
		for (i = 0; i < n; ++i) {
			m[i] = children[i]->score_sum / children[i]->score_num;
			if (children[i]->turn == WHITE) m[i] = -m[i];
			d[i] = rd * sqrt(children[i]->score_square / children[i]->score_num - m[i] * m[i]);
			score_l = m[i] - d[i];
			if (max_l < score_l && d[i] < se) max_l = score_l;
		}
		extern int PRUNE;
		for (i = 0; i < n; ++i) {
			if (m[i] + d[i] < max_l && d[i] < se) {
				if (!children[i]->pruned) {
					++PRUNE;
					update(-children[i]->score_square, -children[i]->score_sum, -children[i]->score_num, true);
				}
				children[i]->pruned = true;
			}
			else {
				if (children[i]->pruned) {
					--PRUNE;
					update(children[i]->score_square, children[i]->score_sum, children[i]->score_num, true);
				}
				children[i]->pruned = false;
				children[i]->prune(rd, se);
			}
		}
		delete m, delete d;
	}

	int Board[BOUNDARYSIZE][BOUNDARYSIZE];
	String* sBoard[BOUNDARYSIZE][BOUNDARYSIZE];
	String* strings[200];
	int string_num;
	int ko;
	double score_square, score_sum;
	int score_num, all_num;
	int turn;
	Node* parent;
	vector<Node*> children;
	bool pruned;
	int layer;
};
