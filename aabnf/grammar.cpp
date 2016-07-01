//
//  grammar.cpp
//  aabnf
//
//

#include <deque>

#include "grammar.hpp"

namespace aa {
/*/// ================================================================================================================================
	Prototype objects to make casting and RTTI a little nicer to look at.
/*/// --------------------------------------------------------------------------------------------------------------------------------
	list   aList;
	alt    anAlt;
	seq    aSeq;
	repeat aRepeat;
	option anOption;
	mod    aMod;
	group  aGroup;
	range  aRange;
	choose aChoose;
	number aNumber;
	symbol aSymbol;
	literal aLiteral;
	rule   aRule;
	epsilon anEpsilon;

/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	std::ostream& operator<< (std::ostream& out, rview& rv) {
		out << rv.lhs << "\t\t\t\t= ";
		rv.rhs->print (out, true);
		return out;
	}

/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	term::~term () { }
	
	list::~list () { for (auto i : parts) { if (i != nullptr) delete i; } }
	
	mod ::~mod  () { if (phrase != nullptr) delete phrase; }

	rule::~rule () { if (rhs != nullptr) delete rhs; }


/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	literal:: literal () { }
	literal:: literal (uchar ch) { text .push_back (ch); }
	literal:: literal (uchar* beg, uchar* end) : text (beg, end) { }
	literal:: literal (const std::string& s) : text (s) { }

	number:: number () { }
	number:: number (uint64_t value) : num (value) { }

	list:: list () { }

	mod:: mod () { }
	mod:: mod (term* inner) : phrase (inner) { }

	repeat:: repeat () { }
	repeat:: repeat (unsigned amin, unsigned amax) : min (amin), max (amax) { }
	repeat:: repeat (unsigned amin, unsigned amax, term* phrase) : mod (phrase), min (amin), max (amax) { }

	range:: range () { }
	range:: range (char32_t amin, char32_t amax) : min (amin), max (amax) { }

	choose:: choose () { }

	rule:: rule () { }
	rule:: rule (uchar* beg, uchar* end) : lhs (beg, end) { }
	rule:: rule (const std::string& s) : lhs (s) { }
	rule:: rule (const std::string& s, term* arhs) : lhs (s), rhs (arhs) { }


/*/// ================================================================================================================================
	To check the dynamic type of an AST node
/*/// --------------------------------------------------------------------------------------------------------------------------------
	bool epsilon:: kindof (const term& other) { return dynamic_cast<const epsilon*> (&other) != nullptr; }
	bool literal:: kindof (const term& other) { return dynamic_cast<const literal*> (&other) != nullptr; }
	bool symbol::  kindof (const term& other) { return dynamic_cast<const symbol*> (&other) != nullptr; }
	bool number::  kindof (const term& other) { return dynamic_cast<const number*> (&other) != nullptr; }
	bool range::   kindof (const term& other) { return dynamic_cast<const range*> (&other) != nullptr; }
	bool choose::  kindof (const term& other) { return dynamic_cast<const choose*> (&other) != nullptr; }
	bool list::    kindof (const term& other) { return dynamic_cast<const list*> (&other) != nullptr; }
	bool alt::     kindof (const term& other) { return dynamic_cast<const alt*> (&other) != nullptr; }
	bool seq::     kindof (const term& other) { return dynamic_cast<const seq*> (&other) != nullptr; }
	bool group::   kindof (const term& other) { return dynamic_cast<const group*> (&other) != nullptr; }
	bool eval::    kindof (const term& other) { return dynamic_cast<const eval*> (&other) != nullptr; }
	bool option::  kindof (const term& other) { return dynamic_cast<const option*> (&other) != nullptr; }
	bool repeat::  kindof (const term& other) { return dynamic_cast<const repeat*> (&other) != nullptr; }
	bool mod::     kindof (const term& other) { return dynamic_cast<const mod*> (&other) != nullptr; }
	bool rule::    kindof (const term& other) { return dynamic_cast<const rule*> (&other) != nullptr; }
	


/*/// ================================================================================================================================
	To rewrite parts of the AST
/*/// --------------------------------------------------------------------------------------------------------------------------------
	void epsilon:: find_and_replace (term& type, const std::function<term*(term*)>& with) { }
	void literal:: find_and_replace (term& type, const std::function<term*(term*)>& with) { }
	void symbol::  find_and_replace (term& type, const std::function<term*(term*)>& with) { }
	void number::  find_and_replace (term& type, const std::function<term*(term*)>& with) { }
	void range::   find_and_replace (term& type, const std::function<term*(term*)>& with) { }
	void choose::  find_and_replace (term& type, const std::function<term*(term*)>& with) { }

