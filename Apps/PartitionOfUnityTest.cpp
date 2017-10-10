#include <cmath>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "LRSpline/LRSplineSurface.h"

typedef unsigned int uint;

using namespace LR;
using namespace std;

int main(int argc, char **argv) {

	// set default parameter values
	const double TOL = 1e-6;
	std::vector<double> knot_u;
	std::vector<double> knot_v;
	int p1 = 3;
	int p2 = 4;
	int n1 = 8;
	int n2 = 5;
	int dim = 4;
	bool rat = false;
	string parameters(" parameters: \n" \
	                  "   -p1  <n> polynomial ORDER (degree+1) in first parametric direction\n" \
	                  "   -p2  <n> polynomial order in second parametric direction\n" \
	                  "   -n1  <n> number of basis functions in first parametric direction\n" \
	                  "   -n2  <n> number of basis functions in second parametric direction\n" \
	                  "   -knot1 <n>  space-seperated list of the first knot vector (must specify n1 and p1 first)\n"\
	                  "   -knot2 <n>  space-seperated list of the second knot vector (must specify n2 and p2 first)\n"\
	                  "   -dim <n> dimension of the controlpoints\n" \
	                  "   -help    display (this) help information\n");

	// read input
	for(int i=1; i<argc; i++) {
		if(strcmp(argv[i], "-p1") == 0) {
			p1 = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-p2") == 0) {
			p2 = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-n1") == 0) {
			n1 = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-n2") == 0) {
			n2 = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-knot1") == 0) {
			knot_u.resize(n1+p1);
			for(int j=0; j<n1+p1; j++)
				knot_u[j] = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-knot2") == 0) {
			knot_v.resize(n2+p2);
			for(int j=0; j<n2+p2; j++)
				knot_v[j] = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-dim") == 0) {
			dim = atoi(argv[++i]);
		} else if(strcmp(argv[i], "-help") == 0) {
			cout << "usage: " << argv[0] << endl << parameters.c_str();
			exit(0);
		} else {
			cout << "usage: " << argv[0] << endl << parameters.c_str();
			exit(1);
		}
	}

	// do some error testing on input
	if(n1 < p1) {
		cerr << "ERROR: n1 must be greater or equal to p1\n";
		exit(2);
	} else if(n2 < p2) {
		cerr << "ERROR: n2 must be greater or equal to p2\n";
		exit(2);
	}

	// make a uniform integer knot vector
	std::vector<double> cp((dim + rat)*n1*n2);
	if(knot_u.size() == 0)
		for(int i=0; i<p1+n1; i++)
			knot_u.push_back( (i<p1) ? 0 : (i>n1) ? n1-p1+1 : i-p1+1 );
	if(knot_v.size() == 0)
		for(int i=0; i<p2+n2; i++)
			knot_v.push_back( (i<p2) ? 0 : (i>n2) ? n2-p2+1 : i-p2+1 );

	// create a list of random control points (all between 0.1 and 1.1)
	int k=0;
	for(int j=0; j<n2; j++)
		for(int i=0; i<n1; i++)
			for(int d=0; d<dim+rat; d++)
				cp[k++] = ((i*2+j*3+d*5) % 13) / 13.0 + 0.1;  // rational weights also random and thus we need >0

	// make the LR B-spline surface
	LRSplineSurface lr(n1, n2, p1, p2, knot_u.begin(), knot_v.begin(), cp.begin(), dim, rat);

	// evaluate function values on edges, knots and in between the knots
	vector<bool> assertion_passed;
	vector<double> par_u_values;
	vector<double> par_v_values;
	for(auto el : lr.getAllElements()) {
		for(int i=0; i<3; i++) {
			for(int j=0; j<3; j++) {
				double u = el->umin() + i/2.0* (el->umax()-el->umin());
				double v = el->vmin() + j/2.0* (el->vmax()-el->vmin());

				vector<vector<double> > lr_basis;
				lr.computeBasis(u,v, lr_basis, 2);
				double sum         = 0;
				double sum_diff_u  = 0;
				double sum_diff_v  = 0;
				double sum_diff_uu = 0;
				double sum_diff_uv = 0;
				double sum_diff_vv = 0;
				for(uint k=0; k<lr_basis.size(); k++) {
					sum         += lr_basis[k][0];
					sum_diff_u  += lr_basis[k][1];
					sum_diff_v  += lr_basis[k][2];
					sum_diff_uu += lr_basis[k][3];
					sum_diff_uv += lr_basis[k][4];
					sum_diff_vv += lr_basis[k][5];
				}
				cout << "sum (" << u << ", " << v <<") minus one = " << sum-1.0 << endl;
				cout << "sum diff u  = " << sum_diff_u  << endl;
				cout << "sum diff v  = " << sum_diff_v  << endl;
				cout << "sum diff uu = " << sum_diff_uu << endl;
				cout << "sum diff uv = " << sum_diff_uv << endl;
				cout << "sum diff vv = " << sum_diff_vv << endl << endl;
				bool correct = !(fabs(sum-1.0)     > TOL ||
								 fabs(sum_diff_u)  > TOL ||
								 fabs(sum_diff_v)  > TOL ||
								 fabs(sum_diff_uu) > TOL ||
								 fabs(sum_diff_uv) > TOL ||
								 fabs(sum_diff_vv) > TOL );

				// collect results for summary at the end
				assertion_passed.push_back(correct);
				par_u_values.push_back(u);
				par_v_values.push_back(v);
			}
		}
	}

	// display results
	bool oneFail = false;
	cout << "     =======    RESULT SUMMARY   ========     \n\n";
	cout << "_____u____________v_____________ASSERT__\n";
	for(uint i=0; i<assertion_passed.size(); i++) {
		printf("%10.4g   %10.4g           ", par_u_values[i], par_v_values[i]);
		if(assertion_passed[i])
			cout << "OK\n";
		else
			cout << "FAIL\n";
		if(!assertion_passed[i])
			oneFail = true;
	}
	cout << "----------------------------------------\n";
	if(oneFail)
		cout << "    test FAILED\n";
	else
		cout << "    all assertions passed\n";
	cout << "========================================\n";


}

