# aabnf
Augmented ABNF Parser Generator

*Warning: pre-alpha. We're exploring the design space. Many things will change.*

AABNF accepts a grammar in ABNF (as pubished in [RFC5234][https://tools.ietf.org/html/rfc5234]) and generates a parser.

### The current state: ###
- The command line accepts an ABNF grammar and a file to parse using that grammar.
- Can parse the file and yield success or failure
- Will parse ambiguities in parallel. The first choice that completes is the winner.

### TODO ###
- Refactor and encapsulate
- Use the internal the lr parser class as the template for code generation
- Error reporting
- Add a recursive descent parser generator
- Add a structured editor generator
- Add an API for user actions.

