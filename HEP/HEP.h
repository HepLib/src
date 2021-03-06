/**
 * @file 
 * @brief HEP header file
 */
 
#pragma once

#include "IBP.h"

namespace HepLib {

    using namespace std;
    using namespace GiNaC;
    using namespace HepLib;
    using namespace IBP;
    
    extern const Symbol NA;
    extern const Symbol NF;
    extern const Symbol gs;
    extern const Symbol as;
    extern const Symbol mu;
    extern const Symbol nL;
    extern const Symbol nH;
    extern exmap SP_map;
    
    extern int form_trace_mode;
    extern const int form_trace_auto;
    extern const int form_trace_all;
    extern const int form_trace_each_all;
    extern const int form_trace_each_each;
    
    extern int form_expand_mode;
    extern const int form_expand_none;
    extern const int form_expand_tr;
    extern const int form_expand_ci;
    extern const int form_expand_li;
    extern const int form_expand_all;
    
    extern bool Apart_using_cache;
    extern bool Apart_using_fermat;
    extern bool form_using_su3;
    extern bool form_using_dim4;
    
    class Index;
    class Vector;
    class Pair;
        
    /**
     * @brief class for FormFormat Output
     */
    class FormFormat : public print_dflt {
        GINAC_DECLARE_PRINT_CONTEXT(FormFormat, print_dflt)
    public:
        FormFormat(ostream &os, unsigned opt=0);
        static void power_print(const power & p, const FormFormat & c, unsigned level=0);
        
        template<class T> const FormFormat & operator << (const T & v) const {
            s << v;
            return *this;
        };
        const FormFormat & operator << (const basic & v) const;
        const FormFormat & operator << (const ex & v) const;
        const FormFormat & operator << (const lst & v) const;
        const FormFormat & operator<<(std::ostream& (*v)(std::ostream&)) const;
        
        #ifndef DOXYGEN_SKIP
        class _init {
            public: _init();
        };
    private:
        static _init FormFormat_init;
        #endif
    };
        
    /**
     * @brief class for FCFormat Output
     */
    class FCFormat : public print_dflt {
        GINAC_DECLARE_PRINT_CONTEXT(FCFormat, print_dflt)
    public:
        FCFormat(ostream &os, unsigned opt=0);
        static void add_print(const add & a, const FCFormat & c, unsigned level=0);
        static void mul_print(const mul & m, const FCFormat & c, unsigned level=0);
        static void ncmul_print(const ncmul & p, const FCFormat & c, unsigned level=0);
        
        template<class T> const FCFormat & operator << (const T & v) const {
            s << v;
            return *this;
        };
        const FCFormat & operator << (const basic & v) const;
        const FCFormat & operator << (const ex & v) const;
        const FCFormat & operator << (const lst & v) const;
        const FCFormat & operator<<(std::ostream& (*v)(std::ostream&)) const;
        
        const FCFormat & operator << (const matrix & v) const;
        const FCFormat & operator << (const exvector & v) const;
        const FCFormat & operator << (const exmap & v) const;
        const FCFormat & operator << (const exset & v) const;
        
        #ifndef DOXYGEN_SKIP
        class _init {
            public: _init();
        };
    private:
        static _init FCFormat_init;
        #endif
    };
    extern FCFormat fcout;
    
