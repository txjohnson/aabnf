//
//  main.cpp
//  aabnf
//
//  Copyright © 2016 Theo Johnson. All rights reserved.
//

#include <iostream>
#include <string>
#include <fstream>

#include "grammar.hpp"
#include "parser.hpp"
#include "genparser.hpp"

using namespace std;

string      spacename;
string      classname;

const char* outname   = 0;
size_t      nextid = 0;
ofstream    hout;
ofstream    fout;

using buffer = std::pair<uint8_t*, uint8_t*>;

buffer read_stream_to_buf (istream& in) {
	streamsize beg = in.tellg();
	in .seekg (0, ios::end);
	streamsize end = in.tellg();
	streamsize len = end - beg;
	in .seekg (beg, ios::beg);
		
	uint8_t* buf = new uint8_t [len+1];
	in .read ((char*)buf, len);
	buf[len] = '\0';
	
	return buffer (buf, buf + len);
}

void usage () {
	cout << "AABNF Parser Generator (c) 2016\n";
	cout << "usage: aabnf input -ns namespace -cl classname -o outputfileprefix\n";
	cout << "where: input is the grammar file\n";
	cout << "       -ns specifies the namespace in which to place abnf's output\n";
	cout << "           the default is jig\n";
	cout << "       -cl specifies the class to give the parser abnf builds\n";
	cout << "           the default is jigparser\n";
	cout << "       -o  specifies the output file abnf creates.\n";
	cout << "           the default is 'output'\n";
	cout << "           the h and hpp suffixes are added by bnf\n";
}

int main(int argc, const char * argv[]) {
	if (argc < 2) { usage(); return 0; }
	spacename = "abnf";
	classname = "abnfparser";
	outname = "output";
/*
	int i = 2;
	while (i < argc) {
		if (strcmp (argv[i], "-ns") == 0) {
			if (argc <= (i+1) || *argv[i+1] == '-') goto error;
			spacename = argv[i+1]; i+= 2;
		}
		else if (strcmp (argv[i], "-cl") == 0) { 
			if (argc <= (i+1) || *argv[i+1] == '-') goto error;
			classname = argv[i+1]; i+= 2;
		}
		else if (strcmp (argv[i], "-o") == 0) {
			if (argc <= (i+1) || *argv[i+1] == '-') goto error;
			outname = argv[i+1]; i +=2;
		}
		continue;
	error:
		cout << "Invalid syntax near " << argv[i] << ". Ignoring parameter.\n";
		++i;
	}
*/	
	ifstream in (argv[1]);
	if (in.fail()) { cout << "Unable to open file " << argv[1] << endl; return 1; }
	
	auto b = read_stream_to_buf (in);
	in .close ();
	
	auto g = aa::parse (b.first, b.second);
	if (g != nullptr) {
		g->dump (cout);
		cout << "\n\n\n";
		g->print (cout);
		g->transform ();
		cout << "\n\n\n";
		
		g->print (cout);
		
		cout << "\n\n\n";
		auto rv = g->make_rules_view ();
		auto nv = g->make_names_view ();
		for (auto& i : rv) {
			cout << i << "\n";
		}
		
		cout << "\n\n\n";
//		aa::lr::generate_from (rv);
		
		if (aa::lr::parse_using (rv, nv, argv[2])) {
			cout << "Successfully parsed file.\n";
		}
		else {
			cout << "Could not parse file.\n";
		}
/*
		hout .open ((string (outname) + ".h").c_str());
		fout .open ((string (outname) + ".cpp").c_str());
		
		hout.close();
		fout.close();
*/
	}
	
	delete[] b.first;
	return 0;
}
