#include "FC.h"

namespace HepLib::FC {

    namespace {
        string di_("di");
        string li_("li");
        string ti_("ti");
        string fi_("fi");
        string ci_("ci");
        string ai_("ai");
        inline string n2s(ex fn) {
            int n = ex_to<numeric>(fn).to_int();
            return (n<0 ? "m" : "") + to_string(abs(n));
        }
        
        ex mat_conj(const ex & e1, const ex & e2, const ex & e3) {
            return Matrix(e1.conjugate(), e3, e2);
        }
    }

    REGISTER_FUNCTION(Propagator, do_not_evalf_params())
    REGISTER_FUNCTION(InField, do_not_evalf_params())
    REGISTER_FUNCTION(OutField, do_not_evalf_params())
    REGISTER_FUNCTION(Matrix, do_not_evalf_params().conjugate_func(mat_conj).set_return_type(return_types::commutative))
    
    unsigned Field2_SERIAL::serial = GiNaC::function::register_new(function_options("Field",2).do_not_evalf_params().overloaded(2));
    unsigned Field3_SERIAL::serial = GiNaC::function::register_new(function_options("Field",3).do_not_evalf_params().overloaded(2));
    unsigned Vertex2_SERIAL::serial = GiNaC::function::register_new(function_options("Vertex",2).do_not_evalf_params().overloaded(5));
    unsigned Vertex3_SERIAL::serial = GiNaC::function::register_new(function_options("Vertex",3).do_not_evalf_params().overloaded(5));
    unsigned Vertex4_SERIAL::serial = GiNaC::function::register_new(function_options("Vertex",4).do_not_evalf_params().overloaded(5));
    unsigned Vertex5_SERIAL::serial = GiNaC::function::register_new(function_options("Vertex",5).do_not_evalf_params().overloaded(5));
    unsigned Vertex6_SERIAL::serial = GiNaC::function::register_new(function_options("Vertex",6).do_not_evalf_params().overloaded(5));

    Index Qgraf::DI(ex fn) { return Index(di_+n2s(fn),Index::Type::VD); }
    Index Qgraf::LI(ex fn) { return Index(li_+n2s(fn),Index::Type::VD); }
    Index Qgraf::TI(ex fn) { return Index(ti_+n2s(fn),Index::Type::CF); }
    Index Qgraf::FI(ex fn) { return Index(fi_+n2s(fn),Index::Type::CF); }
    Index Qgraf::CI(ex fn) { return Index(ci_+n2s(fn),Index::Type::CA); }
    Index Qgraf::AI(ex fn) { return Index(ai_+n2s(fn),Index::Type::CA); }
    
