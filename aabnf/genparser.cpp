//
//  genparser.cpp
//  aabnf
//
//  Created by Theo Johnson on 6/20/16.
//  Copyright © 2016 Theo Johnson. All rights reserved.
//

#include <vector>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <deque>
#include <array>
#include <fstream>
#include <exception>

#include "genparser.hpp"

using namespace std;

namespace aa {

	using strings = std::vector<std::string>;
	using idmap   = std::map <std::string, uint32_t>;
	
/*/// ================================================================================================================================
	Productions is a second low level AST. Very simple. It's basically runs of strings and closest to plain old BNF
/*/// --------------------------------------------------------------------------------------------------------------------------------
	struct prod {
		std::string lhs;
		strings     rhs;
		uint32_t    id = 0;
		
		prod () { }
		
		prod (const rview& rv) : lhs ("V") {
			lhs .append (rv.lhs);
			auto sq = rv.rhs->as (aSeq);
			if (sq != nullptr) {
				for (auto i : sq->parts) {
					add (i);
				}
			}
			else {
				add (rv.rhs);
			}
		}
		
		explicit prod (const std::string alhs) : lhs ("V") { lhs .append (alhs); }
		
		void add (uint8_t ch) {
			std::string v = "T"; v.push_back (ch); rhs.emplace_back (v);
		}
		
		void add (term* at) {
			{	auto t = at->as (aSymbol);
				if (t != nullptr) {
					std::string var = "V"; var .append (t->text); rhs .emplace_back (var); return;
				} }

			{	auto t = at->as (aLiteral);
				if (t != nullptr) {
					for (auto i : t->text) { add (i); }
					return;
				} }

			{	auto t = at->as (aChoose);
				if (t != nullptr) {
					std::stringstream ss;
					ss << "C" << setfill('0') << setw(4) << hex;
					for (auto c : t->chars) { ss << (int)c; }
					rhs .emplace_back (ss.str());
					return;
				} }
			
			{	auto t = at->as (aRange);
				if (t != nullptr) {
					std::stringstream ss;
					ss << "R" << setfill('0') << setw(4) << hex << t->min << t->max;
					rhs .emplace_back (ss.str());
					return;
				} }
		}
	};
	
	// For sorting and searching
	struct prcomp {
		bool operator()(const prod& p, const std::string& s) { return p.lhs < s; }
		bool operator()(const std::string& s, const prod& p) { return s < p.lhs; }
	};

	using prods = std::vector<prod>;
	
	std::ostream& operator<< (std::ostream& out, prods& ps) {
		for (auto& i : ps) {
			out << i.lhs << "\t\t\t\t->";
			for (auto& j : i.rhs) {
				out << " ";
				if (j[0] == 'V') { std::string v (j.begin()+1, j.end()); out << v; }
				else
				if (j[0] == 'T') { std::string v (j.begin()+1, j.end()); out << "\'" << v << "\'"; }
				else
				if (j[0] == 'C') {
					out << "[";
					auto num = j.size() / 4;
					for (auto i = 0; i != num; ++i) {
						if (i) out << ",";
						out << j[i*4] << j[i*4+1] << j[i*4+2] << j[i*4+3];
					}
					out << "]";
				}
				else
				if (j[0] == 'R') {
					out << "[";
					out << j[1] << j[2] << j[3] << j[4] << "-" << j[5] << j[6] << j[7] << j[8];
					out << "]";
				}
				else
				if (j[0] == 'E') { out << "EOS"; }
			}
			out << "\n";
		}
		return out;
	}
	
	
	bool operator== (const prod& a, const prod& b) {
		return a.lhs == b.lhs && a.rhs == b.rhs;
	}
	
	bool operator< (const prod& a, const prod& b) {
		if (a.lhs < b.lhs) return true;
		if (a.lhs == b.lhs) { return a.rhs < b.rhs; }
		return false;
	}
	

	using symbols = std::set <std::string>;
	using mapsets   = std::map <std::string, symbols>;

	bool operator== (const symbols& a, const symbols& b) {
		if (a.size() != b.size()) return false;
		auto bi = b.begin();
		for (auto& ae : a) {
			if (ae != *bi) return false;
		}
		return true;
	}

