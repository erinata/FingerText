//This file is part of FingerText, a notepad++ snippet plugin.
//
//Large part of this file is based on the demo program eval.cpp
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

#ifndef EVALUATE_H
#define EVALUATE_H

#include <string>
#include <sstream>
#include <stack>
#include <vector>

#define OPERATOR 0
#define OPERAND 1

class Expression {
public: 
    Expression(char*);
    double evaluate();
    std::string evaluateToString();
    void evaluateToCharArray(char** output);

private: 
    // implementation of shunting yard algorithm require a stack for operator and a stack for operand
    std::stack<double> operandStack; 
    std::stack<char> operatorStack;             
    // The vaiable for holding  math expression in postfix form
    std::vector<std::pair<int, std::string>> postfix;
    std::string errorMessage;
    bool isDecimal;
    bool isError;

private:
    std::string charToString(const char &);      
    std::string doubleToString(const double &);     
    double stringToDouble(const std::string &); 
    bool isDigit(const char &c);
    int checkOperator(const char&); 
    void parseOperand(const double &);  
    void parseOperator(const char &);
    void processExpression(std::string input);
    std::string rephrasing(std::string input);
    void toPostfix(std::string infix);        
    double operate(const std::string &, const double &, const double &); // calculate result by operator and operand
    void findAndReplace(std::string& str, const std::string& oldStr, const std::string& newStr);
    int factorial(int number);
};

#endif