    ex Qgraf::QuarkPropagator(ex e, ex m) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto mom = e.op(2);
        return I * SP(TI(fi1),TI(fi2)) * Matrix(GAS(mom)+GAS(1)*m, DI(fi1),DI(fi2)) / (SP(mom)-m*m);
    }
    
    ex Qgraf::GluonPropagator(ex e) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto mom = e.op(2);
        return (-I) * SP(CI(fi1),CI(fi2)) * SP(LI(fi1),LI(fi2)) / SP(mom); // Feynman Gauge
    }
    
    ex Qgraf::GhostPropagator(ex e) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto mom = e.op(2);
        return I * SP(CI(fi1),CI(fi2)) / SP(mom);
    }
    
    ex Qgraf::q2gVertex(ex e) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto fi3 = e.op(2).op(1);
        return I*gs*Matrix(GAS(LI(fi3)),DI(fi1),DI(fi2))*SUNT(TI(fi1),TI(fi2),CI(fi3));
    }
    
    ex Qgraf::g3Vertex(ex e) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto fi3 = e.op(2).op(1);
        auto mom1 = e.op(0).op(2);
        auto mom2 = e.op(1).op(2);
        auto mom3 = e.op(2).op(2);
        return gs*SUNF(CI(fi1),CI(fi2),CI(fi3))*(
            SP(mom1-mom2,LI(fi3))*SP(LI(fi1),LI(fi2)) +
            SP(mom2-mom3,LI(fi1))*SP(LI(fi2),LI(fi3)) +
            SP(mom3-mom1,LI(fi2))*SP(LI(fi3),LI(fi1))
        );
    }
    
    ex Qgraf::g4Vertex(ex e) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto fi3 = e.op(2).op(1);
        auto fi4 = e.op(3).op(1);
        return (-I)*gs*gs*(
            SUNF4(CI(fi1),CI(fi2),CI(fi3),CI(fi4))*(SP(LI(fi1),LI(fi3))*SP(LI(fi2),LI(fi4))-SP(LI(fi1),LI(fi4))*SP(LI(fi2),LI(fi3))) +
            SUNF4(CI(fi1),CI(fi3),CI(fi2),CI(fi4))*(SP(LI(fi1),LI(fi2))*SP(LI(fi3),LI(fi4))-SP(LI(fi1),LI(fi4))*SP(LI(fi2),LI(fi3))) +
            SUNF4(CI(fi1),CI(fi4),CI(fi2),CI(fi3))*(SP(LI(fi1),LI(fi2))*SP(LI(fi4),LI(fi3))-SP(LI(fi1),LI(fi3))*SP(LI(fi4),LI(fi2)))
        );
    }
    
    ex Qgraf::gh2gVertex(ex e) {
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto fi3 = e.op(2).op(1);
        auto mom1 = e.op(0).op(2);
        return -gs * SUNF(CI(fi1),CI(fi2),CI(fi3)) * SP(mom1,LI(fi3));
    }
    
    ex Qgraf::eikonalPropagator(ex e, ex n, int mode) { // 0 for gluon, others for quark/anti-quark
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto mom = e.op(2);
        if(mode==0) return I * SP(CI(fi1),CI(fi2)) / (SP(n,mom)+iEpsilon);
        else return I * Matrix(GAS(1), DI(fi1),DI(fi2)) * SP(TI(fi1),TI(fi2)) / (SP(n,mom)+iEpsilon);
    }
    ex Qgraf::eikonalPropagatorR(ex e, ex n, int mode) { // 0 for gluon, others for quark/anti-quark
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto mom = e.op(2);
        if(mode==0) return -I * SP(CI(fi1),CI(fi2)) / (SP(n,mom)+iEpsilon);
        else return -I * Matrix(GAS(1), DI(fi1),DI(fi2)) * SP(TI(fi1),TI(fi2)) / (SP(n,mom)-iEpsilon);
    }
    
    ex Qgraf::eikonalVertex(ex e, ex n, int mode) { // 0 for gluon, 1 for quark, 2 for anti-quark, in<0 & out>0
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto fi3 = e.op(2).op(1);
        auto mom1 = e.op(0).op(2);
        if(mode==0) return I * gs * SP(n,LI(fi3)) * Matrix(GAS(1), DI(fi1),DI(fi2)) * (-I*SUNF(CI(fi3),TI(fi1),TI(fi2)));
        else if(mode==1 || mode==-2) return I * gs * SP(n,LI(fi3)) * Matrix(GAS(1), DI(fi1),DI(fi2)) * (-SUNT(TI(fi2),TI(fi1),CI(fi3)));
        else if(mode==2 || mode==-1) return I * gs * SP(n,LI(fi3)) * Matrix(GAS(1), DI(fi1),DI(fi2)) * SUNT(TI(fi1),TI(fi2),CI(fi3));
        else return 0;
    }
    ex Qgraf::eikonalVertexR(ex e, ex n, int mode) { // 0 for gluon, 1 for quark, 2 for anti-quark, in<0 & out>0
        auto fi1 = e.op(0).op(1);
        auto fi2 = e.op(1).op(1);
        auto fi3 = e.op(2).op(1);
        auto mom1 = e.op(0).op(2);
        if(mode==0) return -I * gs * SP(n,LI(fi3)) * Matrix(GAS(1), DI(fi1),DI(fi2)) * (I*SUNF(CI(fi3),TI(fi1),TI(fi2)));
        else if(mode==1 || mode==-2) return -I * gs * SP(n,LI(fi3)) * Matrix(GAS(1), DI(fi1),DI(fi2)) * (-SUNT(TI(fi2),TI(fi1),CI(fi3)));
        else if(mode==2 || mode==-1) return -I * gs * SP(n,LI(fi3)) * Matrix(GAS(1), DI(fi1),DI(fi2)) * SUNT(TI(fi1),TI(fi2),CI(fi3));
        else return 0;
    }

    lst Qgraf::Amplitudes(symtab st, bool debug) {
        std::ofstream ofs;
        ofs.open("qgraf.dat", ios::out);
                
        ofs << "output='" << Output << "';" << endl;
        ofs << "style='" << InstallPrefix << "/include/HepLib.sty';" << endl;
        ofs << "model='" << Model << "';" << endl;
        ofs << "in=" << In << ";" << endl;
        ofs << "out=" << Out << ";" << endl;
        ofs << "loops=" << Loops << ";" << endl;
        ofs << "loop_momentum=" << LoopPrefix << ";" << endl;
        if(Options!="") ofs << "options=" << Options << ";" << endl;
        for(auto vs : Others) ofs << vs << ";" << endl;
        ofs.close();
        
        if(debug) system((InstallPrefix+"/bin/qgraf").c_str());
        else system((InstallPrefix+"/bin/qgraf > /dev/null").c_str());
        
        ifstream ifs(Output);
        string ostr((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
        ifs.close();
        
        if(access("qgraf.dat",F_OK)!=-1) remove("qgraf.dat");
        if(access(Output.c_str(),F_OK)!=-1) remove(Output.c_str());
        
        const char* rm_chars = " \t\v\r\n,";
        if(!ostr.empty()) {
            ostr.erase(0, ostr.find_first_not_of(rm_chars));
            ostr.erase(ostr.find_last_not_of(rm_chars)+1);
        }
        ostr = "{" + ostr + "}";

        Parser amp(st);
        ex ret = amp.Read(ostr);
        
        return ex_to<lst>(ret);
    }
    
    lst Qgraf::TopoLines(const ex & amp) {
        map<ex,int,ex_is_less> v2id, fid2vid;
        map<int,ex> vid2fs; // fileds in the vertex
        int cid = 0;
        lst lines;
        for(const_preorder_iterator i = amp.preorder_begin(); i != amp.preorder_end(); ++i) {
            ex e = (*i);
            if(isFunction(e,"OutField")) {
                lines.append(lst{ e.op(1), iWF(e.op(1)), lst{e.op(0)}, e.op(2) });
            } else if(isFunction(e,"InField")) {
                lines.append(lst{ iWF(e.op(1)), e.op(1), lst{e.op(0)}, e.op(2) });
            } else if(isFunction(e, "Propagator")) {
                lines.append(lst{ iWF(e.op(0).op(1)), iWF(e.op(1).op(1)), lst{e.op(0).op(0), e.op(1).op(0)}, e.op(2) });
            } else if(isFunction(e, "Vertex")) {
                if(v2id[e]==0) {
                    cid++;
                    v2id[e]=cid;
                    lst fs;
                    for(auto f : e) {
                        ex fid = f.op(1);
                        fid2vid[fid] = cid;
                        fs.append(f.op(0));
                    }
                    vid2fs[cid] = fs;
                }
            }
        }
        lines = ex_to<lst>(MapFunction([&vid2fs,&fid2vid](const ex &e, MapFunction &self)->ex{
            if(e.match(iWF(w))) {
                auto vid = fid2vid[e.op(0)];
                return lst{vid, vid2fs[vid]};
            } else if(!e.has(iWF(w))) return e;
            else return e.map(self);
        })(lines));
        
        return lines.sort();
    }
    
    void Qgraf::DrawPDF(const lst & amps, string fn, bool debug) {
        int id=0;
        vector<ex> amp_vec;
        for(auto item : amps) amp_vec.push_back(item);
        string tex_path = to_string(getpid()) + "_TeX/";
        system(("mkdir -p "+tex_path).c_str());
        int limit = 300;
        
        GiNaC_Parallel(-1, amp_vec.size(), [&](int idx)->ex {
            auto amp = amp_vec[idx];
            ofstream out(tex_path+to_string(idx)+".tex");
            out << "\\documentclass[tikz]{standalone}" << endl;
            out << "\\usepackage{tikz-feynman}" << endl;
            out << "\\tikzfeynmanset{compat=1.1.0}" << endl;
            out << "\\begin{document}" << endl;
            out << "\\feynmandiagram{" << endl;
            auto lines = TopoLines(amp);

            exmap bend_map;
            std::map<ex,int,ex_is_less> vtex_map; // vertex option, only once
            for(auto l : lines) {
                lst ll = lst{l.op(0), l.op(1)};
                ll.sort();
                bend_map[ll] = bend_map[ll] + 1;
                
                auto fidL = (is_a<numeric>(l.op(1)) ? l.op(1) : l.op(1).op(0));
                out << "\"" << fidL << "\"";
                if(fidL<0) {
                    out << "[particle=";
                    if(InOutTeX[fidL].length()>0) out << InOutTeX[fidL];
                    else out << fidL;
                    out << "]";
                } else if(vtex_map[l.op(1).op(1)]==0) {
                    out << VerTeX[l.op(1).op(1)];
                    vtex_map[l.op(1).op(1)]=1;
                }

                out << "  --[";
                auto f = l.op(2).op(0);
                if(LineTeX[f].length()>0) out << LineTeX[f];
                if(bend_map[ll]>2) out << ",half right";
                else if(bend_map[ll]>1) out << ",half left";
                if(is_zero(l.op(0)-l.op(1))) out << ",loop,distance=2cm";
                out << "]";
                
                auto fidR = (is_a<numeric>(l.op(0)) ? l.op(0) : l.op(0).op(0));
                out << "  \"" << fidR << "\"";
                if(fidR<0) {
                    out << "[particle=";
                    if(InOutTeX[fidR].length()>0) out << InOutTeX[fidR];
                    else out<< fidR;
                    out << "]";
                } else if(vtex_map[l.op(0).op(1)]==0) {
                    out << VerTeX[l.op(0).op(1)];
                    vtex_map[l.op(0).op(1)]=1;
                }
                out << ";" << endl;
            }
            out << "};" << endl;
            out << "\\end{document}" << endl;
            out.close();
            system(("cd "+tex_path+" && echo X | lualatex " + to_string(idx) + " 1>/dev/null").c_str());
            return 0;
        }, "TeX");
        
        ofstream out(tex_path+"diagram.tex");
        out << "\\documentclass{standalone}" << endl;
        out << "\\usepackage{graphicx}" << endl;
        out << "\\usepackage{adjustbox}" << endl;
        out << "\\begin{document}" << endl;
        out << "\\begin{adjustbox}{valign=T,width=\\textwidth}" << endl;
        out << "\\begin{tabular}{|cc|cc|cc|cc|}" << endl;
        out << "\\hline" << endl;
        int total = amps.nops();
        int namps = total;
        if((total%4)!=0) total = (total/4+1)*4;
        for(int i=0 ; i<total; i++) {
            
            if((i!=0) && (i+1!=total) && (i%limit)==0) {
                out << "\\end{tabular}" << endl << endl;
                out << "\\begin{tabular}{|cc|cc|cc|cc|}" << endl;
                out << "\\hline" << endl;
            }
            
            out << "{\\tiny " << i+1 << "}&" << endl;
            if(i<namps) {
                out << "\\includegraphics[keepaspectratio,";
                out << "height=0.22\\textwidth,";
                out << "width=0.22\\textwidth]";
                out << "{"<<i<<".pdf}" << endl;
            }
            if((i+1)%4==0) out << "\\\\ \\hline";
            else out << "&";
        }
        out << "\\end{tabular}" << endl;
        out << "\\end{adjustbox}" << endl;
        out << "\\end{document}" << endl;
        out.close();
        if(debug)  system(("cd "+tex_path+" && pdflatex diagram && mv diagram.pdf ../"+fn).c_str());
        else system(("cd "+tex_path+" && echo X | pdflatex diagram 1>/dev/null && mv diagram.pdf ../"+fn).c_str());
        if(!debug) system(("rm -r "+tex_path).c_str());
    }
    
    // lst is actually a lst of lst, different connectted parts
    vector<lst> Qgraf::ShrinkCut(ex amp, lst prop, int n) {
        vector<lst> ret;
        auto tls = Qgraf::TopoLines(amp);
        vector<int> cls_vec;
        for(int i=0; i<tls.nops(); i++) {
            auto pi = tls.op(i).op(2);
            if(pi.nops()<2) continue;
            if(is_zero(pi.op(0)-prop.op(0)) && is_zero(pi.op(1)-prop.op(1))) cls_vec.push_back(i);
        }
        if(cls_vec.size()<n) return ret;
        
        Combinations(cls_vec.size(), n, [n,&ret,cls_vec,tls](const int * is)->void {
            int cls[n];
            for(int i=0; i<n; i++) cls[i] = cls_vec[is[i]];;
            
            // cut each line into 2 half-lines, labeled with 0
            auto tls2 = tls;
            for(auto ci : cls) {
                auto ol = tls2.op(ci);
                tls2.let_op(ci) = lst{ol.op(0), 0, lst{ol.op(2).op(0)}, ol.op(3)};
                tls2.append(lst{0, ol.op(1), lst{ol.op(2).op(1)}, ol.op(3)} );
            }
            
            // shrink internal lines
            int last = 0;
            int ntls2 = tls2.nops();
            while(true) {
                ex lp = 0;
                for(int i=last; i<ntls2; i++) {
                    auto li = tls2.op(i);
                    if(is_zero(li) || li.op(2).nops()<2) continue;
                    last = i;
                    lp = li;
                    tls2.let_op(last) = 0;
                    break;
                }
                if(is_zero(lp)) break;
                
                for(int i=0; i<ntls2; i++) {
                    if(is_zero(tls2.op(i))) continue;
                    if(is_zero(tls2.op(i).op(0)-lp.op(0))) tls2.let_op(i).let_op(0) = lp.op(1);
                    if(is_zero(tls2.op(i).op(1)-lp.op(0))) tls2.let_op(i).let_op(1) = lp.op(1);
                }
            }

            // final connected parts
            map<int, lst> con_map;
            for(auto li : tls2) {
                if(is_zero(li)) continue;
                ex key, val;
                ex fiL = li.op(0);
                if(is_a<lst>(fiL)) fiL = fiL.op(0);
                ex fiR = li.op(1);
                if(is_a<lst>(fiR)) fiR = fiR.op(0);
                if(fiL>0 && fiR<0) {
                    con_map[ex_to<numeric>(fiL).to_int()].append(fiR);
                } else if(fiR>0 && fiL<0) {
                    con_map[ex_to<numeric>(fiR).to_int()].append(fiL);
                } else {
                    if(fiL>0 && is_zero(fiR)) key = fiL;
                    else if(fiR>0 && is_zero(fiL)) key = fiR;
                    else throw Error("ShrinkCut: unexpcected point reached.");
                    val = li.op(2).op(0);
                    con_map[ex_to<numeric>(key).to_int()].append(val);
                }
            }
            
            lst item;
            for(auto kv : con_map) item.append(kv.second.sort());
            ret.push_back(item);
        });
        
        return ret;
    }
    
    bool Qgraf::HasLoop(ex amp, lst prop) {
        auto tls = Qgraf::TopoLines(amp);
        int ntls = tls.nops();
        // shrink internal lines of prop
        int last = 0;
        while(true) {
            ex lp = 0;
            for(int i=last; i<ntls; i++) {
                auto li = tls.op(i);
                if(is_zero(li) || li.op(2).nops()<2) continue; // external line
                auto cpi = li.op(2);
                bool m1 = !(is_zero(cpi.op(0)-prop.op(0)) && is_zero(cpi.op(1)-prop.op(1)));
                bool m2 = !(is_zero(cpi.op(0)-prop.op(1)) && is_zero(cpi.op(1)-prop.op(0)));
                if(m1 && m2) continue; // other propagators
                if(is_zero(li.op(0)-li.op(1))) return true; // a loop found
                last = i;
                lp = li;
                tls.let_op(last) = 0;
                break;
            }
            if(is_zero(lp)) break;
            
            for(int i=0; i<ntls; i++) {
                if(is_zero(tls.op(i))) continue;
                if(is_zero(tls.op(i).op(0)-lp.op(0))) tls.let_op(i).let_op(0) = lp.op(1);
                if(is_zero(tls.op(i).op(1)-lp.op(0))) tls.let_op(i).let_op(1) = lp.op(1);
            }
        }
        return false;
    }
    
}