	bool operator< (const symbols& a, const symbols& b) {
		auto bi = b.begin();
		for (auto& ae : a) {
			if (bi == b.end()) return false;
			if (ae < *bi) return true;
			if (ae > *bi) return false;
			
		}
		if (bi == b.end()) return false;
		return true;
	}
	
	std::ostream& operator<< (std::ostream& out, const symbols& s) {
		out << "{";
		bool sep = false;
		for (auto& j : s) {
			if (sep) out << ", "; else sep = true;
			out << j;
		}
		out << "}";
		return out;
	}

	std::ostream& operator<< (std::ostream& out, const mapsets& s) {
		for (auto& i : s) {
			out << i.first;
			out << " " << i.second << "\n";
		}
		return out;
	}
	
	bool isterminal (const std::string& s) {
		return s[0] != 'V' && s[0] != 'E';
	}

/*/// ================================================================================================================================
	ABNF predefines some symbols.
/*/// --------------------------------------------------------------------------------------------------------------------------------
	symbols valpha  = { "R0061007A", "R0041005A" };
	symbols vbits   = { "C00300031" };
	symbols vcrlf   = { "C000D000A" };
	symbols vdigit  = { "R00300039" };
	symbols vdquote = { "C0022" };
	symbols vhexdig = { "R00300039", "R00410046", "R00610066" };
	symbols vvchar  = { "R0021007E" };
	symbols vwsp    = { "C00200009" };

/*/// ================================================================================================================================
	Buildling a parser according to Compiler Principles Techniques and Tools, Aho etal
	
	Not all that efficient, but it doesn't need to be.
/*/// --------------------------------------------------------------------------------------------------------------------------------
	mapsets calculate_first_sets (prods& is) {
		mapsets ms;
		
		// initialize with builtins
		ms["VALPHA"]  = (valpha);
		ms["VBITS"]   = (vbits);
		ms["VCRLF"]   = (vcrlf);
		ms["VDIGIT"]  = (vdigit);
		ms["VDQUOTE"] = (vdquote);
		ms["VHEXDIG"] = (vhexdig);
		ms["VVCHAR"]  = (vvchar);
		ms["VWSP"]    = (vwsp);
		
		// pass one, scan for terminals
		for (auto& i : is) {
			for (auto& j : i.rhs) {
				if (isterminal (j)) {
					ms[j] .insert (j);
				}
			}
		}
	
		// pass two, work on variables
		bool loop = true;
		int pass = 0;
		while (loop) {
			loop = false;
			++pass;
			for (auto& i : is) {
				auto& in  = ms[i.lhs];
				auto& out = ms[i.rhs[0]];
				auto sz = in.size();
				in .insert (out .begin(), out .end());
				if (sz != in.size()) loop = true;
			}
		}
		
		cout << "** done in " << pass << " passes.\n";
		return ms;
	}
	
	mapsets calculate_follow_sets (prods& is, mapsets& fs) {
		mapsets ms;
		
		bool loop = true;
		while (loop) {
			loop = false;
			// first pass - compute follow(B) from first(C)
			for (auto& i : is) {
				auto& rhs = i.rhs;
				auto  steps = rhs.size()-1;
				
				for (size_t j = 0; j != steps; ++j) {
					auto& B = rhs[j];
					auto& C = rhs[j+1];
					
					auto& first =  fs[C];
					auto& follow = ms[B];
					auto sz = follow.size();
					follow .insert (first.begin(), first.end());
					if (sz != follow.size()) loop = true;
				}
			}
			
			// second pass - compute follow of last element in sequences or lone items B as follow(B) <- follow(A)
			for (auto& i : is) {
				auto& A = ms [i.lhs];
				auto& B = ms [i.rhs.back()];
				
				auto sz = B.size();
				B.insert (A.begin(), A.end());
				if (sz != B.size()) loop = true;
			}
		}
		return ms;
	}

	struct itemset;
	
