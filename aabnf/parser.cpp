//
//  parser.cpp
//  aabnf
//
//  Copyright © 2016 Theo Johnson. All rights reserved.
//

#include <iostream>
#include <vector>

#include "grammar.hpp"
#include "parser.hpp"

using namespace std;

namespace aa {

/*/// ================================================================================================================================
	Recursive Descent Parser.
	start = ruleslist()
	
	It pretty much tracks the ABNF specification.
/*/// --------------------------------------------------------------------------------------------------------------------------------


/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	using terms = vector <term*>;

	
/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	namespace go {
		uchar*  nbeg = nullptr;
		uchar*  nend = nullptr;
		uint64_t num = 0;
		
		terms   wk;
		
		size_t  errs = 0;
		
		void initialize () {
			for (auto& i : wk) {
				if (i != nullptr) { delete i; }
			}
			
			wk.clear();
		}
		
		inline uint64_t get_number () { return num; }
		
		bool note_beg (uchar* p) { nbeg = p; return true; }
		bool note_end (uchar* p) { nend = p; return true; }
		
		bool err_no_rule ()  {
			std::cout << "No rules specified.\n";
			++errs;
			return false;
		}

		bool err_bad_eval (size_t line) {
			std::cout << "Badly formed eval directive at " << line << ".\n";
			++errs;
			return false;
		}
		
		bool err_bad_rule (size_t line) {
			std::cout << "Badly formed rule at or near line " << line << ".\n";
			++errs;
			return false;
		}
		
		bool append () {
			if (wk.size() >= 2) {
				auto t = wk .back(); wk .pop_back();
				wk.back()->as (aList) ->append (t);
				return true;
			}
			return false;
		}
		
		void sort_choose () {
			auto c = wk.back()->as (aChoose);
			if (c != nullptr) {
				std::sort (c->chars.begin(), c->chars.end());
			}
		}
		
		bool set_phrase () {
			if (wk.size() >= 2) {
				auto t = wk .back(); wk .pop_back();
				wk.back()->as (aMod) ->phrase = t;
				return true;
			}
			return false;
		}

		bool set_rhs () {
			if (wk.size() >= 2) {
				auto t = wk .back(); wk .pop_back();
				wk.back()->as (aRule) ->rhs = t;
				return true;
			}
			return false;
		}
		
		bool combine (size_t howmany) {
			if (wk.size() >= howmany+1) {
				auto t = wk.back()-> as (aList); wk.pop_back();
				for (size_t i = 0; i != howmany; ++i) {
					t ->prepend (wk.back()); wk.pop_back();
				}
				wk .push_back (t);
				return true;
			}
			return false;
		}
		
		bool drop () {
			delete wk.back();
			wk.pop_back();
			return true;
		}
		

		bool name ()          { wk .push_back (new aa::symbol (nbeg, nend)); return true; }
		bool text ()          { wk .push_back (new aa::literal (nbeg, nend)); return true; }
		bool rule ()          { wk .push_back (new aa::rule (nbeg, nend)); return true; }
		bool alternation ()   { wk .push_back (new aa::alt()); return true; }
		bool concatenation () { wk .push_back (new aa::seq()); return true; }
		bool group ()         { wk .push_back (new aa::group()); return true; }
		bool choose ()        { wk .push_back (new aa::choose()); return true; }
		bool option ()        { wk .push_back (new aa::option()); return true; }
		bool eval ()          {	wk .push_back (new aa::eval()); return true; }

		bool repeat (uint64_t min, uint64_t max) { wk .push_back (new aa::repeat(min, max)); return true; }
		bool range (uint64_t min, uint64_t max)  { wk .push_back (new aa::range(min, max)); return true; }
		
		void dump () {
			for (auto i : wk) {
				cout << i << "\n";
			}
		}
		
		
		inline void add_char_to_choose (char32_t c) {
			auto ch = wk.back()->as (aChoose);
			if (ch != nullptr) { ch ->add (c); }
		}
		
		void num_from_decimal ()        {
			num = 0;
			for (auto i = nbeg; i != nend; ++i) {
				num = num * 10 + (*i - '0');
			}
		}


		void num_from_binary () {
			num = 0;
			for (auto i = nbeg; i != nend; ++i) {
				num = num * 2 + (*i - '0');
			}
		}

		void num_from_hexadecimal () {
			num = 0;
			for (auto i = nbeg; i != nend; ++i) {
				uint64_t n;
				if      (*i >= 'a') { n = *i - 'a'; }
				else if (*i >= 'A') { n = *i - 'A'; }
				else                { n = *i - '0'; }
				num = num * 16 + n;
			}
		}
		
		grammar* make_grammar () {
			auto g = new grammar ();
			
			for (auto i : wk) {
				if (i ->is (aRule)) {
					g ->insert (i ->as (aRule));
					i = nullptr;
				}
			}
			return g;
		}
	};



/*/// ================================================================================================================================
/*/// --------------------------------------------------------------------------------------------------------------------------------
	namespace pa {
#define many(X)  [&](void)->bool { while (X) {} return true; }()
#define some(X)  [&](void)->bool { if (X) { while (X) {} return true; } return false; }()
#define maybe(X) [&](void)->bool { (X); return true; }()
#define test(X)  [&](void)->bool { auto p = this->save(); if(X) { this->restore(p); return true; } this->restore(p); return false; }
		bool repeat ();
		bool group ();
		bool element ();
		bool option ();
		bool eval ();
		bool char_val ();
		bool num_val ();
		bool prose_val ();
		bool bin_val ();
		bool dec_val ();
		bool hex_val ();
		bool defined_as ();
		bool alternation ();
		bool concatenation ();
		bool repetition ();


