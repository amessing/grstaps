#ifndef GRSTAPS_SYNTAX_ANALYZER_HPP
#define GRSTAPS_SYNTAX_ANALYZER_HPP
#include <string>
#include <vector>

#include <unordered_map>

namespace grstaps
{
    // Number of symbol types
    const unsigned int numSymbols = 71;

    // Symbol types
    enum Symbol
    {
        NAME            = 0,
        VARIABLE        = 1,
        NUMBER          = 2,
        DEFINE          = 3,
        PDDLDOMAIN      = 4,
        OPEN_PAR        = 5,
        CLOSE_PAR       = 6,
        COLON           = 7,
        REQUIREMENTS    = 8,
        TYPES           = 9,
        CONSTANTS       = 10,
        PREDICATES      = 11,
        FUNCTIONS       = 12,
        NUMBER_TYPE     = 13,
        CONSTRAINTS     = 14,
        LENGTH          = 15,
        EITHER          = 16,
        ACTION          = 17,
        PARAMETERS      = 18,
        PRECONDITION    = 19,
        EFFECT          = 20,
        AND             = 21,
        FORALL          = 22,
        PREFERENCE      = 23,
        OR              = 24,
        NOT             = 25,
        IMPLY           = 26,
        EXISTS          = 27,
        MINUS           = 28,
        DIV             = 29,
        PROD            = 30,
        PLUS            = 31,
        GREATER         = 32,
        LESS            = 33,
        EQUAL           = 34,
        GREATER_EQ      = 35,
        LESS_EQ         = 36,
        WHEN            = 37,
        ASSIGN          = 38,
        SCALE_UP        = 39,
        SCALE_DOWN      = 40,
        INCREASE        = 41,
        DECREASE        = 42,
        DURATIVE_ACTION = 43,
        CONDITION       = 44,
        AT              = 45,
        OVER            = 46,
        START           = 47,
        END             = 48,
        ALL             = 49,
        SHARP_T         = 50,
        DERIVED         = 51,
        PROBLEM         = 52,
        OBJECTS         = 53,
        INIT            = 54,
        GOAL            = 55,
        ALWAYS          = 56,
        SOMETIME        = 57,
        WITHIN          = 58,
        AT_MOST_ONCE    = 59,
        SOMETIME_AFTER  = 60,
        SOMETIME_BEFORE = 61,
        ALWAYS_WITHIN   = 62,
        HOLD_DURING     = 63,
        HOLD_AFTER      = 64,
        METRIC          = 65,
        MAXIMIZE        = 66,
        MINIMIZE        = 67,
        TOTAL_TIME      = 68,
        IS_VIOLATED     = 69,
        DURATION        = 70
    };

    // Names of the symbols (only those ones that are keywords)
    const char* const symbolNames[] = {nullptr,
                                       nullptr,
                                       nullptr,
                                       "define",
                                       "domain",
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       "requirements",
                                       "types",
                                       "constants",
                                       "predicates",
                                       "functions",
                                       "number",
                                       "constraints",
                                       "length",
                                       "either",
                                       "action",
                                       "parameters",
                                       "precondition",
                                       "effect",
                                       "and",
                                       "forall",
                                       "preference",
                                       "or",
                                       "not",
                                       "imply",
                                       "exists",
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       nullptr,
                                       "when",
                                       "assign",
                                       "scale-up",
                                       "scale-down",
                                       "increase",
                                       "decrease",
                                       "durative-action",
                                       "condition",
                                       "at",
                                       "over",
                                       "start",
                                       "end",
                                       "all",
                                       "#t",
                                       "derived",
                                       "problem",
                                       "objects",
                                       "init",
                                       "goal",
                                       "always",
                                       "sometime",
                                       "within",
                                       "at-most-once",
                                       "sometime-after",
                                       "sometime-before",
                                       "always-within",
                                       "hold-during",
                                       "hold-after",
                                       "metric",
                                       "maximize",
                                       "minimize",
                                       "total-time",
                                       "is-violated",
                                       "duration"};

    // Description of the symbols
    const char* const symbolDescriptions[] = {"name",
                                              "variable",
                                              "number",
                                              "define",
                                              "domain",
                                              "(",
                                              ")",
                                              ":",
                                              "requirements",
                                              "types",
                                              "constants",
                                              "predicates",
                                              "functions",
                                              "number",
                                              "constraints",
                                              "length",
                                              "either",
                                              "action",
                                              "parameters",
                                              "precondition",
                                              "effect",
                                              "and",
                                              "forall",
                                              "preference",
                                              "or",
                                              "not",
                                              "imply",
                                              "exists",
                                              "-",
                                              "/",
                                              "*",
                                              "+",
                                              ">",
                                              "<",
                                              "=",
                                              ">=",
                                              "<=",
                                              "when",
                                              "assign",
                                              "scale-up",
                                              "scale-down",
                                              "increase",
                                              "decrease",
                                              "durative-action",
                                              "condition",
                                              "at",
                                              "over",
                                              "start",
                                              "end",
                                              "all",
                                              "#t",
                                              "derived",
                                              "problem",
                                              "objects",
                                              "init",
                                              "goal",
                                              "always",
                                              "sometime",
                                              "within",
                                              "at-most-once",
                                              "sometime-after",
                                              "sometime-before",
                                              "always-within",
                                              "hold-during",
                                              "hold-after",
                                              "metric",
                                              "maximize",
                                              "minimize",
                                              "total-time",
                                              "is-violated",
                                              "duration"};

    class Token
    {
       public:
        Symbol symbol;
        std::string description;
        float value;
        Token(Symbol s);
        Token(float v);
        Token(Symbol s, std::string const& desc);
        std::string toString();
    };

    class SyntaxAnalyzer
    {
       private:
        const char* fileName;
        char* buffer;
        int lineNumber;
        int position;
        int bufferLength;
        std::vector<Token*> tokens;
        std::unordered_map<std::string, Symbol> symbols;
        void skipSpaces();
        Token* matchToken();
        bool matchNumber(float* value);

       public:
        int tokenIndex;
        SyntaxAnalyzer(const char* fileName);
        ~SyntaxAnalyzer();
        Token* nextToken();
        Token* readSymbol(Symbol s);
        Token* readSymbol(int numSymbols, ...);
        void openPar();
        void closePar();
        void readColon();
        std::string readName();
        void notifyError(const std::string& msg);
        // Checks whether the token is the given type
        inline bool isSym(Token* token, Symbol s)
        {
            if(token->symbol == s)
                return true;
            if(s == Symbol::NAME)
            {
                // Keywords that can be considered as fluent names
                if(token->symbol == Symbol::AT || token->symbol == Symbol::OVER || token->symbol == Symbol::OBJECTS ||
                   token->symbol == Symbol::LENGTH)
                {
                    token->symbol = Symbol::NAME;
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
        }
        // Reuses the last token
        inline void undoToken()
        {
            tokenIndex--;
        }
    };
}  // namespace grstaps
#endif  // GRSTAPS_SYNTAX_ANALYZER_HPP