	struct item {
		prod*    src;
		size_t   dot;
		symbols* la;
		uint32_t go;
		bool     root;
	};

/* these relators will produce LR(1) item sets where similar sets are merged. Need to work on lookahead conflict resolution though
	inline bool operator== (const item& a, const item& b) {
		return (a.src == b.src && a.dot == b.dot);
	}
	
	inline bool operator!= (const item& a, const item& b) {
		return (a.src != b.src || a.dot != b.dot);
	}
	
	inline bool operator<  (const item& a, const item& b) {
		return a.src < b.src || (a.src == b.src && a.dot < b.dot);
	}
*/

	// LR(1) item sets
	inline bool operator== (const item& a, const item& b) {
		if (a.src == b.src && a.dot == b.dot) {
			if (a.la == b.la) return true;
			if (a.la != nullptr && b.la != nullptr && *a.la == *b.la) return true;
		}
		return false;
	}
	
	inline bool operator!= (const item& a, const item& b) {
		return !(a == b);
	}
	
	inline bool operator<  (const item& a, const item& b) {
		if (a.src < b.src) return true;
		if (a.src == b.src) {
			if (a.dot < b.dot) return true;
			if (a.dot == b.dot) {
				if (a.la == b.la) return false;
				if (a.la != nullptr && b.la != nullptr) { return *a.la < *b.la; }
			}
		}
		return false;
	}

	using items = std::vector <item>;
	
	struct itemset {
		items   items;
		size_t  id;
		
		itemset () : id (0) { }
		explicit itemset (size_t anid) : id (anid) { }
		
		bool add (const item& i) {
			auto f = std::find (items.begin(), items.end(), i);
			if (f == items.end()) {
				items .push_back (i);
				std::sort (items.begin(), items.end());
				return true;
			}
			return false;
		}
		
		inline item& front()                     { return items.front(); }
		inline item& back()                      { return items.back(); }
		inline items::iterator begin()           { return items.begin(); }
		inline items::iterator end()             { return items.end(); }
		inline bool empty() const                { return items.empty(); }
		inline size_t size() const               { return items.size(); }
		inline item& operator[] (size_t i)       { return items[i]; }
		inline const item& operator[] (size_t i) const { return items[i]; }
	};
	
	bool operator== (const itemset& a, const itemset& b) {
		if (a.size() != b.size()) return false;
		for (size_t i = 0; i != a.size(); ++i) {
			if (a[i] != b[i]) return false;
		}
		return true;
	}
	
	using itemlist = std::vector <itemset>;

	symbols emptysyms;
	
	void print_word (std::ostream& out, const std::string& w) {
		if (w[0] == 'V') { std::string v (w.begin()+1, w.end()); out << v; }
		else
		if (w[0] == 'T') { std::string v (w.begin()+1, w.end()); out << "\'" << v << "\'"; }
		else
		if (w[0] == 'C') {
			out << "[";
			auto num = w.size() / 4;
			for (auto i = 0; i != num; ++i) {
				if (i) out << ",";
				out << w[i*4] << w[i*4+1] << w[i*4+2] << w[i*4+3];
			}
			out << "]";
		}
		else
		if (w[0] == 'R') {
			out << "[";
			out << w[1] << w[2] << w[3] << w[4] << "-" << w[5] << w[6] << w[7] << w[8];
			out << "]";
		}
		else
		if (w[0] == 'E') { out << "EOS"; }
		else {
			out << w;
		}
	}
	std::ostream& operator<< (std::ostream& out, const item& i) {
		out << i.src->lhs << "\t\t\t\t->";
		int dot = 0;
		for (auto& j : i.src->rhs) {
			if (dot == i.dot) out << " .";
			out << " ";
			print_word (out, j);
			++dot;
		}
		if (i.dot == i.src->rhs.size()) { out << " . "; }
		out << ", " << ((i.la == nullptr) ? emptysyms : *i.la);
		out << "  (goto " << i.go << ")";
		return out;
	}

	std::ostream& operator<< (std::ostream& out, itemset& is) {
		for (auto& i : is) {
			out << i << "\n";
		}
		return out;
	}

	std::ostream& operator<< (std::ostream& out, itemlist& il) {
		size_t index = 0;
		for (auto& i : il) {
			out << "Item" << dec << index << "\n";
			out << i << "\n\n";
			++index;
		}
		return out;
	}