		uint8_t*   pos = nullptr;
		uint8_t*   beg = nullptr;
		uint8_t*   end = nullptr;
		size_t     line = 1;

		void next () {
			if (pos >= end) return;
			++pos;
			if (*pos == '\n') ++line;
		};

		inline bool ch (char c) { if (*pos == c) { next(); return true; } return false; }
		
		inline bool str (const char* s) {
			uint8_t* p = pos;
			while (*p == *s) {
				++s; ++p;
				if (*s == '\0') { pos = p; return true; }
				if (*p == '\0') { return false; }
			}
			return false;
		}
		
		inline bool range (char lo, char hi) {
			if (*pos >= lo && *pos <= hi) { next(); return true; }
			return false;
		}

		inline bool DQUOTE () { return ch (0x22); }
		inline bool HTAB ()   { return ch (0x09); }
		inline bool CR ()     { return ch (0x0d); }
		inline bool LF ()     { return ch (0x0a); }
		inline bool CRLF ()   { if (*pos == 0x0d || *pos == 0x0a) { next(); return true; } return false; }
		inline bool CTL ()    { if ((*pos >= 0x00 && *pos <= 0x1f) || *pos == 0x7f) { next(); return true; } return false; }
		inline bool SP ()     { return ch (0x20); }
		inline bool WSP ()    { if (*pos == 0x20 || *pos == 0x09) { next(); return true; } return false; }
		inline bool VCHAR ()  { if (*pos >= 0x21 && *pos <= 0x7e) { next(); return true; } return false; }
		inline bool OCTET ()  { if (*pos >= 0x00 && *pos <= 0xff) { next(); return true; } return false; }
		inline bool LWSP ()   { if (many(WSP() || CRLF() && WSP ())) return true; return false; }

		bool comment () {
			return ch(';') && many(WSP() || VCHAR()) && CRLF();
		}

		bool c_nl()    { if (comment() || CRLF()) return true; return false; }
//		bool c_wsp ()  { return WSP() || (c_nl() && WSP()); }
		bool c_wsp ()  { return WSP() || c_nl(); }
		bool c_wsps () { while (c_wsp()) {} return true; }
				
