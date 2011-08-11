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

//TODO: make a class to centralize all the type conversion
// convert char to string
std::string Expression::charToString(const char &c)
{
    std::stringstream ss;
    ss << c;
    return ss.str();
}

// convert double to string
std::string Expression::doubleToString(const double &d) {
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
        //TODO: p - combination, P - permutation, a - absolute value, 
        //TODO: r - round down, R - round up
        
        case 's' :
        case 'c' :
        case 't' :
        case 'l' :
        case 'S' :
        case 'C' :
        case 'T' :
        case 'L' :
        case '!' : return 5;
        case '^' : return 4; 
        case '%' : 
        case '*' :
        case '/' : return 3;
        case '+' :
        case '-' : return 2;
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

// parse operand to operandStack
void Expression::parseOperand(const double &operand) 
{
  postfix.push_back(make_pair(OPERAND, doubleToString(operand)));
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
                postfix.push_back(make_pair(OPERATOR, charToString(operatorStack.top())));
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
                postfix.push_back(make_pair(OPERATOR, charToString(operatorStack.top())));
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
    std::string infix = rephrasing(input);
    this->toPostfix(infix);  // convert infix to postfix
}

std::string Expression::rephrasing(std::string input)
{
    //TODO: cater lower case
    findAndReplace(input,"pi","3.141592654");
    findAndReplace(input,"!","!0");
    findAndReplace(input,"ln","0l");
    findAndReplace(input,"log","0L");
    findAndReplace(input,"sin","0s");
    findAndReplace(input,"asin","0S");
    findAndReplace(input,"cos","0c");
    findAndReplace(input,"acos","0C");
    findAndReplace(input,"tan","0t");
    findAndReplace(input,"atan","0T");

    return input;
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
    
    // If operand is not "", let operand to operandStack.
    if (operand != "") parseOperand(stringToDouble(operand));
    
    // If operatorStack is not empty, push it to postfix vector until operatorStack is empty.
    while(!operatorStack.empty()) 
    {
        postfix.push_back(make_pair(OPERATOR,charToString(operatorStack.top())));
        operatorStack.pop();
    }
}

// calculate result by operator and operand
double Expression::operate(const std::string &operation, const double &operand1, const double &operand2) 
{

    if (operation == "+") 
        return operand2 + operand1;
    else if (operation == "-") 
        return operand2 - operand1;
    else if (operation == "*")
        return operand2 * operand1;
    else if (operation == "/")
        return operand2 / operand1;
    else if (operation == "%")
        return static_cast<int>(operand2) % static_cast<int>(operand1);
    else if (operation == "^")
        return std::pow(operand2,operand1);
    else if (operation == "!")
        return factorial(static_cast<int>(operand2));
    else if (operation == "l")
        return log(operand1);
    else if (operation == "L")
        return log10(operand1);
    else if (operation == "s")
        return sin(operand1);
    else if (operation == "S")
        return asin(operand1);
    else if (operation == "c")
        return cos(operand1);
    else if (operation == "C")
        return acos(operand1);
    else if (operation == "t")
        return tan(operand1);
    else if (operation == "T")
        return atan(operand1);
    else
        return 0;
}

int Expression::factorial(int number)
{
    int result = 1;
    int i;
    for (i = number; i>0; i--)
    {
        result = result * i;
    }

    return result;



}

// get evaluation result
double Expression::evaluate(void) 
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

// Get result in string
std::string Expression::evaluateToString(void) 
{
    std::string result;
    result = doubleToString(evaluate());
    if (!isError)
    {
        return result;
    } else
    {
        return "error";
    }
}
// Get result in char array
void Expression::evaluateToCharArray(char** output)
{
    std::string temp;
    temp = doubleToString(evaluate());
    if (isError)
    {
        temp = "error";
    }

    *output = new char [temp.size()+1];
    strcpy(*output,temp.c_str());
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