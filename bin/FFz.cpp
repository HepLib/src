#include "SD.h"

using namespace HepLib;

auto ep = SD::ep;
auto eps = SD::eps;
auto iEpsilon = SD::iEpsilon;

lst para_sym = lst {ep, eps, iEpsilon};
int zn = 10;
ex z0 = 1/ex(7);
int verb = 1;

bool zeps = true;
ex zz = z(zn);
int zmod = 2;

const char* cm_path = "cm";
const char* SD_path = "zSD";

void Prepare(int idx) {
    
    symbol p("p"), n("n");
    symbol q1("q1"), q2("q2");
    symbol k1("k1"), k2("k2"), k3("k3");
    symbol K1("K1"), K2("K2"), K3("K3");
    
    FeynmanParameter fp;
    fp.LoopMomenta = lst{ };
    fp.tLoopMomenta = lst{ K1, K2 };
    ex S = 1; // phase space symmetry factor
    
    ex kp = 1;
    ex m = 1;
    ex m2=m*m;
    
    ex k1p = z(1)*(1-zz)*kp;
    ex k1m = (pow(K1,2))/(2*k1p);
    ex k2p = z(2)*(1-zz)*kp;
    ex k2m = (pow(K2,2))/(2*k2p);
    ex k3p = z(3)*(1-zz)*kp;
    ex k3m = (pow(K3,2))/(2*k3p);
    
    if(fp.tLoopMomenta.nops()<1) zz = 1;
    ex pp = zz/2*kp;
    ex pm = m2/(2*pp);
    
    symtab table;
    table["ep"] = ep;
    table["eps"] = eps;
    table["iEpsilon"] = iEpsilon;

    table["p"] = p;
    table["n"] = n;
    table["k1"] = k1;
    table["K1"] = K2;
    table["k2"] = k2;
    table["K2"] = K2;
    table["k3"] = k3;
    table["K3"] = K3;
    table["q1"] = q1;
    table["q2"] = q2;
    table["m"] = m;
    table["zz"] = zz;
    
    table["kp"] = kp;
    table["pp"] = pp;
    table["pm"] = pm;
    parser reader(table);

    ostringstream ifn;
    ifn << cm_path << "/" << idx << ".txt";
    ifstream in(ifn.str());
    
    auto cms = reader(in);
    
    fp.Prefactor = pow(2*Pi, (2*ep-4)*fp.LoopMomenta.nops()) * cms.op(2);
    fp.Propagators = ex_to<lst>(cms.op(0));
    fp.Exponents = ex_to<lst>(cms.op(1));
    
    ex NCS = -pow(zz,(1-2*ep))/(16*(2-2*ep)*kp*Pi);
    ex M = 2*m;
    ex nts = fp.tLoopMomenta.nops();
    ex psFactor;
    if(nts>0) psFactor = NCS*M/S*(-((pow(2,(2-nts-(3-2*ep)*nts))*pow(Pi,(1-(3-2*ep)*nts)))/(kp*(-1+zz))));
    else psFactor = NCS*M*4*Pi/kp;
    fp.Prefactor = fp.Prefactor * psFactor;
    
    fp.lReplacements[p*p] = m2;
    fp.lReplacements[n*n] = 0;
    fp.lReplacements[p*n] = pp;
    fp.lReplacements[k1*k1] = 0;
    fp.lReplacements[k2*k2] = 0;
    fp.lReplacements[k3*k3] = 0;
    fp.lReplacements[n*k1] = k1p;
    fp.lReplacements[n*k2] = k2p;
    fp.lReplacements[n*k3] = k3p;
    fp.lReplacements[p*k1] = k1p*pm+k1m*pp;
    fp.lReplacements[p*k2] = k2p*pm+k2m*pp;
    fp.lReplacements[p*k3] = k3p*pm+k3m*pp;
    fp.lReplacements[k1*k2] = k1p*k2m+k2p*k1m-K1*K2;
    fp.lReplacements[k1*k3] = k1p*k3m+k3p*k1m-K1*K3;
    fp.lReplacements[k3*k2] = k3p*k2m+k2p*k3m-K3*K2;
    
    fp.tReplacements[p*p] = m2;
    fp.tReplacements[n*n] = 0;
    fp.tReplacements[p*n] = pp;
    
    if(nts>1) fp.nReplacements[z(wild())] = ex(1)/nts;
    else if(nts>0) fp.nReplacements[z(wild())] = ex(1)/2;
    
    fp.nReplacements[ep] = ex(1)/11;
    fp.nReplacements[eps] = ex(1)/111;
    
    SD work;
    work.epN = 0;
    work.Verbose = verb;
    work.ParallelSymbols = para_sym;
    
    char *CFLAGS = getenv("SD_CFLAGS");
    work.CFLAGS = CFLAGS;
    //work.CheckF1 = true;

    work.Initialize(fp);
    if(zeps) {
        for(auto &kv : work.FunExp) {
            kv.first.append(z(zn));
            kv.second.append(eps);
            kv.first.append(1-z(zn));
            kv.second.append(eps);
        }
    }
    
    //zmod = 2
    if(zmod == 2) {
        
        auto funexp = work.FunExp;
        for(auto &kv : work.FunExp) {
            kv.first.let_op(0) = kv.first.op(0) * z0;
            for(auto i=0; i<kv.first.nops(); i++) kv.first.let_op(i) = kv.first.op(i).subs(zz==zz*z0);
            for(auto i=0; i<kv.second.nops(); i++) kv.second.let_op(i) = kv.second.op(i).subs(zz==zz*z0);
        }
        
        for(auto &kv : funexp) {
            kv.first.let_op(0) = kv.first.op(0) * (1-z0);
            for(auto i=0; i<kv.first.nops(); i++) kv.first.let_op(i) = kv.first.op(i).subs(zz==zz*(z0-1)+1);
            for(auto i=0; i<kv.second.nops(); i++) kv.second.let_op(i) = kv.second.op(i).subs(zz==zz*(z0-1)+1);
        }
        
        for(auto &item : funexp) work.FunExp.push_back(item);
    }
    
    if(work.IsZero) return;
    
    if(work.SecDec==NULL) work.SecDec = new SecDecG();
    if(work.Minimizer==NULL) work.Minimizer = new MinUit();
    
    int xn = work.Deltas[0].nops();
    exmap z2x;
    
    lst zs;
    ex zFactor = 1;
    for(int i=1; i<=nts; i++) {
        z2x[z(i)] = x(xn+i-1);
        zs.append(x(xn+i-1));
        zFactor /= x(xn+i-1);
    }
    z2x[z(zn)] = x(xn+zn);
    
    work.Deltas.push_back(zs);

    if(nts>0)
    for(auto &kv : work.FunExp) {
        
        kv.first.let_op(0) = kv.first.op(0) * zFactor;
        kv.first = lstHelper::subs(kv.first, z2x);
        
        auto tmp = collect_common_factors(kv.first.op(0));
        if(tmp.has(x(wild())) && is_a<mul>(tmp)) {
            ex rem = 1;
            for(auto item : tmp) {
                if(!item.has(x(wild()))) {
                    rem *= item;
                } else if(item.match(pow(wild(0), wild(1)))) {
                    kv.first.append(item.op(0));
                    kv.second.append(item.op(1));
                } else {
                    kv.first.append(item);
                    kv.second.append(1);
                }
            }
            kv.first.let_op(0) = rem;
        } else if(tmp.has(x(wild())) && tmp.match(pow(wild(0), wild(1)))) {
            kv.first.let_op(0) = 1;
            kv.first.append(tmp.op(0));
            kv.second.append(tmp.op(1));
        }
        
    }
    
    work.Normalizes();
    work.XReOrders();
    work.Normalizes();
    work.Scalelesses();
    work.RemoveDeltas();
    
    work.SDPrepares();
    work.EpsEpExpands();
    ostringstream ikey;
    ikey << SD_path << "/" << idx;
    work.CIPrepares(ikey.str().c_str());
    
    delete work.SecDec;
    delete work.Minimizer;
    
}