		inline bool ALPHA () {
			if ((*pos >= 'a' && *pos <= 'z') || (*pos >= 'A' && *pos <= 'Z')) { next(); return true; }
			return false;
		}
		
		inline bool CHAR () {
			if (*pos >= 0x01 && *pos <= 0x7f) { next(); return true; }
			return false;
		}
		
		inline bool DIGIT () {
			if (*pos >= '0' && *pos <= '9') { next(); return true; }
			return false;
		}
		
		inline bool DIGITS() {
			go::note_beg (pos);
			if (DIGIT()) {
				while (DIGIT()) {};
				go::note_end (pos);
				go::num_from_decimal();
				return true;
			}
			go::note_end (pos);
			return false;
		}
		
		inline bool BIT() {
			if (*pos == '0' || *pos == '1') { next(); return true; }
			return false;
		}
		
		inline bool BITS() {
			go::note_beg (pos);
			if (BIT()) {
				while (BIT()) {};
				go::note_end (pos);
				go::num_from_binary();
				return true;
			}
			go::note_end(pos);
			return false;
		}
		
		inline bool HEXDIG() {
			if ((*pos >= '0' && *pos < '9') || (*pos >= 'a' && *pos <= 'f') || (*pos >= 'A' && *pos <= 'F')) { next(); return true; }
			return false;
		}
		
		inline bool HEXDIGS() {
			go::note_beg (pos);
			if (HEXDIG()) {
				while (HEXDIG()) {};
				go::note_end (pos);
				go::num_from_hexadecimal();
				return true;
			}
			go::note_end (pos);
			return false;
		}
		

		uint8_t* save () { return pos; }
		void restore (uint8_t* apos) { pos = apos; }

		bool ruledef() {
			go::note_beg (pos);
			if (ALPHA() && many(ALPHA() || DIGIT() || ch('-'))) {
				go::note_end (pos);
				c_wsps();
				return true;
			}
			go::note_end (pos);
			return false;
		}

		bool elements () {
			return alternation(); // && c_wsps();
		}
		
		bool rule() {
			if (ruledef()) {
				go::rule();
				if (defined_as() && elements()) {
					go::set_rhs();
				}
				else {
					go::err_bad_rule (line);
					return false;
				}
				return true;
			}
			return false;
		}
		
		bool rulename() {
			go::note_beg (pos);
			auto p = save();
			
			if (ALPHA() && many(ALPHA() || DIGIT() || ch('-'))) {
				go::note_end (pos);
				c_wsps();
				
				if (defined_as()) {
					restore (p);
					return false;
				}
				go::name ();
				return true;
			}
			go::note_end (pos);
			return false;
		}
		
		bool defined_as () {
			if (ch('=') || str("=/")) { c_wsps(); return true; }
			return false;
		}
		
		
		bool alternation () {
			if (concatenation()) {
				int c = 1;

				while (ch('/') && c_wsps() && concatenation()) { ++c; }
				if (c > 1) {
					go::alternation();
					go::combine(c);
				}
				return true;
			}
			return false;
		}
		
		bool concatenation () {
			if (repetition()) {
				int c = 1;
				while (repetition()) { ++c; }
				if (c > 1) {
					go::concatenation ();
					go::combine(c);
				}
				return true;
			}
			return false;
		}
		
		bool repetition () {
			if (repeat()) {
				if (element()) {
					go::set_phrase();
					return true;
				}
				else
					return false;
			}
			return element();
		}
		
		bool repeat() {
			uint64_t min = 0;
			uint64_t max = 0;
			if (DIGITS()) {
				min = go::get_number();
				if (ch ('*')) {
					if (DIGITS()) {
						max = go::get_number();
						go::repeat(min, max);
					}
					else {
						go::repeat(min, 0);
					}
				}
				else {
					return false;
				}
				return true;
			}
			else if (ch('*')) {
				go::repeat(0,0);
				return true;
			}
			return false;
		}
		
