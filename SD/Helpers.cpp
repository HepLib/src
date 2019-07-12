#include "SD.h"

namespace HepLib {

/*********************************************************/
// Global Functions
/*********************************************************/

vector<ex> get_xy_from(ex pol) {
    exset xyset;
    bool ok = pol.find(x(wild()), xyset);
    if(!ok) {
        ok = pol.find(y(wild()), xyset);
        if(!ok) {
            vector<ex> xys(0);
            return xys;
        }
    }
    vector<ex> xys(xyset.size());
    copy(xyset.begin(), xyset.end(), xys.begin());
    sort(xys.begin(), xys.end(), [&](const auto &a, const auto &b){
        return ex_to<numeric>(normal((b-a)).subs(lst{ x(wild())==wild(), y(wild())==wild() })).is_positive();
    });
    return xys;
}

vector<ex> get_x_from(ex pol) {
    exset xset;
    bool ok = pol.find(x(wild()), xset);
    if(!ok) {
        vector<ex> xs(0);
        return xs;
    }
    vector<ex> xs(xset.size());
    copy(xset.begin(), xset.end(), xs.begin());
    sort(xs.begin(), xs.end(), [&](const auto &a, const auto &b){
        return ex_to<numeric>(normal((b-a)).subs(lst{ x(wild())==wild() })).is_positive();
    });
    return xs;
}

vector<ex> get_y_from(ex pol) {
    exset yset;
    bool ok = pol.find(y(wild()), yset);
    if(!ok) {
        vector<ex> ys(0);
        return ys;
    }
    vector<ex> ys(yset.size());
    copy(yset.begin(), yset.end(), ys.begin());
    sort(ys.begin(), ys.end(), [&](const auto &a, const auto &b){
        return ex_to<numeric>(normal((b-a)).subs(lst{ y(wild())==wild() })).is_positive();
    });
    return ys;
}

vector<ex> get_pl_from(ex pol) {
    exset plset;
    bool ok = pol.find(PL(wild()), plset);
    if(!ok) {
        vector<ex> pls(0);
        return pls;
    }
    vector<ex> pls(plset.size());
    copy(plset.begin(), plset.end(), pls.begin());
    sort(pls.begin(), pls.end(), [&](const auto &a, const auto &b){
        return ex_to<numeric>(normal((b-a)).subs(lst{ PL(wild())==wild() })).is_positive();
    });
    return pls;
}

vector<pair<lst, lst>> diff_wrt(const pair<lst, lst> &input, ex xi) {
    symbol x;
    vector<pair<lst, lst>> ret;
    auto plst = input.first;
    auto nlst = input.second;
    for(int i=0; i<nlst.nops(); i++) {
        auto dtmp = plst.op(i).subs(xi==x).diff(x).subs(x==xi);
        if(dtmp.is_zero()) continue;
        auto p_tmp = plst;
        auto n_tmp = nlst;
        
        if((nlst[i]-1).is_zero()) {
            p_tmp.let_op(i) = dtmp;
        } else {
            p_tmp.append(nlst[i]*dtmp);
            n_tmp.append(1);
            n_tmp.let_op(i) = nlst[i]-1;
        }
        ret.push_back(make_pair(p_tmp, n_tmp));
    }
    return ret;
}

/*********************************************************/
// VE
/*********************************************************/
ex VESimplify(ex expr, int epN, int epsN) {
    Digits = 50;
    auto ep = SD::ep;
    auto eps = SD::eps;
    auto expr1 = expr.evalf();
    if(expr1.has(eps)) expr1 = series_to_poly(expr1.series(eps, epsN+2));
    expr1 = series_to_poly(expr1.series(ep,epN+2));
    expr1 = expr1.expand().collect(lst{eps,ep}, true);
    ex ret = 0;
    for(int si=expr1.ldegree(eps); si<=epsN; si++) {
    for(int i=expr1.ldegree(ep); i<=epN; i++) {
    
        auto ccRes = expr1.coeff(eps,si).coeff(ep,i).expand();
        if(is_a<add>(ccRes)) {
            auto tmp = ccRes;
            ccRes = 0;
            for(auto item : tmp) ccRes += VF(item);
        } else {
            ccRes = VF(ccRes);
        }
        
        ccRes = GiNaC_Replace(ccRes, VF(wild()), [&](auto vf) {
            ex exp = vf.op(0);
            ex ret;
            if(is_a<mul>(exp)) {
                ret = VF(1);
                for(auto ii : exp) {
                    if(is_a<numeric>(ii) || ii.match(VE(wild(1), wild(2)))) {
                        ret *= ii;
                    } else {
                        ret = ret.subs(lst{VF(wild())==VF(wild()*ii)});
                    }
                }
            } else if(is_a<numeric>(exp) || exp.match(VE(wild(1), wild(2)))) {
                ret = exp*VF(1);
            } else {
                ret =VF(exp);
            }
            return ret;
        });
        
        exset vfs;
        find(ccRes, VF(wild()),vfs);
        
        for(auto vf : vfs) {
            auto tmpIR = ccRes.coeff(vf);
            auto tmp = (tmpIR + tmpIR.conjugate())/2;
            while(true) {
                auto tmp1 = tmp.expand().subs(lst{
                    VE(wild(1), wild(2))+VE(wild(3), wild(4))+wild(5)==VE(wild(1)+wild(3), sqrt(wild(2)*wild(2)+wild(4)*wild(4)))+wild(5),
                    wild(1)*VE(wild(2), wild(3))==VE(wild(1)*wild(2), abs(wild(1)*wild(3)))
                    });
                if(tmp1==tmp) break;
                tmp = tmp1;
            }
            ret += tmp * pow(eps,si) * pow(ep,i) * vf;
            
            tmp = (tmpIR - tmpIR.conjugate())/(2*I);
            while(true) {
                auto tmp1 = tmp.expand().subs(lst{
                    VE(wild(1), wild(2))+VE(wild(3), wild(4))+wild(5)==VE(wild(1)+wild(3), sqrt(wild(2)*wild(2)+wild(4)*wild(4)))+wild(5),
                    wild(1)*VE(wild(2), wild(3))==VE(wild(1)*wild(2), abs(wild(1)*wild(3)))
                    });
                if(tmp1==tmp) break;
                tmp = tmp1;
            }
            ret += I * tmp * pow(eps,si) * pow(ep,i) * vf;
        }
    }}
        
    exset ves;
    ret = ret.subs(lst{VF(wild())==wild()});
    ret.find(VE(wild(1), wild(2)), ves);
    lst repl;
    for(auto it : ves) {
        auto v = it.op(0);
        if(abs(v.evalf()) < 1E-30) v = 0;
        auto e = it.op(1);
        if(abs(e.evalf()) < 1E-30) e = 0;
        if(v.is_zero() || e.is_zero()) repl.append(it == VE(v, e));
    }
    ret = ret.subs(repl).subs(lst{VE(0,0)==0});
    
    return ret.collect(lst{eps,ep}, true);
}

ex VEResult(ex expr) {
    return expr.subs(VE(wild(1),wild(2))==VEO(wild(1),wild(2)));
}

/*********************************************************/
// Functions used in GiNaC
/*********************************************************/
static ex None_Diff(const ex & x, unsigned diff_param) {return 0;}
static ex None_Diff2(const ex & x, const ex & y, unsigned diff_param) {return 0;}
static ex VE_Conjugate(const ex & x, const ex & y) { return VE(x,y).hold(); }

static void print_VEO(const ex & ex1, const ex & ex2, const print_context & c) {
    int digits = 30;
    if(!ex2.is_zero()) {
        auto ratio = ex_to<numeric>(abs(ex1/ex2));
        digits = floorq(logq(ratio.to_double())/logq(10.Q)) + 2;
        digits = digits > 1 ? digits : 1;
        digits = digits > 30 ? 30 : digits;
    }
    auto oDigits = Digits;
    Digits = digits;
    c.s << "(" << ex1.evalf();
    Digits = 2;
    c.s << " +- " << ex2.evalf() << ")";
    Digits = oDigits;
}

REGISTER_FUNCTION(fabs, dummy())
REGISTER_FUNCTION(x, dummy())
REGISTER_FUNCTION(y, dummy())
REGISTER_FUNCTION(z, dummy())
REGISTER_FUNCTION(PL, dummy())
REGISTER_FUNCTION(FTX, derivative_func(None_Diff2))
REGISTER_FUNCTION(CT, dummy())
REGISTER_FUNCTION(VE, conjugate_func(VE_Conjugate))
REGISTER_FUNCTION(VEO, print_func<print_dflt>(print_VEO))

}
