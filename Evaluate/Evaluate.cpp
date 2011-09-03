//This file is part of FingerText, a notepad++ snippet plugin.
//
//This file is modified from the demo program eval.cpp
//on http://oomusou.cnblogs.com
//
//FingerText is released under MIT License.
//
//MIT license
//
//Copyright (C) 2011 by Tom Lam
//
//Permission is hereby granted, free of charge, to any person 
//obtaining a copy of this software and associated documentation 
//files (the "Software"), to deal in the Software without 
//restriction, including without limitation the rights to use, 
//copy, modify, merge, publish, distribute, sublicense, and/or 
//sell copies of the Software, and to permit persons to whom the 
//Software is furnished to do so, subject to the following 
//conditions:
//
//The above copyright notice and this permission notice shall be 
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//DEALINGS IN THE SOFTWARE.

#include "Evaluate.h"

// constructor
Expression::Expression(char *input) 
{
    isError = false;
    this->processExpression(input);        
}

Expression::Expression(std::string input) 
{
    isError = false;
    this->processExpression(input);        
}
//TODO: make a class to centralize all the type conversion
// convert char to string
std::string Expression::toString(const char &c)
{
    std::stringstream ss;
    ss << c;
    return ss.str();
}

// convert double to string
std::string Expression::toString(const double &d) {
    std::stringstream ss;
    ss << d;
    return ss.str();
}

// convert string to double
double Expression::stringToDouble(const std::string &s)
{
    std::stringstream ss(s);
    double d;
    ss >> d;
    return d;
}

// return the priority of a given operator, if the char is not an
// operator, return 0 so that this function can also be used to 
// perform operator check
int Expression::checkOperator(const char &c) 
{
    switch(c) 
    {
        case '$' : 
        case '#' : return 11;
        case 'R' : return 10;
        case 'n' :
        case '%' : return 9;
        case 'r' :
        case 'f' :
        case 'F' :
        case 's' :
        case 'o' :
        case 't' :
        case 'l' :
        case 'S' :
        case 'O' :
        case 'T' :
        case 'L' :
        case 'X' :
        case 'A' :
        case '!' : return 8;
        case '^' : return 7;
        case 'P' :
        case 'p' : return 6;
        case 'M' : 
        case '*' :
        case '/' : return 5;
        case '+' :
        case '-' : return 4;
        case '=' :
        case 'N' : 
        case 'G' :
        case 'g' : 
        case '>' :
        case '<' : return 3;
        case '&' :
        case '|' : return 2;
        case '(' : 
        case ')' : return 1;
        default  : return 0;
    }
}


bool Expression::isDigit(const char &c) 
{
    switch(c) 
    {
        case '.' :
        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' : return true;
        default  : return false;
    }
}

std::string Expression::rephrasing(std::string input)
{
    int length = input.length();
    for (int j=0; j<length; ++j) input[j]=tolower(input[j]);

    //TODO: need to fix the problem of negative number. solution can be find and replace all - by 0- if the char before - is not a digit

    //Operators
    

    findAndReplace(input,"==","=");
    findAndReplace(input,"==","=");
    findAndReplace(input,">=","G");
    findAndReplace(input,"<=","g");
    findAndReplace(input,"!=","N");
    findAndReplace(input,"mod","0M");
    findAndReplace(input,"exp","0X");
    findAndReplace(input,"ceil","0F");
    findAndReplace(input,"rand","0R0");
    findAndReplace(input,"floor","0f");
    findAndReplace(input,"round","0r");
    findAndReplace(input,"abs","0A");
    findAndReplace(input,"ln","0l");
    findAndReplace(input,"log","0L");
    findAndReplace(input,"asin","0S");
    findAndReplace(input,"sin","0s");
    findAndReplace(input,"acos","0O");
    findAndReplace(input,"cos","0o");
    findAndReplace(input,"atan","0T");
    findAndReplace(input,"tan","0t");
    findAndReplace(input,"!","!0");
    findAndReplace(input,"%","%0");
    // Constants
    findAndReplace(input,"pi","3.141592654");
    findAndReplace(input,"e","2.71828183");  // execute the find and replace after all others because other words may contain e
    findAndReplace(input,"p","P");
    findAndReplace(input,"c","p");
    
    findAndReplace2(input,"-","0n");

    return input;
}

// parse operand to operandStack
void Expression::parseOperand(const double &operand) 
{
  postfix.push_back(make_pair(OPERAND, toString(operand)));
}