    /**
     * @brief class for index object
     */
    class Index : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(Index, basic)
    public:
        enum Type {VD, CF, CA};
        Index(const string &s, const Type type=Type::VD);
        Pair operator() (const Index & i);
        Pair operator() (const Vector & p);
        Symbol name;
        Type type;
        void print(const print_context &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static bool hasc(const ex &e);
        static bool hasv(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
    };
    GINAC_DECLARE_UNARCHIVER(Index);
    
    /**
     * @brief class for vector object
     */
    class Vector : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(Vector, basic)
    public:
        Vector(const string &s);
        Pair operator() (const Vector & p);
        Pair operator() (const Index & mu);
        Symbol name;
        void print(const print_context &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
    };
    GINAC_DECLARE_UNARCHIVER(Vector);
    
    /**
     * @brief class for SUNT object
     */
    class SUNT : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(SUNT, basic)
    public:
        SUNT(ex a, ex i, ex j);
        ex aij[3]; // Index
        size_t nops() const override;
        ex op(size_t i) const override;
        ex& let_op(size_t i) override;
        void form_print(const FormFormat &c, unsigned level = 0) const;
        void fc_print(const FCFormat &c, unsigned level = 0) const;
        void print(const print_dflt &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
        ex conjugate() const override;
    };
    GINAC_DECLARE_UNARCHIVER(SUNT);
    
    /**
     * @brief class for SUNF object
     */
    class SUNF : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(SUNF, basic)
    public:
        SUNF(ex i, ex j, ex k);
        ex ijk[3]; // Index
        size_t nops() const override;
        ex op(size_t i) const override;
        ex& let_op(size_t i) override;
        ex eval() const override;
        void print(const print_dflt &c, unsigned level = 0) const;
        void form_print(const FormFormat &c, unsigned level = 0) const;
        void fc_print(const FCFormat &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
    };
    GINAC_DECLARE_UNARCHIVER(SUNF);
    
    /**
     * @brief class for SUNF4 object
     */
    class SUNF4 : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(SUNF4, basic)
    public:
        SUNF4(ex i, ex j, ex k, ex l);
        ex ijkl[4]; // Index
        size_t nops() const override;
        ex op(size_t i) const override;
        ex& let_op(size_t i) override;
        ex eval() const override;
        void print(const print_dflt &c, unsigned level = 0) const;
        void form_print(const FormFormat &c, unsigned level = 0) const;
        void fc_print(const FCFormat &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
    };
    GINAC_DECLARE_UNARCHIVER(SUNF4);
    
    /**
     * @brief class for Pair object
     */
    class Pair : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(Pair, basic)
    public:
        Pair(const Vector &p1, const Vector &p2);
        Pair(const Index &i1, const Index &i2);
        Pair(const Vector &p, const Index &i);
        Pair(const Index &i, const Vector &p);
        size_t nops() const override;
        ex op(size_t i) const override;
        ex& let_op(size_t i) override;
        ex eval() const override;
        void print(const print_dflt &c, unsigned level = 0) const;
        void form_print(const FormFormat &c, unsigned level = 0) const;
        void fc_print(const FCFormat &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
    private:
        ex lr[2];
    };
    GINAC_DECLARE_UNARCHIVER(Pair);
    ex SP(const ex &a, bool use_map=false);
    ex SP(const ex &a, const ex &b, bool use_map=false);
    ex sp(const ex & a, const ex & b);
    ex sp(const ex & a);
    ex& letSP(const ex &p1, const ex &p2);
    ex& letSP(const ex &p);
    void clearSP(const ex &p1, const ex &p2);
    void clearSP(const ex &p);
    void clearSP();
    ex SP2sp(const ex & exin);
    exmap sp_map();
    
    /**
     * @brief class for Levi-Civita object
     * to make Tr(g5, g1, g2, g3, g4) is the same in both HepLib & FORM, require that
     * LC(a,b,c,d) = i_ * e_(a,b,c,d) ( we use the convention as in FeynCalc, Tr[5,1,2,3,4]=(- i) 4 LC[1,2,3,4])
     * LC is real in HepLib, while e_ is imaginary in FORM.
     */
    class Eps : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(Eps, basic)
    public:
        ex pis[4];
        Eps(const Vector &p1, const Vector &p2, const Vector &p3, const Vector &p4);
        Eps(const Vector &p1, const Vector &p2, const Vector &p3, const Index &i1);
        Eps(const Vector &p1, const Vector &p2, const Index &i1, const Index &i2);
        Eps(const Vector &p1, const Index &i1, const Index &i2, const Index &i3);
        Eps(const Index &i1, const Index &i2, const Index &i3, const Index &i4);
        Eps(vector<Vector> vs, vector<Index> is);
        size_t nops() const override;
        ex op(size_t i) const override;
        ex & let_op(size_t i) override;
        ex eval() const override;
        void print(const print_dflt &c, unsigned level = 0) const;
        void form_print(const FormFormat &c, unsigned level = 0) const;
        void fc_print(const FCFormat &c, unsigned level = 0) const;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
    };
    GINAC_DECLARE_UNARCHIVER(Eps);
    ex LC(ex pi1, ex pi2, ex pi3, ex pi4);
    
