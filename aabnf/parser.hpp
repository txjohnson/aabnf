//
//  parser.hpp
//  aabnf
//
//  Copyright © 2016 Theo Johnson. All rights reserved.
//

#ifndef parser_hpp
#define parser_hpp

namespace aa {
	struct grammar;
	
	grammar* parse (uint8_t* bufbeg, uint8_t* bufend);

}

#endif /* parser_hpp */
