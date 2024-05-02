#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cmath>

char const number = '8';
char const quit = 'q';
char const print = ';';
char const help = 'h';  // Help command
char const clear = 'c'; // Clear command

class token
{
    char kind_;
    double value_;

public:
    token(char ch) : kind_(ch), value_(0) {}
    token(double val) : kind_(number), value_(val) {}

    char kind() const { return kind_; }
    double value() const { return value_; }
};

std::string const prompt = "> ";
std::string const result = "= ";

class token_stream
{
    bool full;
    token buffer;

public:
    token_stream() : full(false), buffer('\0') {}

    token get();
    void putback(token);
    void ignore(char c);
};

token_stream ts;

void token_stream::putback(token t)
{
    if (full)
        throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get()
{
    if (full)
    {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;

    switch (ch)
    {
    case '(':
    case ')':
    case ';':
    case 'q':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case 's': // sin function
    case 'c': // cos function
    case 't': // tan function
        return token(ch);
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        std::cin.putback(ch);
        double val;
        std::cin >> val;
        return token(val);
    }
    default:
        throw std::runtime_error("Bad token");
    }
}

void token_stream::ignore(char c)
{
    if (full && c == buffer.kind())
    {
        full = false;
        return;
    }
    full = false;

    char ch = 0;
    while (std::cin >> ch)
    {
        if (ch == c)
            break;
    }
}

double expression();

double primary()
{
    token t = ts.get();
    switch (t.kind())
    {
    case '(':
    {
        double d = expression();
        t = ts.get();
        if (t.kind() != ')')
            throw std::runtime_error("')' expected");
        return d;
    }
    case number:
        return t.value();
    case '-':
        return -primary();
    case '+':
        return primary();
    case 's': // sin function
        return std::sin(primary());
    case 'c': // cos function
        return std::cos(primary());
    case 't': // tan function
        return std::tan(primary());
    default:
        throw std::runtime_error("primary expected");
    }
}

double term()
{
    double left = primary();
    while (true)
    {
        token t = ts.get();
        switch (t.kind())
        {
        case '*':
            left *= primary();
            break;
        case '/':
        {
            double d = primary();
            if (d == 0)
                throw std::runtime_error("divide by zero");
            left /= d;
            break;
        }
        case '%':
        {
            double d = primary();
            if (d == 0)
                throw std::runtime_error("divide by zero");
            left = std::fmod(left, d);
            break;
        }
        case '^':
            left = std::pow(left, primary());
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

double expression()
{
    double left = term();
    while (true)
    {
        token t = ts.get();
        switch (t.kind())
        {
        case '+':
            left += term();
            break;
        case '-':
            left -= term();
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

void calculate()
{
    while (std::cin)
    {
        try
        {
            std::cout << prompt;
            token t = ts.get();

            while (t.kind() == print)
                t = ts.get();

            if (t.kind() == quit)
                return;

            if (t.kind() == help)
            {
                std::cout << "Simple Calculator Commands:\n"
                             "- Enter expressions using numbers and operators: +, -, *, /, % (modulus), ^ (exponentiation).\n"
                             "- Use parentheses for grouping, e.g., (3 + 4) * 2.\n"
                             "- Supported functions: sin, cos, tan.\n"
                             "- Type 'q' to quit.\n"
                             "- Type 'c' to clear the screen.\n";
            }
            else if (t.kind() == clear)
            {
                std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear screen
            }
            else
            {
                ts.putback(t);
                std::cout << result << expression() << std::endl;
            }
        }
        catch (std::runtime_error const &e)
        {
            std::cerr << e.what() << std::endl;
            clean_up_mess();
        }
    }
}

int main()
{
    try
    {
        calculate();
        return 0;
    }
    catch (...)
    {
        std::cerr << "exception\n";
        return 2;
    }
}