    /**
     * @brief class for Dirac Gamma object
     */
    class DGamma : public basic {
    GINAC_DECLARE_REGISTERED_CLASS(DGamma, basic)
    public:
        ex pi;
        unsigned rl;
        DGamma(const Vector &p, unsigned rl=0);
        DGamma(const Index &i, unsigned rl=0);
        DGamma(int int_1567, unsigned _rl=0);
        DGamma(const DGamma &g, unsigned _rl);
        void print(const print_dflt &c, unsigned level = 0) const;
        void form_print(const FormFormat &c, unsigned level = 0) const;
        void fc_print(const FCFormat &c, unsigned level = 0) const;
        return_type_t return_type_tinfo() const override;
        unsigned return_type() const override { return return_types::noncommutative; }
        bool match_same_type(const basic & other) const override;
        unsigned get_rl();
        size_t nops() const override;
        ex op(size_t i) const override;
        ex& let_op(size_t i) override;
        ex eval() const override;
        void archive(archive_node & n) const override;
        void read_archive(const archive_node& n, lst& sym_lst) override;
        static bool has(const ex &e);
        static lst all(const ex &e);
        ex derivative(const symbol & s) const override;
        ex conjugate() const override;
    };
    GINAC_DECLARE_UNARCHIVER(DGamma);
    
    //-----------------------------------------------------------
    // TR/GAS functions
    //-----------------------------------------------------------
    DECLARE_FUNCTION_3P(Matrix)
    DECLARE_FUNCTION_1P(TR)
    DECLARE_FUNCTION_1P(TTR)
    DECLARE_FUNCTION_1P(HF)
    
    inline ex GAS(const Vector &p, unsigned rl=0) { return DGamma(p,rl); }
    inline ex GAS(const Index &i, unsigned rl=0) { return DGamma(i,rl); }
    ex GAS(const ex &expr, unsigned rl=0);
    
    // Form, TIR, Apart
    ex charge_conjugate(const ex &);
    ex form(const ex &expr, int verb=0);
    ex TIR(const ex &expr_in, const lst &loop_ps, const lst &ext_ps);
    ex MatrixContract(const ex & expr_in);
    ex Apart(const ex &expr_in, const lst &vars, exmap sgnmap={});
    ex Apart(const ex &expr_in, const lst &loops, const lst & extmoms, exmap sgnmap={});
    ex ApartIR2ex(const ex & expr_in);
    ex ApartIR2F(const ex & expr_in);
    ex F2ex(const ex & expr_in);
    ex ApartIRC(const ex & expr_in);
    void ApartIBP(int IBPmethod, exvector &io_vec, const lst & loops, const lst & exts,
        const lst & cut_props=lst{}, std::function<lst(const Base &, const ex &)> uf=IBP::LoopUF);
        
    struct AIOption {
        lst Internal; // Internal for Apart/IBP
        lst External; // External for Apart/IBP
        lst DSP; // DSP for IBP
        exmap smap; // Sign Map for Apart
        lst Cuts; // Cut Propagators. optional
        lst CSP; // SP in Cuts, to be cleared. optional
        lst ISP; // SP for IBP. optional
        bool CutFirst = true;
        int mcl = 1; // collect_ex level, 0-nothing, 1-exnormal, 2-exfactor
        std::function<lst(const Base &, const ex &)> UF = IBP::LoopUF;
    };
    void ApartIBP(int IBPmethod, exvector &io_vec, AIOption aip);
    
    bool IsZero(const ex & e);
    
    #ifndef DOXYGEN_SKIP
    
    class ApartIR1_SERIAL { public: static unsigned serial; };
    template<typename T1>
    inline GiNaC::function ApartIR(const T1 & p1) {
        return GiNaC::function(ApartIR1_SERIAL::serial, ex(p1));
    }
    
    class ApartIR2_SERIAL { public: static unsigned serial; };
    template<typename T1, typename T2>
    inline GiNaC::function ApartIR(const T1 & p1, const T2 & p2) {
        return GiNaC::function(ApartIR2_SERIAL::serial, ex(p1), ex(p2));
    }
    
    #endif
        
        
}

