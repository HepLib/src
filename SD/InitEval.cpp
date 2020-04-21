/**
 * @file
 * @brief Functions for Initialize and Evaluate
 * @author F. Feng
 * @version 1.0.0
 * @date 2020-04-21
 */
 
#include "SD.h"
#include <math.h>
#include <cmath>

namespace HepLib::SD {

    SecDec::~SecDec() {
        if(SecDec!=NULL) delete SecDec;
        if(Integrator!=NULL) delete Integrator;
        if(Minimizer!=NULL) delete Minimizer;
        SecDec = NULL;
        Integrator = NULL;
        Minimizer = NULL;
    }

    void Replacements2(exmap &repl) {
        auto tmp = repl;
        for(auto &kv : repl) {
            kv.second = kv.second.subs(tmp, subs_options::algebraic);
        }
    }

    void SecDec::Initialize(FeynmanParameter fp) {
            
        if(fp.Propagators.nops() != fp.Exponents.nops()) {
            cerr << Color_Error << "Initialize: the length of Propagators and Exponents are NOT equal." << RESET << endl;
            exit(1);
        }
        
        if(fp.Prefactor.is_zero()) {
            IsZero = true;
            return;
        }
        
        Digits = 50;
        IsZero = false;
        
        for(auto kv: fp.lReplacements) {
            if((lst{kv.first, kv.second}).has(iEpsilon)) {
                cerr << Color_Error << "Initialize: (lst{kv.first, kv.second}).has(iEpsilon) /1" << RESET << endl;
                exit(1);
            }
        }
        for(auto kv: fp.tReplacements) {
            if((lst{kv.first, kv.second}).has(iEpsilon)) {
                cerr << Color_Error << "Initialize: (lst{kv.first, kv.second}).has(iEpsilon) /2" << RESET << endl;
                exit(1);
            }
        }
        for(auto kv: fp.nReplacements) {
            if((lst{kv.first, kv.second}).has(iEpsilon)) {
                cerr << Color_Error << "Initialize: (lst{kv.first, kv.second}).has(iEpsilon) /3" << RESET << endl;
                exit(1);
            }
        }
        
        auto sop = subs_options::algebraic;
        
        auto ps = fp.Propagators;
        auto ns = fp.Exponents;
        
        auto ls = fp.LoopMomenta;
        auto tls = fp.tLoopMomenta;
        
        for(auto item : ls) {
            if(!is_a<symbol>(item)) throw Error("SecDec::Initialize failed, NOT a symbol: "+ex2str(item));
        }
        for(auto item : tls) {
            if(!is_a<symbol>(item)) throw Error("SecDec::Initialize failed, NOT a symbol: "+ex2str(item));
        }
        
        Replacements2(fp.lReplacements);
        Replacements2(fp.tReplacements);
        Replacements2(fp.nReplacements);
        
        auto lsubs = fp.lReplacements;
        auto tsubs = fp.tReplacements;
        auto nsubs = fp.nReplacements;
        nReplacements = fp.nReplacements;
        
        if(Verbose > 1) cout << now() << " - Initialize ..." << endl;
        
        ex asgn = 1;
        ex a = 0;
        ex ad = (4-2*ep)*ls.nops();
        if(fp.isQuasi) ad += (3-2*ep)*tls.nops();
        else ad += (2-2*ep)*tls.nops();
        int xn = ps.nops();
        ex rem = 0;
        exmap xtNeg;
        
        ex pre = fp.Prefactor; // come from below
        for(int i=0; i<ps.nops(); i++) {
            bool ltQ = false; {
                auto tps = ps.op(i).expand().subs(lsubs,sop).subs(tsubs,sop);
                for(auto lsi : ls) {
                    if(tps.has(lsi)) {
                        ltQ = true;
                        break;
                    }
                }
                
                if(!ltQ) {
                    for(auto lsi : tls) {
                        if(tps.has(lsi)) {
                            ltQ = true;
                            break;
                        }
                    }
                }
            }
            
            if(ltQ) a += ns.op(i);
            ex sgn = 0;
            
            if(!ltQ) {
                pre = pre * pow(ps.op(i).expand().subs(lsubs,sop).subs(tsubs,sop), ex(0)-ns.op(i));
                ns.let_op(i) = 0;
                ps.let_op(i) = 1;
                continue;
            } else if(is_a<numeric>(ns.op(i)) && (ns.op(i)<=0)) {
                xtNeg[x(i)]=0;
                if(ns.op(i)<0) {
                    asgn *= pow(-1, ns.op(i));
                    rem += x(i) * ps.op(i).subs(iEpsilon==0);
                }
                continue;
            }

            auto p = ps.op(i).expand().subs(lsubs,sop).subs(tsubs,sop).subs(nsubs);
            p = p.subs(lsubs,sop).subs(tsubs,sop).subs(nsubs);
            // check loop^2
            for(auto m : ls) {
                if(!is_a<numeric>(p.coeff(m,2))) {
                    cout << Color_Error << "not numeric: " << p.coeff(m,2) << endl;
                    cout << "nsubs = " << nsubs << RESET << endl;
                    exit(1);
                }
                numeric nm = ex_to<numeric>(p.coeff(m,2));
                if(nm.is_zero()) continue;
                sgn = nm>0 ? -1 : 1;
                break;
            }
            // check iEpsilon
            if(sgn.is_zero()) {
                if(!is_a<numeric>(p.coeff(iEpsilon))) {
                    cerr << Color_Error << "Initialize: (!is_a<numeric>(p.coeff(iEpsilon)))" << RESET << endl;
                    exit(1);
                }
                numeric nm = ex_to<numeric>(p.coeff(iEpsilon));
                if(!nm.is_zero()) sgn = nm>0 ? -1 : 1;
            }
            // check tloop^2
            if(sgn.is_zero()) {
                for(auto m : tls) {
                    if(!is_a<numeric>(p.coeff(m,2))) {
                        cerr << Color_Error << "not numeric: " << p.coeff(m,2) << RESET << endl;
                        exit(1);
                    }
                    numeric nm = ex_to<numeric>(p.coeff(m,2));
                    if(nm.is_zero()) continue;
                    sgn = nm>0 ? -1 : 1;
                    break;
                }
            }
            // others
            if(sgn.is_zero()) {
                sgn = 1;
                if(is_a<numeric>(p) && ex_to<numeric>(p)>0) sgn = -1;
                if(Verbose>0) {
                    cout << Color_Warn << " - Warning: Can NOT determine the iEpsilon sign." << RESET << endl;
                    cout << " - " << p << " from " << ps.op(i) << endl;
                }
            }
            
            p = (ps.op(i)*sgn).subs(iEpsilon==0);
            if(sgn==-1) asgn *= exp(I * Pi * ns.op(i));
            rem += x(i) * p;
        }

        rem = rem.expand();
        lst uList1, uList2;
        ex u=1, cu=1;
        
        // Loop
        if(ls.nops()>0) {
            u=1;
            for(int i=0; i<ls.nops(); i++) {
                auto t2 = rem.coeff(ls.op(i),2);
                auto t1 = rem.coeff(ls.op(i),1);
                auto t0 = rem.coeff(ls.op(i),0);
                u *= (-t2);
                if(t2==0) {
                    IsZero = true;
                    return;
                }
                rem = expand(t0 - pow(t1,2)/(4*t2));
            }
            rem = normal(rem.subs(lsubs,sop).subs(lsubs,sop));
            u = normal(u.subs(lsubs,sop));
            for(auto m: tls) {
                if(u.has(m)) {
                    cerr << Color_Error << "Initialize: u.has(m), " << u << ", " << m << RESET << endl;
                    exit(1);
                }
            }
            
            cu *= u;
            auto u_nd = numer_denom(u);
            ex usgn = u_nd.op(1).subs(xtNeg).subs(x(w)==ex(1)/2).subs(nsubs);
            if(usgn.is_zero()) usgn = u_nd.op(1).subs(xtNeg).subs(x(w)==ex(1)/3).subs(nsubs);
            if(usgn.is_zero()) {
                cerr << Color_Error << "Initialize: (usgn.is_zero())" << RESET << endl;
                exit(1);
            }
            usgn = normal(usgn)>0 ? 1 : -1;
            
            if(!xPositive(normal(usgn*u_nd.op(0)).subs(xtNeg).subs(nReplacements).subs(lst{
                CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
            }))) {
                cerr << Color_Error << "Initialize: NOT positive (1) - un: " << normal(usgn*u_nd.op(0)).subs(xtNeg).subs(nReplacements).subs(lst{
                    CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
                }) << RESET << endl;
                exit(1);
            }
            if(!xPositive(normal(usgn*u_nd.op(1)).subs(xtNeg).subs(nReplacements).subs(lst{
                CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
            }))) {
                cerr << Color_Error << "Initialize: NOT positive - ud: " << normal(usgn*u_nd.op(1)).subs(xtNeg).subs(nReplacements).subs(lst{
                    CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
                }) << RESET << endl;
                exit(1);
            }
            
            uList1.append(usgn*u_nd.op(0));
            uList2.append(-(4-2*ep)/2);
            if((usgn*u_nd.op(0)) != 1) {
                uList1.append(usgn*u_nd.op(1));
                uList2.append((4-2*ep)/2);
            }
        } else {
            rem = normal(rem.subs(lsubs,sop).subs(lsubs,sop));
        }
        
        // t-Loop
        if(tls.nops()>0) {
            u=1;
            for(int i=0; i<tls.nops(); i++) {
                auto t2 = rem.coeff(tls.op(i),2);
                auto t1 = rem.coeff(tls.op(i),1);
                auto t0 = rem.coeff(tls.op(i),0);
                u *= (-t2);
                if(t2.is_zero()) {
                    IsZero = true;
                    return;
                }
                rem = expand(t0 - pow(t1,2)/(4*t2));
            }
            rem = normal(rem.subs(tsubs,sop));
            u = normal(u.subs(lsubs,sop));
            for(auto m: tls) {
                if(u.has(m)) {
                    cerr << Color_Error << "Initialize: u.has(m), " << u << ", " << m << RESET << endl;
                    exit(1);
                }
            }
            
            cu *= u;
            auto u_nd = numer_denom(u);
            ex usgn = u_nd.op(1).subs(xtNeg).subs(x(w)==ex(1)/2).subs(nsubs);
            if(usgn.is_zero()) usgn = u_nd.op(1).subs(xtNeg).subs(x(w)==ex(1)/3).subs(nsubs);
            if(usgn.is_zero()) {
                cerr << Color_Error << "Initialize: (usgn.is_zero())" << RESET << endl;
                exit(1);
            }
            usgn = normal(usgn)>0 ? 1 : -1;
            
            if(!xPositive(normal(usgn*u_nd.op(0)).subs(xtNeg).subs(nReplacements).subs(lst{
                CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
            }))) {
                cerr << Color_Error << "Initialize: NOT positive (2) - un: " << normal(usgn*u_nd.op(0)).subs(xtNeg).subs(nReplacements).subs(lst{
                    CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
                }) << RESET << endl;
                exit(1);
            }
            if(!xPositive(normal(usgn*u_nd.op(1)).subs(xtNeg).subs(nReplacements).subs(lst{
                CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
            }))) {
                cerr << Color_Error << "Initialize: NOT positive - ud: " << normal(usgn*u_nd.op(1)).subs(xtNeg).subs(nReplacements).subs(lst{
                    CV(w1,w2)==w2, ep==ex(1)/111, eps==ex(1)/1111
                }) << RESET << endl;
                exit(1);
            }
            
            uList1.append(usgn*u_nd.op(0));
            if(fp.isQuasi) uList2.append(-(3-2*ep)/2);
            else uList2.append(-(2-2*ep)/2);
            if(usgn*u_nd.op(1) != 1) {
                uList1.append(usgn*u_nd.op(1));
                if(fp.isQuasi) uList2.append((3-2*ep)/2);
                else uList2.append((2-2*ep)/2);
            }
        }

        u = normal(cu);
        auto u_nd = numer_denom(u);
        rem = normal(rem * u);
        auto rem_nd = numer_denom(rem);
        
        ex usgn = u_nd.op(1).subs(xtNeg).subs(x(w)==ex(1)/2).subs(nsubs);
        usgn = normal(usgn)>0 ? 1 : -1;
        ex fsgn = rem_nd.op(1).subs(xtNeg).subs(x(w)==ex(1)/2).subs(nsubs);
        fsgn = normal(fsgn)>0 ? 1 : -1;
        
        lst fList1, fList2;
        fList1.append(usgn*u_nd.op(0));
        fList2.append(a-ad/2);
        fList1.append(fsgn*rem_nd.op(0));
        fList2.append(-a+ad/2);
        if(usgn*u_nd.op(1) != 1) {
            fList1.append(usgn*u_nd.op(1));
            fList2.append(-a+ad/2);
        }
        if(fsgn*rem_nd.op(1) != 1) {
            fList1.append(fsgn*rem_nd.op(1));
            fList2.append(a-ad/2);
        }
        
        for(int i=0; i<uList1.nops(); i++) {
            fList1.append(uList1[i]);
            fList2.append(uList2[i]);
        }

        vector<ex> ret;
        ret.push_back(lst{fList1, fList2});

        // negative index
        for(int i=0; i<xn; i++) {
        if(is_a<numeric>(ns.op(i)) && ns.op(i)<0) {
            if(!ex_to<numeric>(ex(0)-ns.op(i)).is_pos_integer()) {
                cerr << Color_Error << "Initialize: (!ex_to<numeric>(ex(0)-ns.op(i)).is_pos_integer())" << RESET << endl;
                exit(1);
            }
            for(int j=0; j<-ns.op(i); j++) {
                vector<ex> nret;
                for(auto fe : ret) {
                    auto plst = ex_to<lst>(fe.op(0));
                    auto nlst = ex_to<lst>(fe.op(1));
                    for(int ij=0; ij<nlst.nops(); ij++) {
                        auto dtmp = nlst.op(ij) * mma_diff(plst.op(ij),x(i),1,false);
                        if(dtmp.is_zero()) continue;
                        auto plst2 = plst;
                        auto nlst2 = nlst;
                        if((nlst.op(ij)-1).is_zero()) {
                            plst2.let_op(ij) = dtmp;
                        } else {
                            nlst2.let_op(ij) = nlst.op(ij)-1;
                            int nn = plst.nops();
                            if(!(nlst.op(nn-1)-1).is_zero()) {
                                plst2.append(dtmp);
                                nlst2.append(1);
                            } else plst2.let_op(nn-1) = plst.op(nn-1) * dtmp;
                        }
                        nret.push_back(lst{plst2, nlst2});
                    }
                }
                ret = nret;
            }
            
            for(auto &fe : ret) {
                fe = ex_to<lst>(subs(fe, x(i)==0));
            }
        }}

        // simplification
        // ex pre = fp.Prefactor; // moved to above
        pre *= asgn * pow(I,ls.nops()+(fp.isQuasi ? tls.nops() : 0)) * pow(Pi, ad/2) * tgamma(a-ad/2);
        for(int i=0; i<ns.nops(); i++) {
            if(is_a<numeric>(ns.op(i)) && ns.op(i)<=0) continue;
            pre /= tgamma(ns.op(i));
        }
        if(tls.nops()>0 && (!fp.isQuasi)) pre *= exp(I * Pi * tls.nops()*(2-2*ep)/2);
        
        ex xpre = 1;
        for(int i=0; i<ns.nops(); i++) {
            if(is_a<numeric>(ns.op(i)) && ns.op(i)<=1) continue;
            else {
                for(auto &fe : ret) {
                    let_op_append(fe, 0, x(i));
                    let_op_append(fe, 1, ns.op(i)-1);
                 }
            }
        }

        for(auto &fe : ret) {
            let_op_append(fe, 0, pre);
            let_op_append(fe, 1, 1);
            if(xpre != 1) {
                let_op_append(fe, 0, xpre);
                let_op_append(fe, 1, 1);
            }
            auto nnn = fe.op(0).nops();
            for(int j=0; j<nnn; j++) {
                fe.let_op(0).let_op(j) = collect_common_factors(fe.op(0).op(j));
                fe.let_op(1).let_op(j) = collect_common_factors(fe.op(1).op(j));
            }
        }

        lst delta;
        for(int i=0; i<ns.nops(); i++) {
            if(is_a<numeric>(ns.op(i)) && ns.op(i)<=0) continue;
            delta.append(x(i));
        }
        for(auto &fe : ret) let_op_append(fe, lst{delta});
        FunExp = ret;
        
        GiNaC_archive_Symbols_from(FunExp);
        Normalizes();
        if(fp.isAsy) DoAsy();
        XReOrders();
        Normalizes();
    }
    