	void list::    find_and_replace (term& type, const std::function<term*(term*)>& with) {
		for (auto& i : parts) {
			if (i != nullptr && i ->kindof (type)) {
				auto r = with (i);
				if (r != nullptr) i = r;
			}
			else {
				i->find_and_replace (type, with);
			}
		}
	}

	void mod::    find_and_replace (term& type, const std::function<term*(term*)>& with) {
		if (phrase != nullptr && phrase ->kindof (type)) {
			auto r = with (phrase);
			if (r != nullptr) phrase = r;
		}
		else {
			phrase ->find_and_replace (type, with);
		}
	}

	void rule::   find_and_replace (term& type, const std::function<term*(term*)>& with) {
		if (rhs != nullptr && rhs->kindof (type)) {
			auto r = with (rhs);
			if (r != nullptr) { rhs = r; }
		}
		else {
			if (rhs != nullptr) { rhs ->find_and_replace (type, with); }
		}
	}
	
	void rule::   inner_find_and_replace (term& type, const std::function<term*(term*)>& with) {
		if (rhs != nullptr) { rhs ->find_and_replace (type, with); }
	}


/*/// ================================================================================================================================
	printing, of course
/*/// --------------------------------------------------------------------------------------------------------------------------------
	void epsilon:: dump (std::ostream& out) { out << "nil"; }
	void literal:: dump (std::ostream& out) { out << "\"" << text << "\""; }
	void symbol::  dump (std::ostream& out) { out << text; }
	void number::  dump (std::ostream& out) { out << num; }
	void mod::     dump (std::ostream& out) { }
	void option::  dump (std::ostream& out) { out << "(maybe: "; phrase->dump (out); out << ")"; }
	
	void list::    dump (std::ostream& out) {
		out << "(";
		this->dumpfunctor (out); out << ":";
		for (auto& i : parts) { out <<" "; i->dump (out); }
		out << ")";
	}
	
	void repeat::  dump (std::ostream& out) {
		out << "(repeat ";
		if (min == 0 && max == 0) { out << "* : "; }
		else
		if (max == 0)             { out << min << " - * : "; }
		else                      { out << min << " - " << max << ": "; }
		phrase ->dump (out);
		out << ")";
	}
	
	void range::   dump (std::ostream& out) { out << "(within " << min << " - " << max << ")"; }
	void choose::  dump (std::ostream& out) {
		out << "(one of: ";
		for (size_t i = 0; i != chars.size(); ++i) {
			if (i) out << " ";
			out << chars[i];
		}
		out << ")";
	}
	
	void rule::    dump (std::ostream& out) { out << "(def: " << lhs << " "; rhs ->dump (out); out << ")"; }

/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	void list::  dumpfunctor (std::ostream& out) { }
	void alt::   dumpfunctor (std::ostream& out) { out << "alt"; }
	void seq::   dumpfunctor (std::ostream& out) { out << "seq"; }
	void group:: dumpfunctor (std::ostream& out) { }
	void eval::  dumpfunctor (std::ostream& out) { out << "eval"; }

/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	void epsilon:: print (std::ostream& out, bool isroot) { out << "nil"; }
	void literal:: print (std::ostream& out, bool isroot) { out << "\"" << text << "\""; }
	void symbol::  print (std::ostream& out, bool isroot) { out << text; }
	void number::  print (std::ostream& out, bool isroot) { out << num; }
	void mod::     print (std::ostream& out, bool isroot) { }
	void option::  print (std::ostream& out, bool isroot) { out << "["; phrase->print (out); out << "]"; }

	void list::    print (std::ostream& out, bool isroot) {
		if (!isroot) { out << "("; }
		for (size_t i = 0; i != parts.size(); ++i) {
			if (i) out << " ";
			parts[i]->print (out);
		}
		if (!isroot) { out << ")"; }
	}
	
	void alt::     print (std::ostream& out, bool isroot) {
		if (!isroot) { out << "("; }
		for (size_t i = 0; i != parts.size(); ++i) {
			if (i) out << " | ";
			parts[i]->print (out);
		}
		if (!isroot) { out << ")"; }
	}
	
	void eval::    print (std::ostream& out, bool isroot) {
		out << "{";
		for (size_t i = 0; i != parts.size(); ++i) {
			if (i) out << " ";
			parts[i]->print (out);
		}
		out << "}";
	}

