#include <stdlib.h>
#include <vector>

#include <iostream>

using namespace std;

#ifndef STRING
#define STRING

const int dx[4] = { -1, 0, 1, 0 };
const int dy[4] = { 0, 1, 0, -1 };

class String {
public:
	String() : liberty(0), size(0) {}
	String(String* s, String* sBoard[11][11]) : liberty(s->liberty), size(s->size) {
		for (int i = 0; i < s->size; ++i) {
			member[i][0] = s->member[i][0];
			member[i][1] = s->member[i][1];
			sBoard[member[i][0]][member[i][1]] = this;
		}
	}
	~String() {}
	
	void push_back(int x, int y) {
		member[size][0] = x;
		member[size][1] = y;
		++size;
	}

	void add(int Board[11][11], String* sBoard[11][11], int x, int y) {
		int i, j, x2, y2, x3, y3;
		bool self = false;
		push_back(x, y);
		liberty += 4;
		for (i = 0; i < 4; ++i) {
			x2 = x + dx[i], y2 = y + dy[i];
			if (Board[x2][y2] == Board[x][y]) self = true;
			if (Board[x2][y2]) --liberty;
			else {
				for (j = 0; j < 4; ++j) {
					x3 = x2 + dx[j], y3 = y2 + dy[j];
					if (sBoard[x3][y3] == this) {
						--liberty;
						break;
					}
				}
			}
		}
		if (self) --liberty;
	}

	void merge(String* s, String* sBoard[11][11], String* strings[200]) {
		for (int i = 0; i < (int)s->size; ++i) {
			push_back(s->member[i][0], s->member[i][1]);
			sBoard[s->member[i][0]][s->member[i][1]] = this;
		}
		strings[s->location] = NULL;
		delete s;
	}
	
	void count_liberty(int Board[11][11]) {
		bool iscount[11][11];
		int i, j, x, y;
		for (i = 1; i < 10; ++i) {
			for (j = 1; j < 10; ++j) iscount[i][j] = false;
		}
		liberty = 0;
		for (i = 0; i < size; ++i) {
			for (j = 0; j < 4; ++j) {
				x = member[i][0] + dx[j], y = member[i][1] + dy[j];
				if (!Board[x][y] && !iscount[x][y]) {
					iscount[x][y] = true;
					++liberty;
				}
			}
		}
	}

	void print() {
		cerr << size << " " << liberty;
		for (int i = 0; i < size; ++i) {
			cerr << " (" << member[i][0] << "," << member[i][1] << ")";
		}
		cerr << endl;
	}

	void remove(int Board[11][11], String* sBoard[11][11], String* strings[200]) {
		String* s[4];
		int num;
		bool exist;
		int i, j, k, x, y;
		for (i = 0; i < size; ++i) {
			Board[member[i][0]][member[i][1]] = 0;
			sBoard[member[i][0]][member[i][1]] = NULL;
			num = 0;
			for (j = 0; j < 4; ++j) {
				x = member[i][0] + dx[j], y = member[i][1] + dy[j];
				if (Board[x][y] == 0 || Board[x][y] == 3 || sBoard[x][y] == this) continue;
				exist = false;
				for (k = 0; k < num; ++k) {
					if (sBoard[x][y] == s[k]) {
						exist = true;
						break;
					}
				}
				if (!exist) {
					s[num] = sBoard[x][y];
					++(s[num]->liberty);
					++num;
				}
			}
		}
		strings[location] = NULL;
		delete this;
	}

	int member[81][2];
	int size;
	int liberty;
	int location;
};

#endif