	bool compute_one_closure (itemset& is, mapsets& first, prods& ps) {
		if (is.empty()) return false;
		
		item& i = is.front();
		if (i.dot == i.src->rhs.size()) { return false; }
		
		std::deque <item> work;
		work.push_back (i);

		
		while (!work.empty()) {
			auto it = work.front ();
			work.pop_front ();

			bool inherit = (it.src->rhs.size() - it.dot) == 1;
			
			std::string& v = it.src->rhs[it.dot];
			if (!isterminal(v)) {
				auto j = std::equal_range (ps.begin(), ps.end(), v, prcomp());

				for (auto k = j.first; k != j.second; ++k) {
					symbols* sl = nullptr;
					
					prod* pd = &(*k);
					if (inherit)  { sl = it.la; }
					else          { sl = &first[k->lhs]; }
					
//					if (k->rhs.size() > 1) { sl = &first [k->rhs[1]]; }
//					else                   { sl = it.la; }
					
					auto newi = item { pd, 0, sl, 0, false };

					if (is.add (newi)) { work .push_back (newi); }
				}
			}
		}
		cout << is << "\n\n\n";
		return true;
	}
	
	struct workitem {
		item it;    // next item to be tried
		item* back; // back ref to item that begat this one
		size_t id;  // the goto id we want to send to the back ref
	};

	void compute_all_closures (item& init, itemlist& its, mapsets& first, prods& ps) {
		std::deque <size_t> work;
		std::set <item> completed;
		
		work .push_back (its.size());
		its .emplace_back (itemset(its.size()));
		itemset& is = its .back();
		is .add (init);
		
		while (!work.empty()) {
			auto index = work .front();
			work .pop_front();

			auto& is = its[index];
			
			if (compute_one_closure (is, first, ps)) {
				for (auto& r : is) {
					auto next = item {r.src, r.dot + 1, r.la, 0, true }; // go index will be properly set later
					uint32_t newi = its.size(); // new index

					if (completed.find (next) == completed.end()) {
						completed .insert (next);
						work .push_back (newi);
						its .emplace_back (itemset(newi));
						itemset& is = its .back();
						is .add (next);
					}

					auto f = completed.find (r);
					if (f != completed.end()) {
						if (f->go == 0) {
							auto update = *f;
							update.go = newi;
							completed .erase (f);
							completed .insert (update);
						}
						else {
							r.go = f->go;
						}
					}
					else {
						r.go  = newi;
						completed .insert (r);
					}
				}
			}
		}
		
		cout << "\n\n--- cleanup phase\n";
		// final pass. use the completed list to do final fix up
		for (auto& is : its) {
			for (auto& it : is) {
				if (it.go == 0 && it.dot < it.src->rhs.size()) {
					cout << "searching for" << it << "\n";
					auto f = completed.find (it);
					if (f != completed.end()) {
						cout << "found with goto of " << f->go << "\n";
						it.go = f->go;
					}
					else {
						cout << "*** problem: item not found ***\n";
					}
				}
			}
		}
	}

	symbols s_end = { "E\xff" };
	
	itemlist create_closures (prods& ps, mapsets& first) {
		ps .emplace_back (prod ("~S~"));
		ps .back().rhs.push_back ("Vstart");
		
		itemlist its;
		its .emplace_back (itemset()); // zero position is taken
		
		auto it = item { &ps.back(), 0, &s_end, 0, true };

		compute_all_closures (it, its, first, ps);
		
		return its;
	}
	
	struct action {
		uint32_t target:28; // for shift = new state, for reduce = production, for multi, list where ops reside
		uint32_t op:4;  // 0 = error, 1 = shift, 2 = reduce, 3 = go, 4 = multi, 5 = accept
	};
	
	std::ostream& operator<< (std::ostream& out, const action& a) {
		switch (a.op) {
			case 0: cout << "Err"; break;
			case 1: cout << "Shf"; break;
			case 2: cout << "Red"; break;
			case 3: cout << "Go "; break;
			case 4: cout << "Con"; break;
			case 5: cout << "Acc"; break;
		}
		
		cout << "(" << a.target << ")";
		return out;
	}
	