		bool element () {
			if (rulename() || group () || eval () || option() || char_val() || num_val() || prose_val()) {
				c_wsps();
				return true;
			}
			return false;
		}
		
		bool group () {
			if (ch('(')) {
				c_wsps();
				if (alternation() && ch(')')) {
					return true;
				}
			}
			return false;
		}
		
		bool option () {
			if (ch('[')) {
				go::option ();
				c_wsps();
				if (alternation() && ch(']')) {
					go::set_phrase();
				}
				return true;
			}
			return false;
		}

		bool eval () {
			if (ch('{')) {
				go::eval ();
				c_wsps();
				while (element()) { go::append(); }
				if (ch('}')) {
					return true;
				}
				else {
					go::err_bad_eval (line);
					go::drop ();
					return false;
				}
			}
			return false;
		}
		
		bool char_val () {
			if (DQUOTE()) {
				go::note_beg(pos);
				many(range(0x20, 0x21) || range(0x23, 0x7e));
				go::note_end (pos);
				go::text ();
				if (DQUOTE()) { return true; }
				go::drop();
			}
			return false;
		}
		
		bool num_val () {
			if (ch('%')) {
				if ((bin_val() || dec_val() || hex_val())) {
					return true;
				}
			}
			return false;
		}
		
		bool bin_val () {
			if (ch('b') && BITS()) {
				uint64_t first = go::get_number();
				if (ch('.') && BITS()) {
					go::choose ();
					go::add_char_to_choose (first);
					go::add_char_to_choose (go::get_number());
					while (ch('.') && BITS()) { go::add_char_to_choose (go::get_number()); }
					go::sort_choose ();
				}
				else if (ch('-') && BITS()) {
					uint64_t second = go::get_number();
					go::range(first, second);
				}
				return true;
			}
			return false;
		}
		
		bool dec_val () {
			if (ch('d') && DIGITS()) {
				uint64_t first = go::get_number();
				if (ch('.') && DIGITS()) {
					go::choose ();
					go::add_char_to_choose (first);
					go::add_char_to_choose (go::get_number());
					while (ch('.') && DIGITS()) { go::add_char_to_choose (go::get_number()); }
				}
				else if (ch('-') && DIGITS()) {
					uint64_t second = go::get_number();
					go::range(first, second);
				}
				return true;
			}
			return false;
		}
		
		bool hex_val () {
			if (ch('x') && HEXDIGS()) {
				uint64_t first = go::get_number();
				if (ch('.') && HEXDIGS()) {
					go::choose ();
					go::add_char_to_choose (first);
					go::add_char_to_choose (go::get_number());
					while (ch('.') && HEXDIGS()) { go::add_char_to_choose (go::get_number()); }
				}
				else if (ch('-') && HEXDIGS()) {
					uint64_t second = go::get_number();
					go::range(first, second);
				}
				return true;
			}
			return false;
		}
		
		bool prose_val () {
			if (ch('<')) {
				go::note_beg (pos);
				many(range(0x20, 0x3d) || range(0x3f, 0x7e));
				go::note_end (pos);
				go::text ();
				if (ch('>')) { return true; }
				go::drop();
			}
			return false;
		}
		
		bool rulelist_a () {
			return rule();
		}
		
		bool rulelist () {
			return (some (rulelist_a()) || go::err_no_rule ());
		}
		
		
#undef many
#undef some
#undef maybe
	}

/*/// ================================================================================================================================
	Entry point for parsing a buffer
/*/// --------------------------------------------------------------------------------------------------------------------------------
	grammar* parse (uint8_t* bufbeg, uint8_t* bufend) {
		pa::beg = bufbeg; pa::pos = bufbeg; pa::end = bufend;
		
		go::initialize ();
		pa::c_wsps();
		if (pa::rulelist()) {
			return go::make_grammar ();
		}
		return nullptr;
	}

}