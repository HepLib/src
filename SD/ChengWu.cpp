#include "SD.h"
#include <cmath>

namespace HepLib {

void SD::Projectivize(lst &fe, lst delta, ex xsum) {
    symbol s;
    lst sRepl;
    for(int j=0; j<delta.nops(); j++) sRepl.append(delta.op(j)==delta.op(j)*s);
    if(xsum.is_zero()) {
        for(auto xi : delta) xsum += xi;
    }
    
    ex over_all_sn = 0;
    int nnn = fe.op(0).nops();
    for(int i=0; i<nnn; i++) {
        if(!fe.op(0).op(i).has(x(wild()))) continue;
        auto tmp = expand(fe.op(0).op(i));
        auto sn = tmp.subs(sRepl).degree(s);
        over_all_sn += sn*fe.op(1).op(i);
        lst items;
        if(is_a<add>(tmp)) {
            for(auto ii : tmp) items.append(ii);
        } else {
            items.append(tmp);
        }
        tmp = 0;
        for(auto ii : items) {
            auto sni = ii.subs(sRepl).degree(s);
            if(sni!=sn) tmp += ii * pow(xsum, sn-sni);
            else tmp += ii;
        }
        fe.let_op(0).let_op(i) = tmp;
    }

    over_all_sn = normal(over_all_sn+delta.nops());
    if(!over_all_sn.is_zero()) {
        let_op_append(fe, 0, xsum);
        let_op_append(fe, 1, ex(0)-over_all_sn);
    }
    
}

void SD::Scalelize(lst &fe, ex xi, ex cyi) {
    auto nnn = fe.op(0).nops();
    auto yi = xi.subs(x(wild())==y(wild()));
    for(int i=0; i<nnn; i++) {
        if(!fe.op(0).op(i).has(x(wild()))) continue;
        auto tmp = fe.op(0).op(i).subs(xi==cyi*yi).subs(yi==xi);
        tmp = tmp.normal();
        tmp = tmp.numer_denom();
        if(tmp.op(1).subs(x(wild())==1)<0) {
            tmp.let_op(0) = ex(0)-tmp.op(0);
            tmp.let_op(1) = ex(0)-tmp.op(1);
        }
        fe.let_op(0).let_op(i) = tmp.op(0);
        if(tmp.op(1)!=1) {
            let_op_append(fe, 0, tmp.op(1));
            let_op_append(fe, 1, ex(0)-fe.op(1).op(i));
        }
    }
    ex det = cyi.normal();
    det = det.numer_denom();
    if(det.op(1).subs(x(wild())==1)<0) {
        det.let_op(0) = ex(0)-det.op(0);
        det.let_op(1) = ex(0)-det.op(1);
    }
    if(det.op(0)!=1) {
        let_op_append(fe, 0, det.op(0));
        let_op_append(fe, 1, 1);
    }
    if(det.op(1)!=1) {
        let_op_append(fe, 0, det.op(1));
        let_op_append(fe, 1, -1);
    }
}

vector<lst> SD::Binarize(lst fe, ex eqn) {
    auto xij = get_x_from(eqn);
    assert(xij.size()==2);
    ex xi = xij[0];
    ex xj = xij[1];
    ex ci = eqn.coeff(xi);
    ex cj = eqn.coeff(xj);
    assert((ci*xi+cj*xj-eqn).is_zero() && is_a<numeric>(ci * cj) && (ci*cj)<0);
    
    vector<lst> ret;
    ci = abs(ci);
    cj = abs(cj);
    symbol yi,yj;
    // Part I: ci xi-cj xj>0, i.e., xi>cj/ci xj
    auto f1 = fe.op(0);
    auto e1 = fe.op(1);
    ex c1 = cj/ci;
    for(int i=0; i<f1.nops(); i++) {
        f1.let_op(i) = f1.op(i).subs(lst{xi==c1*yj/(1+c1)+yi,xj==yj/(1+c1)}).subs(lst{yi==xi,yj==xj});
    }
    f1.let_op(0) = f1.op(0)/(1+c1); // Jaccobi
    ret.push_back(lst{f1,e1});
    
    // Part II: ci xi-cj xj<0, i.e., i.e., xj>ci/cj xi
    auto f2 = fe.op(0);
    auto e2 = fe.op(1);
    ex c2 = ci/cj;
    for(int i=0; i<f2.nops(); i++) {
        f2.let_op(i) = f2.op(i).subs(lst{xj==c2*yi/(1+c2)+yj,xi==yi/(1+c2)}).subs(lst{yi==xi,yj==xj});
    }
    f2.let_op(0) = f2.op(0)/(1+c2); // Jaccobi
    ret.push_back(lst{f2,e2});
    
    return ret;
}

bool SD::Partilize(ex f0, ex xs, lst &ret0) {
    ex f = f0;
    for(auto xi : xs) {
        lst ret;
        if(f.degree(xi)!=1) continue;
        auto cxi = f.coeff(xi);
        if(!xPositive(cxi) && !xPositive(ex(0)-cxi)) continue;
        if(cxi.subs(x(wild())==1)<0) cxi = ex(0)-cxi;
        ret.append(lst{xi, cxi});
        f = f.subs(xi==0);
        if(f.is_zero() || Partilize(f, xs, ret)) {
            for(int i=0; i<ret.nops(); i++) ret0.append(ret.op(i));
            return true;
        }
    }
    return false;
}

void SD::ChengWu() {
    bool repeat = false;
    for(auto &fe : FunExp) {
        if(fe.nops()<3) return;
        for(int di=0; di<fe.op(2).nops(); di++) {
            auto delta = ex_to<lst>(fe.op(2).op(di));
            Projectivize(fe, delta);
            auto ft = fe.op(0).op(1);
            if(xPositive(ft)) continue;
            ft = Factor(ft);
            while(true) {
                auto ft0 = ft;
                if(ft.match(pow(wild(1), wild(2)))) {
                    ft = ft.op(0);
                } else if(is_a<mul>(ft)) {
                    ex tmp = 1;
                    for(auto fti : ft) {
                        if(xPositive(fti)) continue;
                        tmp = tmp * fti;
                    }
                    ft = tmp;
                    if((ft-ft0).is_zero()) break;
                    continue;
                }
                break;
            }

            lst ret;
            bool ok = Partilize(ft, delta, ret);
            if(ok) {
                if(Verbose>10) cout << "  \\--" << WHITE << "Cheng-Wu @ size="  << ret.nops() << RESET << endl;
                            
                lst rm_xs;
                ex inv_det = 1;
                for(int i=ret.nops()-1; i>=0; i--) {
                    auto xi = ret.op(i).op(0);
                    rm_xs.append(xi);
                    auto yi = xi.subs(x(wild())==y(wild()));
                    auto s = ret.op(i).op(1);
                    inv_det *= s;
                    ret.let_op(i).let_op(1) = yi/s;
                    for(int j=i-1; j>=0; j--) {
                        ret.let_op(j) = ret.op(j).subs(xi==yi/s);
                    }
                }
                lst rep;
                for(auto ss : ret) {
                    rep.append(ss.op(0)==ss.op(1));
                }

                auto nnn = fe.op(0).nops();
                for(int i=0; i<nnn; i++) {
                    if(!fe.op(0).op(i).has(x(wild()))) continue;
                    auto tmp = normal(fe.op(0).op(i).subs(rep));
                    tmp = tmp.subs(y(wild())==x(wild()));
                    auto num_den = numer_denom(tmp);
                    if(num_den.op(1).subs(x(wild())==1)<0) {
                        num_den.let_op(0) = ex(0)-num_den.op(0);
                        num_den.let_op(1) = ex(0)-num_den.op(1);
                    }
                    fe.let_op(0).let_op(i) = num_den.op(0);
                    if(num_den.op(1)!=1) {
                        let_op_append(fe, 0, num_den.op(1));
                        let_op_append(fe, 1, ex(0)-fe.op(1).op(i));
                    }
                }

                inv_det = normal(inv_det.subs(y(wild())==x(wild())));
                auto idet_num_den = numer_denom(inv_det);
                if(idet_num_den.op(1).subs(x(wild())==1)<0) {
                    idet_num_den.let_op(0) = ex(0)-idet_num_den.op(0);
                    idet_num_den.let_op(1) = ex(0)-idet_num_den.op(1);
                }
                if(idet_num_den.op(0)!=1) {
                    let_op_append(fe, 0, idet_num_den.op(0));
                    let_op_append(fe, 1, -1);
                }
                if(idet_num_den.op(1)!=1) {
                    let_op_append(fe, 0, idet_num_den.op(1));
                    let_op_append(fe, 1, 1);
                }

                ex re_xi;
                for(auto xi : delta) {
                    if(!rm_xs.has(xi)) {
                        re_xi = xi;
                        break;
                    }
                }
                Projectivize(fe, delta, re_xi);
            } else { // TODO: add more cases
                for(auto xi : delta) {
                    if(ft.degree(xi)!=1) continue;
                    auto cxi = ft.coeff(xi);
                    if(!xPositive(cxi) && !xPositive(ex(0)-cxi)) continue;
                    auto cft = ft.subs(xi==0);
                    if(!xPositive(cft) && !xPositive(ex(0)-cft)) continue;
                    
                    ex xi1 = 0, xi2 = 0;
                    for(int i=0; i<=delta.nops(); i++) {
                        if(!(xi-x(i)).is_zero() && xi2.is_zero()) xi2 = x(i);
                        else if(!delta.has(x(i)) && xi1.is_zero()) xi1 = x(i);
                        if(!xi1.is_zero() && !xi2.is_zero()) break;
                    }
                    if(xi1.is_zero() || xi2.is_zero()) continue;
                    
                    delta.append(xi1);
                    fe.let_op(2) = delta;
                    let_op_append(fe, 0, xi);
                    let_op_append(fe, 0, xi1+xi);
                    let_op_append(fe, 1, 1);
                    let_op_append(fe, 1, -2);
                    
                    Scalelize(fe, xi, 1/xi1);
                    Projectivize(fe, delta, xi2);
                    goto fe_loop_end;
                }
            }
        }
        fe_loop_end: ;
    }
    
    if(repeat) {
        ChengWu();
        return;
    }
    
    KillPowers();
}

}
