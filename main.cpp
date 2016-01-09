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
			if (board[row][col] != -1) {
				clauses.push_back(threesome_to_var(row, col, board[row][col]));
				clauses.push_back(0);
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

		//for (int i = 0; i < 9; i++) {
		//	int square_x = (i / 3) * 3;
		//	int square_y = (i % 3) * 3;s

		//	for (int j = 0; j < 9; j++) {
		//		clauses.push_back(threesome_to_var(square_x + j / 3, square_y + (j % 3), digit));
		//	}
		//	clauses.push_back(0);
		//}
	}
}

void output_clauses_file(const vector<int>& clauses, string filename)
{
	std::ofstream f(filename);
	const int variable_count = 9 * 9 * 9;
	int clause_count = std::count(clauses.begin(), clauses.end(), 0);

	f << "p cnf " << variable_count << ' ' << clause_count << endl;

	for (int p : clauses) {
		f << p;
		if (p == 0) {
			f << endl;
		}
		else {
			f << ' ';
		}
	}
}

void run_sat_solver(const string& clauses_filename, const string& result_filename)
{
	string cmd = "java -jar org.sat4j.core.jar " + clauses_filename + " > " + result_filename;
	system(cmd.c_str());
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
		if (input == "v") {
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

void solve(const string& filename)
{
	read_board(filename);
	print_board();
	cout << endl;
	
	vector<int> clauses;
	add_def_clauses(clauses);
	add_uniq_clauses(clauses);
	add_validity_clauses(clauses);

	output_clauses_file(clauses, filename + ".cnf");
	run_sat_solver(filename + ".cnf", filename + ".sat_result.txt");
	read_sat_result_file(filename + ".sat_result.txt");
	print_board();
}

int main()
{
	solve("board.txt");
	std::getchar();
	return 0;
}

#ifdef SUGGESTED_DEFINITIONS

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <windows.h>

#define VAR_NO 729      // Variable number
#define CLS_NO 11745    // Clauses number

using namespace std;
using std::vector;
using std::string;

// CLASSES
class threesome
{
private:
	int x;
	int y;
	int d;

public:
	threesome();
	threesome(int a, int b, int c);
	threesome(const threesome &th);
	void tset(int a, int b, int c) { x = a; y = b; d = c; }
	bool comp(int a, int b, int c) { if (x == a && y == b && d == c) return true; else return false; }    // compare method
	void print();
	int getx() { return x; }
	int gety() { return y; }
	int getd() { return d; }
};

class clause
{
private:
	vector<int> arr;

public:
	int size;

	clause() { size = 0; }
	clause(int x) :arr(x) { size = x; }

	void cset(int ind, int val);
	int cget(int ind) { return arr[ind]; }
	void ccopy(clause cls);
	void print(vector<int> *pvec);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// GLOBAL FUNCTIONS:
void init_board(string board_filename);     // Sudoku board initialization
void print_board();                         // Print a Sudoku board

int threesome_to_var(int x, int y, int d);   // Conversion of threesome to variable number
threesome var_to_threesome(int x);           // Conversion of variable number to threesome

void fill_var_table();                  // Variable table filling
void print_var_table(threesome vt[]);   // Print the variable table

void def_clauses(clause clss[], int *clss_ctr);         // Creation of definition clauses
void uniq_clauses(clause clss[], int *clss_ctr);        // Creation of uniqueness clauses
void valid_clauses(clause clss[], int *clss_ctr);       // Creation of validity clauses

void grid_checker(clause clss[], int *clss_ctr, int x, int y);  // Creating validity clauses for the 9grid areas
void line_checker(clause clss[], int *clss_ctr, int x);         // Creating validity clauses for the lines
void row_checker(clause clss[], int *clss_ctr, int x);          // Creating validity clauses for the rows
int grider(int x, int y, int k, int pls);                       // Auxiliary function for the grid_checker function (returns the position in the 9grid area)

string output_clauses_file(clause clss[], int clss_ctr);    // Creation of output_clauses file (for the SAT solver)
string run_sat_solver(string clauses_filename);             // Running SAT solver program on the output_clauses file
void read_sat_result_file(string filename);                 // Reading the result output file of the SAT solver program and translating it to the Sudoku board

															//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
															//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

															// GLOBAL VARS:
int board[9][9];  // Sudoku board
threesome vt[730]; // Variable table
int vt_ctr;        // Variable counter

				   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
				   //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main()
{
	clause clauses[11750];      // clauses array
	int op;                   // operation number
	int clss_ctr = 0;         // clauses counter
	string board_filename = "board.txt";
	string output_clauses_filename;
	string sat_result_filename;

	vt_ctr = 0;

	init_board(board_filename);
	fill_var_table();

	cout << "Welcome to Sudoku solver, please enter the number of the desired operation:" << endl;

	while (true) {
		cout << endl;
		cout << "1. Create a valid Sudoku board" << endl;
		cout << "2. Print board" << endl;
		cout << "3. Print var table" << endl;
		cout << "4. exit" << endl;
		cin >> op;
		switch (op) {
		case 1:
			def_clauses(clauses, &clss_ctr);
			uniq_clauses(clauses, &clss_ctr);
			valid_clauses(clauses, &clss_ctr);
			output_clauses_filename = output_clauses_file(clauses, clss_ctr);
			sat_result_filename = run_sat_solver(output_clauses_filename);
			read_sat_result_file(sat_result_filename);
			break;
		case 2:
			print_board();
			break;
		case 3:
			print_var_table(vt);
			break;
		case 4:
			return 0;
		default:
			cout << "Unrecognized operation number, please try again" << endl;
		}
	}
	return 0;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void init_board(string board_filename)
{
	fstream boardfile(board_filename.c_str(), std::ios_base::in);      // open boardfile file named board_filename for reading

	for (int i = 0; i<9; i++) {
		for (int j = 0; j<9; j++) {
			boardfile >> board[i][j];
		}
	}
}

void print_board()
{
	for (int i = 0; i<9; i++) {
		if (i == 0 || (i % 3) == 0)
			cout << "-------------------------" << endl;
		for (int j = 0; j<9; j++) {
			if (j == 0 || (j % 3) == 0)
				cout << "| ";
			cout << board[i][j] << " ";
		}
		cout << "|" << endl;
	}
	cout << "-------------------------" << endl;
}

int threesome_to_var(int x, int y, int d)
{
	int i;

	for (i = 1; !vt[i].comp(x, y, d); ++i)
		;
	return i;
}

threesome var_to_threesome(int x)
{
	return vt[x];
}

void fill_var_table()
{
	for (int i = 1; i <= 9; i++) {
		for (int j = 1; j <= 9; j++) {
			for (int k = 1; k <= 9; k++) {
				vt_ctr++;
				vt[vt_ctr].tset(i, j, k);     // Variable number vt_ctr means: (i,j)=k in the sudoku board
			}
		}
	}

}

void print_var_table(threesome vt[])
{
	for (int i = 1; i <= vt_ctr; i++) {
		cout << "x" << i << " | ";
		vt[i].print();
		if ((i % 100) == 0) {    // Every 100 lines wait for input (just to be able to see the output)
			int x;
			cin >> x;
		}
	}
}

void def_clauses(clause clss[], int *clss_ctr)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

void uniq_clauses(clause clss[], int *clss_ctr)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

void valid_clauses(clause clss[], int *clss_ctr)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

void grid_checker(clause clss[], int *clss_ctr, int x, int y)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

void line_checker(clause clss[], int *clss_ctr, int x)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

void row_checker(clause clss[], int *clss_ctr, int x)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

int grider(int x, int y, int k, int pls)
{
	// YOUR CODE HERE  <<<<<<<<<<<<<<<<<<
}

string output_clauses_file(clause clss[], int clss_ctr)
{
	string clauses_filename = "clauses.cnf";
	ofstream clausefile(clauses_filename.c_str());      // open clausefile with clauses_filename for writing

	clausefile << "p cnf " << VAR_NO << " " << CLS_NO << endl;      // first line in the file
	for (int i = 0; i<clss_ctr; i++) {
		vector<int> pvec;
		vector<int>::iterator it;

		clss[i].print(&pvec);       // fill vector pvec with clss[i] values
		clausefile << " ";
		for (it = pvec.begin(); it != pvec.end(); it++) {      // run trough pvec and insert its values into the clausefile
			clausefile << *(it) << " ";
		}
		clausefile << "0" << endl;      // every line ends with 0
	}
	clausefile.close();

	return clauses_filename;

}

string run_sat_solver(string clauses_filename)
{
	string sat_command = "java -jar org.sat4j.core.jar ";     // the command to run the SAT solver
	string redirect = " > ";                                  // output redirection command
	string sat_result_filename = "sat_result.txt";            // target filename for SAT solver result

	sat_command.append(clauses_filename);
	sat_command.append(redirect);
	sat_command.append(sat_result_filename);

	system(sat_command.c_str());        // execute the combined command - SAT solver runs and outputs result to sat_result.txt

	return sat_result_filename;

}

void read_sat_result_file(string sat_result_filename)
{
	fstream parsefile(sat_result_filename.c_str(), std::ios_base::in);      // open parsefile file named sat_result_filename for reading
	string input;
	string satisfiable = "SATISFIABLE";
	string unsatisfiable = "UNSATISFIABLE";
	int data[9];
	threesome th;

	parsefile >> input;     // first input from parsefile
	while (input.compare(satisfiable) != 0 && input.compare(unsatisfiable) != 0) {   // the result file contains unneeded data, this loop runs trough it...
		parsefile >> input;                                                    //...until it reaches SATISFIABLE
	}
	parsefile >> input;     // last input ("v") from parsefile before the actual result

	for (int i = 0; i<81; i++) {
		for (int j = 0; j<9; j++) {      // read the result in 9 sized clauses
			parsefile >> data[j];
		}
		for (int k = 0; k<9; k++) {
			if (data[k]>0) {
				threesome th(var_to_threesome(data[k]));       // positive values in the result converted to (i,j)=k threesome
				board[th.getx() - 1][th.gety() - 1] = th.getd();      // board is filled accordingly to the threesome (i,j)=k
			}
		}
	}

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

threesome::threesome()
{     // c'tor
	x = 0;
	y = 0;
	d = 0;
}

threesome::threesome(int a, int b, int c)
{      // int c'tor
	x = a;
	y = b;
	d = c;
}

threesome::threesome(const threesome &th)
{      // copy c'tor
	x = th.x;
	y = th.y;
	d = th.d;
}

void threesome::print()
{        // printing method
	cout << "((" << x << "," << y << ") == " << d << ")" << endl;
}
//---------------------------------------------------------------------

void clause::cset(int ind, int val)
{        // insert val into arr[ind]
	arr[ind] = val;
}

void clause::ccopy(clause cls)
{         // clause copy method
	int i;

	for (i = 0; i<cls.size; ++i) {
		arr.push_back(cls.cget(i));
	}
	size = i;
}

void clause::print(vector<int> *pvec)
{      // printing the clause to the passed vector
	vector<int>::iterator it_arr;

	for (it_arr = arr.begin(); it_arr != arr.end(); ++it_arr) {
		(*pvec).push_back(*(it_arr));
	}
}

#endif