// parse operator to operatorStack
void Expression::parseOperator(const char &c)
{

    if (operatorStack.empty() || c == '(') 
    {
        operatorStack.push(c);
    } else 
    {
        if (c == ')') 
        {
            while(operatorStack.top() != '(') 
            {
                postfix.push_back(make_pair(OPERATOR, toString(operatorStack.top())));
                operatorStack.pop();
                
                if (operandStack.empty()) 
                {
                    break;
                }
            }
            // Remove '('
            if (!operatorStack.empty()) 
            {
                operatorStack.pop();
            } else
            {
                // Missing '('
                isError = true;
            }
        }
        else 
        { // not ')'
            while(checkOperator(c) <= checkOperator(operatorStack.top()) && !operatorStack.empty()) 
            {
                postfix.push_back(make_pair(OPERATOR, toString(operatorStack.top())));
                operatorStack.pop();
                
                if (operatorStack.empty()) 
                    break;
            }
            operatorStack.push(c);
        }
    }
}

void Expression::processExpression(std::string input)
{
    std::string infix = rephrasing("0$"+input+"#0");
    this->toPostfix(infix);  // convert infix to postfix
}

// Convert infix to postfix
// Algorithm : 
// Read the infix string one char by one char. If char is operator, 
// check if operand is "", if no, let operand to operandStack.
// Parse the operator to the stack.  
// If char is digit, concatenate to operand string if the digit
// is not '.'. If the digit is '.' concat only if the operand is not 
// a decimal.

void Expression::toPostfix(std::string infix)
{
    isDecimal = false;
    std::string operand;
    for(std::string::iterator p = infix.begin(); p != infix.end(); ++p) 
    {
        if (checkOperator(*p)) 
        {
            if (operand != "") 
            {
                parseOperand(stringToDouble(operand));
                operand.clear();
                isDecimal = false;
                //parseOperator(*p);
            }
            //else if ((*p == '(') || (*p == ')'))
            //{
            parseOperator(*p);
            //}
        } else if (isDigit(*p))
        {
            if (*p == '.') 
            {
                if (!isDecimal) 
                {
                    operand.push_back(*p);
                    isDecimal = true;
                }
            } else
            {
                operand.push_back(*p);
            }
        }
    }
    
    // If operand is not "", let operand to operandStack.
    if (operand != "") parseOperand(stringToDouble(operand));
    
    // If operatorStack is not empty, push it to postfix vector until operatorStack is empty.
    while(!operatorStack.empty()) 
    {
        postfix.push_back(make_pair(OPERATOR,toString(operatorStack.top())));
        operatorStack.pop();
    }
}

// calculate result by operator and operand
double Expression::operate(const std::string &operation, const double &operand1, const double &operand2) 
{
    switch(operation[0])
    {   
        case '+':
            return operand2 + operand1;
        case '-':
            return operand2 - operand1;
        case '*':
            return operand2 * operand1;
        case '/':
            return operand2 / operand1;
        case 'M':
            return static_cast<long>(operand2) % static_cast<long>(operand1);
        case '^':
            return std::pow(operand2,operand1);
        case '!':
            return factorial(static_cast<long>(operand2));
        case 'l':
            return log(operand1);
        case 'L':
            return log10(operand1);
        case 's':
            return sin(operand1);
        case 'S':
            return asin(operand1);
        case 'o':
            return cos(operand1);
        case 'O':
            return acos(operand1);
        case 't':
            return tan(operand1);
        case 'T':
            return atan(operand1);
        case 'n':
            return operand2 - operand1;
        case '#':
            return operand2;
        case '$':
            return operand1;
        case '=':
            return isEqual(operand1,operand2);
        case 'N':
            return isNotEqual(operand1,operand2);
        case 'G':
            return isGreaterOrEqual(operand1,operand2);
        case 'g':
            return isSmallerOrEqual(operand1,operand2);
        case '>':
            return isGreater(operand1,operand2);
        case '<':
            return isSmaller(operand1,operand2);
        case '&':
            return operateAnd(operand1,operand2);
        case '|':
            return operateOr(operand1,operand2);
        case 'p':
            return ncr(operand1,operand2);
        case 'P':
            return npr(operand1,operand2);
        case 'r':
            return floor(operand1+0.5);
        case 'F':
            return ceil(operand1);
        case 'f':
            return floor(operand1);
        case 'X':
            return exp(operand1);
        case 'A':
            return fabs(operand1);
        case '%':
            return operand2/100;
        case 'R':
            return randomNumber();
        default:
            return 0;
    }
}