    void SecDec::Initialize(XIntegrand xint) {
        
        Digits = 50;
        IsZero = false;
        Replacements2(xint.nReplacements);
        nReplacements = xint.nReplacements;
        
        for(int di=0; di<xint.Deltas.nops(); di++) {
            auto delta = xint.Deltas.op(di);
            if(!is_a<lst>(delta) || delta.nops()<1) {
                cout << Color_Error << "Deltas is NOT valide: " << xint.Deltas << RESET << endl;
                exit(1);
            }
        }

        FunExp.clear();
        FunExp.shrink_to_fit();
        if(xint.Deltas.nops()>0) FunExp.push_back(lst{xint.Functions, xint.Exponents, xint.Deltas});
        else FunExp.push_back(lst{xint.Functions, xint.Exponents});
        
        GiNaC_archive_Symbols_from(FunExp);
        Normalizes();
        if(xint.isAsy) DoAsy();
        XReOrders();
        Normalizes();
    }
    
    void SecDec::Evaluate(FeynmanParameter fp, const string & key) {
        
        if(Verbose>1) cout << endl << "Starting @ " << now() << endl;
        if(CFLAGS=="") CFLAGS = getenv("SD_CFLAGS");
        
        Initialize(fp);
        MB();
        if(FunExp.size()<1) return;
        GiNaC_archive_Symbols_from(FunExp);
        Scalelesses();
        ChengWu();
        RemoveDeltas();
        KillPowers();
        SDPrepares();
        EpsEpExpands();
        CIPrepares(key);
        auto pps = ParallelProcess;
        ParallelProcess = 0;
        Contours(key);
        Integrates(key);
        ParallelProcess = pps;
        if(Verbose>1) cout << "Finished @ " << now() << endl << endl;
    }