	using actionrow = std::vector <action>;
	using conflictlist = std::vector <action>;
	using conflictset  = std::vector <conflictlist>;
	
	using actiontable = std::vector <actionrow>;
	
	std::ostream& operator<< (std::ostream& out, const actionrow& r) {
		bool delim = false;
		for (auto& a : r) {
			if (delim) cout << ", "; else delim = true;
			out << a;
		}
		out << "\n";
		return out;
	}

	std::ostream& operator<< (std::ostream& out, const actiontable& t) {
		for (auto& r : t) { out << r; }
		return out;
	}

	using prodinfo = std::pair<uint32_t, uint32_t>;
	using prodinfos = std::vector <prodinfo>;
	
/*/// ================================================================================================================================
	Action Table
	
	Conflicts are allowed. The driver will spawn new workers (at the expense of memory and time) to work it out.
/*/// --------------------------------------------------------------------------------------------------------------------------------
	struct actionfsm {
		actiontable  actions;   // each row is a state, one row per itemset
		conflictset  conflicts; // overflow area for when a state has more than one action for a transition
		size_t       columns;   // terminals + vars
		prodinfos    pdata;     // number of elemnets to pop and what var to trampolline thru
		prods&       prs;       // size info
		idmap&       vars;      // map name to column
		strings      errinfo;  // the item desired but not found
		
		actionfsm (itemlist& il, prods& ps, idmap& ids) : actions (il.size()), columns (256 + ids.size()), prs (ps), vars (ids) {
		
			pdata.reserve (prs.size());
			for (auto& p : prs) {
				pdata .push_back ({ p.rhs.size(), ids[p.lhs] });
			}
		
			errinfo .resize (il.size());
			
			for (size_t i = 0; i != il.size(); ++i) {
				auto& row = actions[i];
				prepare_row (row);
				
				for (auto& it : il[i]) {
					if (it.root == true) {
						if (it.dot < it.src->rhs.size()) {
							errinfo[i] = it.src->rhs[it.dot];
						}
						else {
							stringstream ss;
							bool sep = false;
							for (auto& i : *it.la) {
								if (sep) ss << "|"; else sep = true;
								print_word (ss, i);
							}
							errinfo[i] = ss.str();
						}
						
						cout << "at " << i << " seek " << errinfo[i] << "\n";
					}
					
					if (it.go != 0) { // somewhere inside a production
						auto& v = it.src->rhs[it.dot];
						if (isterminal (v)) {
							if (v[0] == 'R')  { setrange (row, v, 1, it.go); }
							else
							if (v[0] == 'C')  { setchoices (row, v, 1, it.go); }
							else              { setitem (row, v[1], 1, it.go); }
						}
						else {
							setitem (row, vars[v], 3, it.go);
						}
					}
					else { // end of a production
						if (it.src->lhs != "V~S~") {
							for (auto& f : *it.la) {
								if (f[0] == 'R') { setrange (row, f, 2, (uint32_t)it.src->id); }
								else
								if (f[0] == 'C') { setchoices (row, f, 2, (uint32_t)it.src->id); }
								else
								if (f[0] == 'E') { row[255].op = 2; row[255].target = (uint32_t)it.src->id; }
								else             { setitem (row, f[1], 2, (uint32_t)it.src->id); }
							}
						}
						else {
							row [255].op = 5; row[255].target = 0;
						}

					}
				}
			}
		}
		
		void setitem (actionrow& r, size_t i, short op, uint32_t target) {
			if (r[i].op == 0) {
				r[i].op = op;
				r[i].target = target;
			}
			else
			if (r[i].op == 4) {
				conflicts[r[i].target].push_back (action { target, (uint32_t) op });
			}
			else {
				uint32_t target = (uint32_t) conflicts.size();
				conflicts.emplace_back (conflictlist());
				conflicts.back().push_back (r[i]);
				r[i].op = 4;
				r[i].target = target;
			}
		}
		