long Expression::randomNumber()
{
    //clock_t t1;
    //t1 = clock();
    //long clockSeed = t1 % 10;
    //static long seed = time(NULL);            
    //seed = (104729 + 7919 * (seed+clockSeed)) % 15485863;  
    //return abs(seed)%100;
    

    //static long seed = time(NULL);            
    //seed = (104729 + 7919 * seed) % 15485863;  
    //return abs(seed)%100;
    
    //if (rand() > rand())
    //{
    //    int s = static_cast<int>(time(0));
    //    srand (rand()*s%32767);
    //}
    //return rand() % 100;

    return rand() % 100;  //TODO: this rely on the seed somewhere else
}

int Expression::operateAnd(double operand1, double operand2)
{
    if ((operand2 == 0) && (operand1 == 0)) return 0;
    else return 1;
}

int Expression::operateOr(double operand1, double operand2)
{
    if ((operand2 == 0) || (operand1 == 0)) return 0;
    else return 1;
}

int Expression::isGreater(double operand1, double operand2)
{
    if (operand2 > operand1) return 0;
    else return 1;
}

int Expression::isSmaller(double operand1, double operand2)
{
    if (operand2 < operand1) return 0;
    else return 1;
}

int Expression::isGreaterOrEqual(double operand1, double operand2)
{
    if (operand2 >= operand1) return 0;
    else return 1;
}

int Expression::isSmallerOrEqual(double operand1, double operand2)
{
    if (operand2 <= operand1) return 0;
    else return 1;
}

int Expression::isEqual(double operand1, double operand2)
{
    if (operand1 == operand2) return 0;
    else return 1;
}

int Expression::isNotEqual(double operand1, double operand2)
{
    if (operand1 != operand2) return 0;
    else return 1;
}

long Expression::ncr(long operand1, long operand2)
{
    if (operand1 < 0 || operand2 < 0 || operand1 > operand2)
    {
        operand1 = 1;
        operand2 = 1;
        isError = 1;
    }

    return factorial(operand2)/(factorial(operand1)*factorial(operand2 - operand1));
}

long Expression::npr(long operand1, long operand2)
{
    if (operand1 < 0 || operand2 < 0 || operand1 > operand2)
    {
        operand1 = 1;
        operand2 = 1;
        isError = 1;
    }
    return factorial(operand2)/factorial(operand2 - operand1);
}

long Expression::factorial(long number)
{
    if (number < 0) isError = 1;
    long result = 1;
    for (long i = number; i>0; i--) result = result * i;
    return result;
}

// get evaluation result
double Expression::compute(void) 
{
    // The operandstack should be empty, if not, empty it label it an error
    while(!operandStack.empty()) 
    {
        isError = true;
        operandStack.pop();
    }

    for(std::vector<std::pair<int, std::string>>::iterator iter = postfix.begin(); iter != postfix.end(); ++iter) 
    {
        if (iter->first == OPERATOR) 
        {
            if (operandStack.size()>=2)
            {
                double operand1 = operandStack.top();
                operandStack.pop();
                double operand2 = operandStack.top();
                operandStack.pop();
                operandStack.push(operate(iter->second, operand1, operand2));
            } else
            {
                //Missing operand error
                isError = true;
            }
        }
        else if (iter->first == OPERAND) 
        {
            operandStack.push(stringToDouble(iter->second));
        }
    }
    return operandStack.top();
}

//TODO: try to refactor the 2 evaluate function, and set the return value to error message
// Get result in string
//std::string Expression::evaluateToString(void) 
//{
//    std::string result;
//    result = toString(evaluate());
//    if (!isError)
//    {
//        return result;
//    } else
//    {
//        return "err";
//    }
//}
int Expression::evaluate(std::string& output) 
{
    output = toString(compute());
    if (isError)
    {
        return 1;
    } else
    {
        return 0;
    }
}
// Get result in char array
int Expression::evaluate(char** output)
{
    std::string temp;
    temp = toString(compute());
    *output = new char [temp.size()+1];
    strcpy(*output,temp.c_str());

    if (isError)
    {
        return 1;
    } else
    {
        return 0;
    }

    
}

void Expression::findAndReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos)
    {
       str.replace(pos, oldStr.length(), newStr);
       pos += newStr.length();
    }
}

void Expression::findAndReplace2(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos)    // npos means "end of string"
    {
        
        if (pos != 0)
        {
            if (!isDigit(str[pos-1]))
            {
                str.replace(pos, oldStr.length(), newStr);
                pos += newStr.length();
            } else
            {
                pos += 1;
            }
        } else
        {
            str.replace(pos, oldStr.length(), newStr);
            pos += newStr.length();
        }
    }
}