void Contour(int idx) {
    SD work;
    char *CFLAGS = getenv("SD_CFLAGS");
    work.CFLAGS = CFLAGS;
    work.ParallelSymbols = para_sym;
    work.Verbose = verb;
    
    ostringstream ikey;
    ikey << SD_path << "/" << idx;
    
    if(work.Minimizer==NULL) work.Minimizer = new MinUit();
    work.Contours(ikey.str().c_str());
    
    delete work.Minimizer;
}

ex Integrate(int idx) {
    SD work;
    char *CFLAGS = getenv("SD_CFLAGS");
    work.CFLAGS = CFLAGS;
    work.ParallelSymbols = para_sym;
    work.Verbose = verb;
    
    work.EpsAbs = 1E-7;
    work.RunPTS = 10000;
    work.RunMAX = 500;
    
    ostringstream ikey;
    ikey << SD_path << "/" << idx;
    
    if(work.Integrator==NULL) {
        auto intor = new HCubature();
        intor->use_last = false;
        work.Integrator = intor;
    }
    work.Integrates(ikey.str().c_str());
    
    delete work.Integrator;
    return work.ResultError;
}

int main(int argc, char** argv) {
    char* dc = getenv("SD_DLCLOSE");
    if(dc!=NULL && !strcmp(dc, "0")) SD::use_dlclose = false;
    
    const char* arg_a = NULL;
    const char* arg_n = NULL;
    const char* arg_v = "1";
    bool is_o = false;
    const char* arg_o = "";
    
    for (int opt; (opt = getopt(argc, argv, "a:n:o:v:")) != -1;) {
        switch (opt) {
            case 'a': arg_a = optarg; break;
            case 'n': arg_n = optarg; break;
            case 'v': arg_v = optarg; break;
            case 'o': arg_o = optarg; is_o = true; break;
            default:
                cout << "Not supported option!" << endl;
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    
    ostringstream cmd;
    cmd << "mkdir -p " << SD_path;
    system(cmd.str().c_str());
    cmd.clear();
    cmd.str("");
    cmd << "ls " << cm_path << "|wc -l";
    auto owc = RunOS(cmd.str().c_str());
    int nmi;
    stringstream ss;
    ss << owc;
    ss >> nmi;
    
    int in = 0;
    if(arg_n != NULL) in = stoi(arg_n);
    verb = stoi(arg_v);
    
    if(arg_a == NULL || !strcmp(arg_a, "p")) {
        vector<int> fid;
        for(int i=1; i<=nmi; i++) {
            if(in>0 && i!=in) continue;
            try {
                cout << "Preparing " << i << "/" << nmi << endl;
                Prepare(i);
            } catch(...) {
                fid.push_back(i);
            }
            cout << endl;
        }
        
        if(fid.size()>0) {
            cout << "Failed IDs: {";
            for(auto item : fid) cout << item << ", ";
            cout << "}" << endl;
            return 0;
        }
    }
    
    if(arg_a == NULL || !strcmp(arg_a, "c")) {
        for(int i=1; i<=nmi; i++) {
            if(in>0 && i!=in) continue;
            stringstream ss;
            ss << SD_path << "/" << i << ".ci.gar";
            if(!file_exists(ss.str().c_str())) continue;
            cout << "Contouring - " << i << "/" << nmi << endl;
            Contour(i);
            cout << endl;
        }
    }
    
    if(arg_a == NULL || !strcmp(arg_a, "i")) {
        ex fRes = 0;
        vector<int> nid;
        for(int i=1; i<=nmi; i++) {
            if(in>0 && i!=in) continue;
            stringstream ss;
            ss << SD_path << "/" << i << ".ci.gar";
            if(!file_exists(ss.str().c_str())) continue;
            cout << "Integrating - " << i << "/" << nmi << endl;
            auto res = Integrate(i);
            fRes += res;
            //if(!res.is_zero()) cout << VEResult(res) << endl;
            cout << endl;
            if(res.has(SD::NaN)) nid.push_back(i);
            
            auto err = res.subs(lst{VE(wild(1),wild(2))==wild(2)});
            if(abs(err.coeff(ep,0))>ex(1)/100) {
                cout << "n = " << i << endl;
                cout << res << endl << endl;
            }
        }
        
        if(nid.size()>0) {
            cout << "NaN IDs: {";
            for(auto item : nid) cout << item << ", ";
            cout << "}" << endl;
            return 0;
        }
    }
    
    if(arg_a == NULL || !strcmp(arg_a, "r")) {
        ex fRes = 0;
        vector<int> nid;
        for(int i=1; i<=nmi; i++) {
            if(in>0 && i!=in) continue;
            stringstream ss;
            ss << SD_path << "/" << i << ".ci.gar";
            if(!file_exists(ss.str().c_str())) continue;
            
            ss.clear();
            ss.str("");
            ss << SD_path << "/" << i << ".res.gar";
            auto res = garResult(ss.str().c_str(), para_sym);
            fRes += res;
            if(res.has(SD::NaN)) nid.push_back(i);
            
            auto err = res.subs(lst{VE(wild(1),wild(2))==wild(2)});
            if(abs(err.coeff(ep,0).coeff(eps,0))>ex(1)/100) {
                cout << RED;
                cout << "n = " << i << endl;
                cout << res << endl << endl;
                cout << RESET;
            }
        }
        
        if(nid.size()>0) {
            cout << "NaN IDs: {";
            for(auto item : nid) cout << item << ", ";
            cout << "}" << endl;
            return 0;
        }
        
        fRes = VESimplify(fRes, 0);
        cout << endl << "Final Result: " << endl;
        cout << VEResult(fRes) << endl;
        
        if(is_o) {
            ofstream ofs;
            ofs.open(arg_o, ios::out);
            if (!ofs) throw runtime_error("failed to open final out file!");
            ofs << fRes << endl;
            ofs.close();
        }
    }

    return 0;
}