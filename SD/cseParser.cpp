/**
 * @file
 * @brief Functions for C++ code generation for Numerical Integration
 */
 
#include "SD.h"

namespace HepLib::SD {

int cseParser::on() { return no; }
vector<pair<int, ex>> cseParser::os() { return o_ex_vec;}

ex cseParser::Parse(ex expr, bool reset) {
    if(reset) {
        no = 0;
        o_ex_vec.clear();
    }
    
    if(is_a<numeric>(expr) || is_a<symbol>(expr) || is_a<symbol>(-expr) || isFunction(expr,"x") || isFunction(-expr,"x") || isFunction(expr,"PL") || isFunction(-expr,"PL")) return expr;
    if(!is_zero(ex_var_map[expr])) return ex_var_map[expr];
    
    ex ret = expr;
    if(is_a<add>(expr)) {
        ret = 0;
        for(auto item : expr) ret += Parse(item,false);
    } else if(is_a<lst>(expr)) {
        lst oret;
        for(auto item : expr) oret.append(Parse(item,false));
        return oret;
    } else if(is_a<mul>(expr)) {
        ret = 1;
        for(auto item : expr) ret *= Parse(item,false);
    } else if(is_a<power>(expr)) {
        ret = power(Parse(expr.op(0),false), expr.op(1));
    } else if(expr.match(log(w))) {
        ret = log(Parse(expr.op(0),false));
    } else if(expr.match(exp(w))) {
        ret = exp(Parse(expr.op(0),false));
    } else if(expr.match(sqrt(w))) {
        ret = sqrt(Parse(expr.op(0),false));
    } // else ret is expr
    stringstream ss;
    ss << oc << "[" << no << "]";
    symbol so(ss.str().c_str());
    o_ex_vec.push_back(make_pair(no, ret));
    no++;
    ex_var_map[expr] = so;
    return so;
}


}