	void repeat::  print (std::ostream& out, bool isroot) {
		if (min == 0 && max == 0) { out << "*"; }
		else
		if (min != 0 && max == 0) { out << min << "*"; }
		else
		if (min == max)           { out << min; }
		else                      { out << min << "*" << max; }
		phrase ->print (out);
	}

	void range::   print (std::ostream& out, bool isroot) {
		out << "U+" << std::hex << min << "-U+" << std::hex << max;
	}

	void choose::  print (std::ostream& out, bool isroot) {
		if (!isroot) { out << "("; }
		for (size_t i = 0; i != chars.size(); ++i) {
			if (i) out << " | ";
			out << "U+" << std::hex << chars[i];
		}
		if (!isroot) { out << ")"; }
	}

	void rule::    print (std::ostream& out, bool isroot) {
		out << lhs << "\t= ";
		rhs ->print (out, true);
	}



/*/// ================================================================================================================================
	Copy is used to support certain grammar manipulations and is not a general interface. In fact, we want this to blow up
	if general use is attempted.
/*/// --------------------------------------------------------------------------------------------------------------------------------
	term* literal:: copy () { return new literal (*this); }
	term* symbol::  copy () { return new symbol (*this); }
	term* epsilon:: copy () { return new epsilon (*this); }
	term* number::  copy () { return new number (*this); }
	term* list::    copy () { return nullptr; }
	term* mod::     copy () { return nullptr; }
	term* range::   copy () { return nullptr; }
	term* choose::  copy () { return nullptr; }
	term* rule::    copy () { return nullptr; }


/*/// ================================================================================================================================
	Check to see if a string exsits somewhere withn the AST
/*/// --------------------------------------------------------------------------------------------------------------------------------
	bool literal:: contains (std::string n) { return text == n; }
	bool mod::     contains (std::string n) { return phrase->contains (n); }
	bool range::   contains (std::string n) { return false; }
	bool choose::  contains (std::string n) { return false; }
	bool epsilon:: contains (std::string n) { return false; }
	bool number::  contains (std::string n) { return false; }
	bool rule::    contains (std::string n) { return rhs->contains (n); }

	bool list::    contains (std::string n) {
		for (auto i : parts) { if (i->contains (n)) return true; }
		return false;
	}

/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	list& list:: merge_after (list* other) {
		parts .insert (parts.end(), other->parts.begin(), other->parts.end());
		other->parts.empty();
		return *this;
	}
	
	list& list:: merge_before (list* other) {
		parts .insert (parts.begin(), other->parts.begin(), other->parts.end());
		other->parts.empty();
		return *this;
	}


/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	std::string generate_name (std::string& from, size_t& sym) {
		std::string s (from);
		s .append ("_");
		s .append (std::to_string (sym));
		++sym;
		return s;
	}


/*/// ================================================================================================================================
	Transformations to undo the high level notation:
/*/// --------------------------------------------------------------------------------------------------------------------------------
	struct transform {
		grammar& g;
		std::deque <rule*> q;
		size_t  sym = 1;
		
		transform (grammar& agrammar) : g (agrammar) { }
		~transform () { }
		
		
		// promote nested alternatives to a dedicated rule
		
		void expand_nested_alternation () {
			for (auto& r : g.rules) {
				q .push_back (r.second);
			}
			
			while (!q.empty()) {
				auto rule = q.front (); q.pop_front ();
				rule ->inner_find_and_replace (anAlt,
				[&, this](term* t) -> term* {
					auto nr = new aa::rule (generate_name (rule->lhs, sym));
					this->g.insert (nr);
					q .push_back (nr);

					nr ->rhs = t;
					return new symbol (nr->lhs);
				});
			}
		}

		void expand_choices () {
			for (auto& r : g.rules) {
				q .push_back (r.second);
			}
			
			while (!q.empty()) {
				auto rule = q.front (); q.pop_front ();
				
				rule ->find_and_replace (aChoose,
				[&, this](term* t) -> term* {
					auto ch = t ->as (aChoose);
					auto nr = new aa::rule (generate_name (rule->lhs, sym));
					auto al = new alt ();
					this->g.insert (nr);
					q .push_back (nr);

					for (auto c : ch->chars) {
						al ->append (new aa::literal (c));
					}
					nr ->rhs = al;
					return new symbol (nr->lhs);
				});
			}
		}
		
		// expand character choices into primitive alternation
		