    void SecDec::Evaluate(XIntegrand xint, const string & key) {
        
        if(Verbose>1) cout << endl << "Starting @ " << now() << endl;
        if(CFLAGS=="") CFLAGS = getenv("SD_CFLAGS");
        
        Initialize(xint);
        MB();
        if(FunExp.size()<1) return;
        GiNaC_archive_Symbols_from(FunExp);
        Scalelesses();
        ChengWu();
        RemoveDeltas();
        KillPowers();
        SDPrepares();
        EpsEpExpands();
        CIPrepares(key);
        auto pps = ParallelProcess;
        ParallelProcess = 0;
        Contours(key);
        Integrates(key);
        ParallelProcess = pps;
        if(Verbose>1) cout << "Finished @ " << now() << endl << endl;
    }
    
    void SecDec::Evaluate(vector<ex> funexp, const string & key) {
        
        if(Verbose>1) cout << endl << "Starting @ " << now() << endl;
        if(CFLAGS=="") CFLAGS = getenv("SD_CFLAGS");
        
        FunExp = funexp;
        MB();
        if(FunExp.size()<1) return;
        GiNaC_archive_Symbols_from(FunExp);
        Scalelesses();
        ChengWu();
        RemoveDeltas();
        KillPowers();
        SDPrepares();
        EpsEpExpands();
        CIPrepares(key);
        auto pps = ParallelProcess;
        ParallelProcess = 0;
        Contours(key);
        Integrates(key);
        ParallelProcess = pps;
        if(Verbose>1) cout << "Finished @ " << now() << endl << endl;
    }
    
}