		void setrange (actionrow& r, const std::string& v, short op, uint32_t target) {
			size_t min = (v[1]-'0')*0x1000 + (v[2]-'0')*0x0100 + (v[3]-'0')*0x0010 + (v[4]-'0');
			size_t max = (v[5]-'0')*0x1000 + (v[6]-'0')*0x0100 + (v[7]-'0')*0x0010 + (v[8]-'0');
			
			for (auto i = min; i <= max; ++i) {
				setitem (r, i, op, target);
			}
		}

		void setchoices (actionrow& r, const std::string& v, short op, uint32_t target) {
			for (int i = 0; i != v.size()/4; ++i) {
				size_t ch = (v[1+i*4]-'0')*0x1000 + (v[2+i*4]-'0')*0x0100 + (v[3+i*4]-'0')*0x0010 + (v[4+i*4]-'0');
				setitem (r, ch, op, target);
			}
		}

		void prepare_row (actionrow& r) {
			// memset (&r, sizeof (actionrow), 0);
			r .resize (columns);
		}
	};
	
	using reverseidmap = std::map <size_t, std::string>;
	
	std::ostream& operator<< (std::ostream& out, const actionfsm& fsm) {
		bool delim = false;
		
		for (int i = 0; i != 256; ++i) {
			if (delim) out << ", "; else delim = true;
			out << (char) i;
		}
		
		reverseidmap rvars;
		for (auto& i : fsm.vars) {
			rvars [i.second] = i.first;
		}
		
		for (auto& p : rvars) {
			out << ", " << p.second;
		}
		
		out << "\n";
		out << fsm.actions;
		return out;
	}
	
	using lrstate = std::vector <size_t>;
	using lrworkqueue = std::deque <lrstate*>;

/*/// ================================================================================================================================
	A Test Driver
	This driver is push driven. Keep giving it tokens until it says no mas. If accepting is set, then we succeeded.
	
	If you run out of characters and it's still expecting, then send 0xff. It will eventually signal no mas.
	
/*/// --------------------------------------------------------------------------------------------------------------------------------
	using errinfo_fn = std::function <void(const std::string&)>;
	
	void donothing (const std::string&) { }
	
	struct lrparser {
		actionfsm&  afsm;
		lrworkqueue states;
		uint8_t     la;
		lrstate*    accepting = nullptr;
		
		errinfo_fn  report_error;
		
		lrparser (actionfsm& af) : afsm (af), report_error (donothing) {
			auto st = new lrstate ();
			st ->push_back (1);
			states .push_back (st);
		}
		
		~lrparser () { }
	
		inline bool accepted ()  { return accepting != nullptr; }
		inline bool pending ()   { return !states.empty(); }
		inline size_t threads () { return states.size(); }
		
		inline void on_error_do (errinfo_fn efn) { report_error = efn; }
		
		
		bool step (uint8_t ch) {
			la = ch;
			if (accepting != nullptr) return false;
			if (states.empty()) return false;

			states .push_back (nullptr); // sentry
			action act;
			
			while (states.front() != nullptr) {
				auto ls = states.front();
				states .pop_front();

				another_pass:
				act = afsm.actions [ls ->back()][la];
				switch (act.op) {
				case 0:  for (auto i : *ls) {
								stringstream out;
								out << "expecting a ";
								print_word (out, afsm.errinfo [i]);
								report_error (out.str());
							}
							delete ls;
							break;
				
				case 1: 	ls ->push_back (act.target);
							states .push_back (ls);
							break;
				
				case 2: 	for (int i = 0; i != afsm.pdata [act.target].first; ++i) { // remove |RHS| states
								ls->pop_back();
						  	}
						  	{
						  		auto next = afsm.actions [ls ->back()][afsm.pdata[act.target].second]; // column of var
								ls ->push_back (next.target);
							}
							goto another_pass;

				case 3: 	ls ->pop_back();
						  	ls->push_back (act.target);
							states .push_back (ls);
							break;
				
				case 4: 	{ // conflict... spawn multiple parsers
								auto& conflicts = afsm.conflicts [act.target];
								for (auto a : conflicts) {
									auto nlr = new lrstate (ls->begin(), ls->end());
									spawn_step (nlr, a);
								}
								delete ls;
							}
 						   break;
				
				case 5: 	if (accepting == nullptr) { accepting = ls; }
				        	break;
				}
			}
			states .pop_front();
			return true;
		}
		
