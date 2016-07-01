//
//  grammar.hpp
//  aabnf
//
//

#ifndef grammar_hpp
#define grammar_hpp

#include <string>
#include <iostream>
#include <vector>
#include <map>

#include "defs.hpp"

namespace aa {

/*/// ================================================================================================================================
	AST
/*/// --------------------------------------------------------------------------------------------------------------------------------
	struct term {
		unsigned label = 0;
		
		virtual ~term ();
		
		template <typename Other> auto as (const Other&) -> Other* {
			return dynamic_cast <Other*> (this);
		}

		template <typename Other> auto is (const Other&) -> bool {
			return dynamic_cast <Other*> (this) != nullptr;
		}
		
		virtual bool kindof (const term& other) = 0;
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with) = 0;
		
		virtual void dump (std::ostream& out) = 0;
		virtual void print (std::ostream& out, bool isroot = false) = 0;
		
		virtual term* copy () = 0;
		virtual bool contains (std::string n) = 0;
	};

	struct epsilon : public term {
		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	using terms = std::vector <term*>;
	
	struct literal : public term {
		literal ();
		literal (uchar ch);
		literal (uchar* beg, uchar* end);
		literal (const std::string& s);
		std::string text;

		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	struct symbol : public literal {
		using literal::literal;

		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
	};
	
	struct number : public term {
		number ();
		number (uint64_t value);
		
		uint64_t num   = 0;

		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	struct list : public term {
		terms parts;
		
		list ();
		template <typename...Args> list (Args... args) { append (args...); }
		virtual ~list ();
		
		inline list& append (term* other)            { parts .push_back (other); return *this; }
		template <typename First, typename... Rest>
		inline list& append (First f, Rest... args)  { parts .push_back (f); append (args...); return *this; }
		template <typename Last>
		inline list& append (Last a)                 { parts .push_back (a); return *this; }

		inline list& append () { return *this; }

		inline list& prepend (term* other)           { parts .insert (parts.begin(), other); return *this; }
		template <typename First, typename... Rest>
		inline list& prepend (First f, Rest... args) { parts .insert (parts.begin(), f); prepend (args...); return *this; }
		template <typename Last>
		inline list& prepend (Last a)                { parts .insert (parts.begin(), a); return *this; }

		inline list& prepend () { return *this; }
		
		list& merge_after (list* other);
		list& merge_before (list* other);
	
		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void dumpfunctor (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	struct alt : public list {
		using list::list;

		virtual bool kindof (const term& other);

		virtual void dumpfunctor (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);
	};
	
	struct seq : public list {
		using list::list;

		virtual bool kindof (const term& other);

		virtual void dumpfunctor (std::ostream& out);
	};
	
	struct group : public list {
		using list::list;

		virtual bool kindof (const term& other);

		virtual void dumpfunctor (std::ostream& out);
	};
	
	struct eval : public list {
		using list::list;

		virtual bool kindof (const term& other);
		virtual void dumpfunctor (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);
	};
	
	struct mod : public term {
		mod ();
		mod (term* inner);
		virtual ~mod ();
		
		term* phrase    = nullptr;

		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	struct option : public mod {
		virtual bool kindof (const term& other);
		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);
	};
	
	struct repeat : public mod {
		int min    = 0;
		int max    = 0;

		repeat ();
		repeat (unsigned amin, unsigned max);
		repeat (unsigned amin, unsigned amax, term* phrase);
		using mod::mod;

		virtual bool kindof (const term& other);
		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);
	};
	
	struct range : public term {
		char32_t min    = 0;
		char32_t max    = 0;
		
		range ();
		range (char32_t amin, char32_t amax);

		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	struct choose : public term {
		std::vector <char32_t> chars;
		
		choose ();

		inline choose& add (char32_t c) { chars .push_back (c); return *this; }
		
		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	
	struct rule : public term {
		std::string lhs;
		term*       rhs;

		rule ();
		rule (uchar* beg, uchar* end);
		rule (const std::string& s);
		rule (const std::string& s, term* arhs);

		virtual ~rule ();
		
		virtual bool kindof (const term& other);
		virtual void find_and_replace (term& type, const std::function<term*(term*)>& with);
		virtual void inner_find_and_replace (term& type, const std::function<term*(term*)>& with);

		virtual void dump (std::ostream& out);
		virtual void print (std::ostream& out, bool isroot = false);

		virtual term* copy ();
		virtual bool contains (std::string n);
	};
	
	struct rview {
		std::string lhs;
		term*       rhs;
	};

	std::ostream& operator<< (std::ostream& out, rview& rv);
	
/*/// ================================================================================================================================
	The highlevel representation of a grammar
/*/// --------------------------------------------------------------------------------------------------------------------------------
	using rulesmap = std::map <const std::string, rule*>; // internal representation of a grammar using the AST form
	using rulesview = std::vector <rview>;                // BNF like view of a grammar
	using namesview = std::vector <std::string>;          // Just the names
	
	struct grammar {
		rulesmap rules;
		size_t   nextsym = 1;
		
		grammar ();
		~grammar ();
		
		rule* find (const std::string& s);
		
		grammar& insert (rule* r);

		void transform ();
		
		void dump (std::ostream& out);
		virtual void print (std::ostream& out);
		
		rulesview make_rules_view ();
		
		namesview make_names_view ();
	};
	
	extern list   aList;
	extern alt    anAlt;
	extern seq    aSeq;
	extern repeat aRepeat;
	extern option anOption;
	extern mod    aMod;
	extern group  aGroup;
	extern range  aRange;
	extern choose aChoose;
	extern number aNumber;
	extern symbol aSymbol;
	extern literal aLiteral;
	extern rule   aRule;
	extern epsilon anEpsilon;
}

#endif /* grammar_hpp */