		void expand_options () {
			for (auto& r : g.rules) {
				q .push_back (r.second);
			}
			
			while (!q.empty()) {
				auto rule = q.front (); q.pop_front ();
				
				rule ->find_and_replace (anOption,
				[&, this](term* t) -> term* {
					auto op = t ->as (anOption);
					auto nr = new aa::rule (generate_name (rule->lhs, sym));
					auto al = new alt ();
					this->g.insert (nr);
					q .push_back (nr);

					al ->append (op->phrase);
					al ->append (new epsilon ());
					nr ->rhs = al;
					op ->phrase = nullptr;
					return new symbol (nr->lhs);
				});
			}
		}

		// expand repeats by unrolling minimum amounts and capping with right recursion
		
		void expand_repeats () {
			for (auto& r : g.rules) {
				q .push_back (r.second);
			}
			
			while (!q.empty()) {
				auto rule = q.front(); q.pop_front();
				
				rule ->find_and_replace (aRepeat,
				[&, this](term* t) -> term* {
					auto rp = t ->as (aRepeat);
					auto nr = new aa::rule (generate_name (rule->lhs, sym));
					this->g.insert (nr);
					q .push_back (nr);

					if (rp->min == rp->max) {
						if (rp->min == 0) { // zero or more
							auto a = new alt ();
							auto s = new seq ();
							nr ->rhs = a;
							a ->append (s);
							s ->append (rp->phrase);
							s ->append (new symbol (nr->lhs));
							a ->append (new epsilon());
						}
						else { // exactly
							auto s = new seq ();
							nr ->rhs = s;
							for (int i = 0; i != rp->min; ++i) { s ->append (rp->phrase); }
						}
					}
					else {
						if (rp->max > 0) { // at most max
							auto a = new alt ();
							nr ->rhs = a;
							for (int i = rp->max; i >= rp->min; --i) {
								if (rp->min > 0) {
									auto s = new seq ();
									for (int j = 0; j >= i; ++j) { s ->append (rp->phrase); }
									a ->append (s);
								}
								else {
									a->append (new epsilon());
								}
							}
						}
						else { // at least min
							auto s = new seq ();
							nr ->rhs = s;
							for (int i = 0; i != rp->min; ++i) { s->append (rp->phrase); }
							
							auto nr2 = new aa::rule (generate_name (rule->lhs, sym));
							auto al2 = new alt ();
							nr2 ->rhs = al2;
							al2 ->append (new aa::seq (rp->phrase, new symbol (nr2->lhs)));
							al2 ->append (new epsilon());
							this->g.insert (nr2);
							q .push_back (nr2);
							s ->append (new symbol (nr2->lhs));
						}
					}
					rp->phrase = nullptr;
					return new symbol (nr->lhs);
				});
			}
		}
		
		
		// only used by flatten_sequence
		
		void unnest (terms& ts, seq* s) {
			for (auto i : s->parts) {
				if (i ->is (aSeq)) {
					unnest (ts, i->as (aSeq));
				}
				else {
					ts .push_back (i);
				}
			}
		}
	
		// remove nested sequences -- expanding into a single, flat, run
		void flatten_sequences () {
			for (auto& ri : g.rules) {
				auto r = ri.second;
				if (r->rhs->is (aSeq)) {
					terms ts;
					auto   s = r->rhs->as (aSeq);
					unnest (ts, s);
					s->parts .clear ();
					s->parts = ts;
				}
				else
				if (r->rhs->is (anAlt)) {
					auto a = r->rhs->as (anAlt);
					for (auto ai : a->parts) {
						if (ai ->is (aSeq)) {
							terms ts;
							auto   s = ai->as (aSeq);
							unnest (ts, s);
							s->parts .clear();
							s->parts = ts;
						}
					}
				}
			}
		}

		// true when an epsilon alternative was encountered and removed
		bool removed_epsilon (rule* r) {
			if (r->rhs ->is (anAlt)) {
				auto a = r->rhs->as(anAlt);
				if (a->parts.back()->is (anEpsilon)) {
					delete a->parts.back();
					a->parts.pop_back();
					return true;
				}
			}
			return false;
		}
		
		// to support epsilon removal, we have to do combinatorial substitutions
		void create_combinations (std::string& name, size_t occurs, term* t, terms& out) {
			auto s = t->as (aSeq);
			if (s == nullptr) {
				out .push_back (t->copy());
				out .push_back (new epsilon());
				return;
			}
			
			for (size_t combo = 0; combo != (1 << occurs); ++combo) {
				size_t pos = 0;
				auto ns = new seq ();
				out .push_back (ns);
				for (auto& i : s->parts) {
					
					auto sym = i->as (aSymbol);
					if (sym == nullptr) {
						ns->append (i->copy());
					}
					else
					if (sym->text == name) {
						if (combo & (1 << pos)) { ns->append (i->copy()); }
						++pos;
					}
					else {
						ns->append (i->copy());
					}
				}
			}
		}
		
