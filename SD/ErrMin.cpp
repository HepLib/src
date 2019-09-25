#include "SD.h"

namespace HepLib {

int ErrMin::Verbose;
unsigned int ErrMin::xsize;
IntegratorBase::SD_Type ErrMin::fp = NULL;
IntegratorBase::SD_Type ErrMin::fpQ = NULL;
IntegratorBase *ErrMin::Integrator = NULL;
MinimizeBase *ErrMin::miner = NULL;
qREAL *ErrMin::paras = NULL;
dREAL *ErrMin::lambda = NULL;
dREAL ErrMin::err_max;
dREAL ErrMin::hjRHO = 0.75;
dREAL ErrMin::err_min = -0.001;
long long ErrMin::MaxPTS = 500;
long long ErrMin::RunPTS = 0;
ex ErrMin::lastResErr;

dREAL ErrMin::IntError(int nvars, dREAL *las, dREAL *n1, dREAL *n2) {
    RunPTS++;
    qREAL qlas[nvars];
    for(int i=0; i<nvars; i++) qlas[i] = las[i];
    auto res = Integrator->Integrate(xsize, fp, fpQ, paras, qlas);
    if(res.has(SD::NaN)) return 1.E15;
    auto err = res.subs(VE(wild(0), wild(1))==wild(1));
    numeric nerr = numeric("1E15");
    try {
        nerr = ex_to<numeric>(abs(err).evalf());
        if(nerr > numeric("1E15")) nerr = numeric("1E15");
        if(err_max > nerr.to_double()) {
            auto diff = VESimplify(lastResErr - res);
            diff = diff.subs(VE(0,0)==0);
            exset ves;
            diff.find(VE(wild(0), wild(1)), ves);
            for(auto ve : ves) {
                if(abs(ve.op(0)) > ve.op(1)) return 1E15;
            }
            if(Verbose > 3) {
                cout << "\r                             \r";
                cout << WHITE << "     " << RunPTS << ": " << RESET;
                for(int i=0; i<nvars; i++) cout << las[i] << " ";
                cout << endl << "     " << res.subs(VE(0,0)==0) << endl;
            }
            err_max = nerr.to_double();
            for(int i=0; i<nvars; i++) lambda[i] = las[i];
            if(err_max<=err_min) {
                cout << "\r                             \r";
                cout << "     ------------------------------" << endl;
                miner->ForceStop();
                return 0.;
            }
        } else {
            if(Verbose > 3) {
                cout << "\r                             \r";
                cout << WHITE << "     [ " << RunPTS << " / " << MaxPTS << " ] ..." << RESET << flush;
            }
        }
    } catch(domain_error &ex) {
        throw ex;
    } catch(...) { }
    if(RunPTS>=MaxPTS) {
        cout << "\r                             \r";
        cout << "     ------------------------------" << endl;
        miner->ForceStop();
        return 0.;
    }
    return nerr.to_double();
}


}