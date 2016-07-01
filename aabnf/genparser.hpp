//
//  genparser.hpp
//  aabnf
//
//  Copyright © 2016 Theo Johnson. All rights reserved.
//

#ifndef genparser_hpp
#define genparser_hpp

#include "grammar.hpp"

namespace aa {
	namespace lr {
	
		// generate a c++ class that will parse a file
		void generate_from (rulesview& rv);
		
		bool parse_using (rulesview& rv, namesview& nv, const char* filename);
	};
}

#endif /* genparser_hpp */