		// find out how many occurences of an elimainated epsilon rule occurs in a referencing rule
		size_t count_occurences (std::string& name, term* in) {
			if (in ->is (aSymbol) && in->as (aSymbol)->text == name) return 1;
			auto sq = in->as (aSeq);
			if (sq == nullptr) return 0;
			
			size_t occ = 0;
			for (auto i : sq->parts) {
				auto sym = i ->as (aSymbol);
				if (sym != nullptr && sym->text == name) ++occ;
			}
			
			return occ;
		}
		
		// locate rules that referenced epsilon generating rules and rewrite them
		void fixup_rules_for (std::string& name) {
			for (auto& i : g.rules) {
				if (i.second->contains (name)) {
					auto r = i.second;
					if (r->rhs->is (anAlt)) {
						auto a  = r->rhs->as (anAlt);
						terms nl;
						
						for (auto i : a->parts) {
							auto occ = count_occurences (name, i);
							if (occ) {
								create_combinations (name, occ, i, nl);
							}
							else {
								nl .push_back (i);
							}
						}
						
						a->parts.clear();
						a->parts = nl;
					}
					else {
						auto occ = count_occurences (name, r->rhs);
						if (occ) {
							terms nl;
							create_combinations (name, occ, r->rhs, nl);
							if (r->rhs->is(aSeq)) { r->rhs->as (aSeq)->parts.clear(); }
							delete r->rhs;
							auto a = new alt();
							r->rhs = a;
							a->parts = nl;
						}
					}
				}
			}
		}
		
		// remove epsilon rules
		void remove_epsilon () {
			loop:
			bool changed = false;
			for (auto& i : g.rules) {
				if (removed_epsilon (i.second)) {
					auto& n = i.second->lhs;
					fixup_rules_for (n);
					changed = true;
				}
			}
			if (changed) goto loop;
		}
		
	};

/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	grammar:: grammar () {
	}
	
	grammar:: ~grammar () {
	}
	
	rule* grammar:: find (const std::string& s) {
		auto f = rules .find (s);
		if (f == rules.end()) { return nullptr; }
		return f->second;
	}
	
	grammar& grammar:: insert (rule* r) {
		auto f = find (r->lhs);
		
		if (f == nullptr) {
			rules .insert ({r->lhs, r});
		}
		else {
			if (f->rhs ->is (anAlt)) {
				if (r->rhs ->is (anAlt)) {
					f->rhs->as (aList)-> merge_after (r->rhs->as (aList));
				}
				else {
					f->rhs->as (aList)-> append (r->rhs);
					r->rhs = nullptr;
				}
			}
			else {
				if (r->rhs ->is (anAlt)) {
					r->rhs ->as (aList)-> merge_before (f->rhs->as (aList));
					f->rhs = r->rhs;
				}
				else {
					auto a = new alt ();
					a ->append (f->rhs);
					a ->append (r->rhs);
					f->rhs = a;
				}
			}
			r->rhs = nullptr;
			delete r;
		}
		return *this;
	}

	void grammar:: transform () {
		aa::transform xform (*this);
		xform .expand_nested_alternation ();
		xform .expand_options ();
		xform .expand_repeats ();
		xform .flatten_sequences ();
		xform .remove_epsilon ();
	}

	void grammar:: dump (std::ostream& out) {
		for (auto& i : rules) {
			i.second ->dump (out);
			out << "\n";
		}
	}

	void grammar:: print (std::ostream& out) {
		for (auto& i : rules) {
			i.second ->print (out);
			out << "\n";
		}
	}

	rulesview grammar:: make_rules_view () {
		rulesview rv;
		
		for (auto& i : rules) {
			auto rhs = i.second->rhs;
			if (rhs->is (anAlt)) {
				auto a = rhs ->as (anAlt);
				for (auto j : a->parts) {
					rv .emplace_back (rview { i.second->lhs, j});
				}
			}
			else {
				rv .emplace_back (rview {i.second->lhs, rhs});
			}
		}
		
		return rv;
	}

	namesview grammar:: make_names_view () {
		namesview nv;
		
		for (auto& i : rules) {
			nv .push_back (i.second->lhs);
		}
		
		return nv;
	}

}