		// identical behavior to above. Our only nod to efficiency is to take conflict resolution out of the
		// happy path. (It's logic is identical to above, so now there's two places to make changes. Enjoy!)
		void spawn_step (lrstate* ls, action act) {
			bool nextime = false;
			another_pass:
			if (nextime) act = afsm.actions [ls ->back()][la];
			else nextime = true;
			
			switch (act.op) {
			case 0:	for (auto i : *ls) {
							stringstream out;
							out << "expecting a ";
							print_word (out, afsm.errinfo [i]);
							report_error (out.str());
						}
						delete ls;
						break;
			
			case 1: 	ls ->push_back (act.target);
						states .push_back (ls);
						break;
			
			case 2: 	for (int i = 0; i != afsm.pdata [act.target].first; ++i) { // remove |RHS| states
							ls->pop_back();
						}
						{
							auto next = afsm.actions [ls ->back()][afsm.pdata[act.target].second]; // column of var
							ls ->push_back (next.target);
						}
						goto another_pass;

			case 3: 	ls ->pop_back();
						ls->push_back (act.target);
						states .push_back (ls);
						break;
			
			case 4: 	{ // conflict... spawn multiple parsers
							auto& conflicts = afsm.conflicts [act.target];
							for (auto a : conflicts) {
								auto nlr = new lrstate (ls->begin(), ls->end());
								spawn_step (nlr, a);
							}
							delete ls;
						}
						break;
					
			case 5: 	if (accepting == nullptr) { accepting = ls; }
						break;
			}
		}
		
		
	};
	
	
/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	namespace lr {

		void generate_from (rulesview& rv) {
			prods ps;
			uint32_t id = 256;
			for (auto& i : rv) {
				ps .emplace_back (prod (i));
				ps.back().id = id;
				++id;
			}
			
			cout << ps;
			cout << "\n\n\n";
			
			auto first = calculate_first_sets (ps);
			cout << "Firsts\n";
			cout << first;
			cout << "\n\n\n";
			
			auto follow = calculate_follow_sets (ps, first);
			cout << "Follows\n";
			cout << follow;
			cout << "\n\n\n";

			auto items  = create_closures (ps, first);
			cout << items;
		}

		bool parse_using (rulesview& rv, namesview& nv,  const char* filename) {
			prods ps;
			idmap ids;
			
			{  uint32_t id = 0;
				for (auto& i : rv) {
					ps .emplace_back (prod (i));
					ps.back().id = id;
					++id;
				}
			}
			
			{
				ids["V~S~"] = 256;
				uint32_t id = 257;
				for (auto& i : nv) { std::string s = "V"; s.append (i); ids[s] = id; ++id; }
			}
			
			cout << ps;
			cout << "\n\n\n";
			
			auto first = calculate_first_sets (ps);
			cout << "Firsts\n";
			cout << first;
			cout << "\n\n\n";
			
			auto follow = calculate_follow_sets (ps, first);
			cout << "Follows\n";
			cout << follow;
			cout << "\n\n\n";

			auto items  = create_closures (ps, first);
			cout << items;
			
			auto afsm = std::unique_ptr<actionfsm> (new actionfsm (items, ps, ids));
			auto parser = lrparser (*afsm);
			strings errs;
			size_t line = 1;
			
			parser.on_error_do ([&](const std::string& s)->void {
				if (parser.threads() <= 2) {
					string report ("Error at line ");
					report .append (std::to_string (line));
					report .append (": ");
					report .append (s);
					errs .push_back (report);
				}
			});
			
			cout << "\n\n\n";
			cout << *afsm;
			
			cout << "\n\n\n";
			std::ifstream in (filename);
			if (!in.fail()) {
				uint8_t ch = in.eof() ? '\xff' : in.get();
				if (ch == '\n') ++line;
				
				while (parser.step (ch)) {
					ch = in.eof() ? '\xff' : in.get();
				}
				
				if (!parser.accepted()) {
					for (auto& i : errs) { cout << i << "\n"; }
					return false;
				}
				else return true;
			}
			else {
				cout << "Unable to open file " << filename << "\n";
			}
			return false;
		}
	}

}