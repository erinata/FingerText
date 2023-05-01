// DuckEval is originally developed as part of Fingertext.
// It will be released as a separate project later. 
// Currently there is no license for sharing and reusing.

#ifndef DUCKEVAL_H
#define DUCKEVAL_H

#include <string>
#include <sstream>
#include <stack>
#include <vector>
#include <ctime>        // For the time seed used to generate random number

#include "ConversionUtils.h"   // For the toDouble and toString


#define OPERATOR 0
#define OPERAND 1

int execDuckEval(std::string& output);

class Expression 
{
    public:
        Expression(std::string input);
        int Expression::evaluate(std::string& output);
        
    private: 
        std::stack<double> operandStack; 
        std::stack<char> operatorStack;             
        
        std::vector<std::pair<int, std::string>> postfix; // The vaiable for holding  math expression in postfix form
        //std::string errorMessage;
        bool isDecimal;
        bool isError;
    
    private:
        double Expression::compute();
        // Implement the following conversion functions if not using together with ConversionUtils
        //std::string Expression::toString(const char &);      
        //std::string Expression::toString(const double &);     
        //double Expression::toDouble(const std::string &); 
        bool Expression::isDigit(const char &c);
        int Expression::checkOperator(const char&); 
        void Expression::parseOperand(const double &);  
        void Expression::parseOperator(const char &);
        void Expression::processExpression(std::string input);
        std::string Expression::rephrasing(std::string input);
        void Expression::toPostfix(std::string infix);        
        double Expression::operate(const std::string &, const double &, const double &); // calculate result by operator and operand
        void Expression::signReplace(std::string& str, const std::string& oldStr, const std::string& newStr,bool negative = false);
        double Expression::factorial(double number);
        int Expression::isNotEqual(double operand1, double operand2);
        int Expression::isEqual(double operand1, double operand2);
        int Expression::isSmallerOrEqual(double operand1, double operand2);
        int Expression::isGreaterOrEqual(double operand1, double operand2);
        int Expression::isSmaller(double operand1, double operand2);
        int Expression::isGreater(double operand1, double operand2);
        int Expression::operateOr(double operand1, double operand2);
        int Expression::operateAnd(double operand1, double operand2);
        double Expression::ncr(double operand1, double operand2);
        double Expression::npr(double operand1, double operand2);
        long Expression::randomNumber(double operand1);
        double Expression::randomFraction(double operand1);
        
};


#endif //DUCKEVAL_H