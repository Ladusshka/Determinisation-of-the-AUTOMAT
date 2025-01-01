# Determinisation-of-the-AUTOMAT




his repository provides implementations of two key functions for working with finite automata in C++:

DFA complement(const MISNFA& nfa);

Constructs a deterministic finite automaton (DFA) that recognizes the complement of the language of the given nondeterministic finite automaton (MISNFA).
The resulting DFA is free of unreachable/redundant states.
If the complement language is empty, the function returns a single-state DFA over the original alphabet.
bool run(const DFA& dfa, const Word& word);

Checks whether a deterministic finite automaton (dfa) accepts a given input word (Word).
Returns true if the word is accepted, and false otherwise.
Key Details
MISNFA and DFA are data structures representing automata, where:
State denotes a state ID.
Symbol denotes an alphabet symbol.
Word is std::vector<Symbol>.
The complement function:
Takes a multistate NFA (MISNFA) and converts it into a DFA that accepts the complement language.
Ensures that unreachable states and states unnecessary for recognition are removed.
The run function:
Operates on valid deterministic automata.
Decides if a given word is accepted by the DFA.
Usage
Add your implementation to a single source file along with any helper functions you need.
Keep the conditional-compilation blocks (#ifdef/#endif) from the provided template so the code integrates with the testing environment.
Testing
The provided sample file contains basic tests for the run function.
To validate complement, you can manually test or use external tools (like “ALT”) because your automaton’s internal state naming may differ from reference solutions.
The testing environment will:
Compare automata by converting them to minimal DFAs and checking language equivalence.
Impose time and memory limits during execution.
With this repository, you can:

Generate a complement DFA from a given NFA with multiple start states.
Verify whether a deterministic automaton recognizes a particular input word.
