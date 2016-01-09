#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using std::string;
using std::vector;
using std::cout;
using std::endl;

int board[9][9];

// Read the board transforming digits from 1..9 to 0..8
// zeroes become -1
void read_board(const string& board_filename)
{
	std::fstream boardfile(board_filename.c_str(), std::ios_base::in);

	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			boardfile >> board[i][j];
			// Digits are now 0..8
			board[i][j]--;
		}
	}
}

void print_board()
{
	const char* row_separator = "x-------x-------x-------x";

	for (int i = 0; i < 9; i++) {
		if (i % 3 == 0)
			cout << row_separator << endl;
		for (int j = 0; j < 9; j++) {
			if (j % 3 == 0)
				cout << "| ";

			if (board[i][j] == -1)
				cout << ' ';
			else
				cout << (board[i][j] + 1);
			cout << ' ';
		}
		cout << "|" << endl;
	}
	cout << row_separator << endl;
}

int threesome_to_var(int row, int col, int digit)
{
	return row * 81 + col * 9 + digit + 1;
}

void set_board_digit(int var)
{
	var--;
	board[var / 81][(var / 9) % 9] = var % 9;
}

void add_def_clauses(vector<int>& clauses)
{
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			int digit = board[row][col];
			if (digit != -1) {
				for (int i = 0; i < 9; i++) {
					if (i == digit) {
						clauses.push_back(threesome_to_var(row, col, digit));
					}
					else {
						clauses.push_back(-threesome_to_var(row, col, i));
					}
					clauses.push_back(0);
				}
			}
		}
	}
}

void add_uniq_clauses(vector<int>& clauses)
{
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			for (int i = 0; i < 8; i++) {
				for (int j = i + 1; j < 9; j++) {
					clauses.push_back(-threesome_to_var(row, col, i));
					clauses.push_back(-threesome_to_var(row, col, j));
					clauses.push_back(0);
				}
			}
		}
	}
}

void add_validity_clauses(vector<int>& clauses)
{
	for (int digit = 0; digit < 9; digit++) {
		for (int row = 0; row < 9; row++) {
			for (int i = 0; i < 9; i++) {
				clauses.push_back(threesome_to_var(row, i, digit));
			}
			clauses.push_back(0);
		}

		for (int col = 0; col < 9; col++) {
			for (int i = 0; i < 9; i++) {
				clauses.push_back(threesome_to_var(i, col, digit));
			}
			clauses.push_back(0);
		}

		for (int square_x = 0; square_x < 9; square_x += 3) {
			for (int square_y = 0; square_y < 9; square_y += 3) {
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						clauses.push_back(threesome_to_var(square_x + i, square_y + j, digit));
					}
				}
				clauses.push_back(0);
			}
		}
	}
}

void output_clauses_file(const vector<int>& clauses, const string& filename)
{
	std::ofstream f(filename.c_str());
	const int variable_count = 9 * 9 * 9;
	int clause_count = std::count(clauses.begin(), clauses.end(), 0);

	f << "p cnf " << variable_count << ' ' << clause_count << endl;

	for (vector<int>::size_type i = 0; i < clauses.size(); i++) {
		f << clauses[i];
		if (clauses[i] == 0) {
			f << endl;
		}
		else {
			f << ' ';
		}
	}
}

void read_sat_result_file(const string& sat_result_filename)
{
	std::fstream f(sat_result_filename.c_str(), std::ios_base::in);

	string input;

	while (f >> input) {
		if (input == "s") {
			string result;
			f >> result;
			std::cout << "The sudoku is " << result << "!" << endl;
		}
		else if (input == "v") {
			int var;
			f >> var;

			while (var != 0) {
				if (var > 0) {
					set_board_digit(var);
				}
				f >> var;
			}
		}
	}
}

void solve(const string& board_file)
{
	read_board(board_file);
	print_board();
	cout << endl;
	
	vector<int> clauses;
	add_def_clauses(clauses);
	add_uniq_clauses(clauses);
	add_validity_clauses(clauses);

	string cnf_file = board_file + ".cnf";
	string sat_result_file = board_file + ".sat_result.txt";

	output_clauses_file(clauses, cnf_file);
	system(("java -jar org.sat4j.core.jar " + cnf_file + " > " + sat_result_file).c_str());
	read_sat_result_file(sat_result_file);

	print_board();
}

int main()
{
	solve("board.txt");
	std::getchar();
	return 0;
}
