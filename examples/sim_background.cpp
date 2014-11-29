/* 
* Copyright 2014 Friedemann Zenke
*
* This file is part of Auryn, a simulation package for plastic
* spiking neural networks.
* 
* Auryn is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Auryn is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with Auryn.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "auryn.h"

#define N_REC_WEIGHTS 5000

using namespace std;

namespace po = boost::program_options;
namespace mpi = boost::mpi;

int main(int ac, char* av[]) 
{

	double w = 0.16;
	double w_ext = w;
	double wmax = 1.0;

	double w_ee = w;
	double w_ei = w;

	double gamma = 1.0;
	double w_ie = gamma;
	double w_ii = gamma;

	NeuronID ne = 20000;
	NeuronID ni = ne/4;


	double sparseness = 0.05;
	double kappa = 3.;

	bool quiet = false;
	bool scaling = false;
	bool wmatdump = false;
	bool loadbalance = false;

	bool wall = false;

	double tau_chk = 100e-3;
	double simtime = 3600.;
	double stimtime = simtime;
	double wmat_interval = 600.;

	double ampa_nmda_ratio = 1.0;
	double wstim = 0.1;

	NeuronID psize = 200;
	NeuronID hsize = 100;
	NeuronID offset = 0;

	string patfile = "";
	string prefile = "";
	string currentfile = "";

	double stimfreq = 10;

	bool corr = false;
	int  plen = 3;
	int  hlen = 3;

	double ampl = 1.0;
	bool recall = false;
	bool decay = false;
	bool adapt = false;
	bool noisyweights = false;
	bool switchweights = false;
	bool ei_plastic = false;

	double bg_rate = 2;
	bool fast = false;
	AurynWeight wdecay = w;
	double tau_decay = 3600.;

	double tau_hom = 50.;
	double eta = 1;
	double onperiod = 2;
	double offperiod = 30;
	double scale = 1;

	int n_strengthen = 0;

	vector<Connection*> corr_connections;
	string dir = ".";
	string stimfile = "";
	string label = "";
	string infilename = "";

	const char * file_prefix = "bg2";
	char strbuf [255];
	string msg;

	int errcode = 0;

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("quiet", "quiet mode")
            ("scaling", "scaling mode")
            ("balance", "activate load balance")
            ("load", po::value<string>(), "input weight matrix")
            ("patfile", po::value<string>(), "pattern file for StimulusGroup")
            ("prefile", po::value<string>(), "pattern file for loading patterns")
            ("onperiod", po::value<double>(), "mean stimulus on period")
            ("offperiod", po::value<double>(), "mean stimulus off period")
            ("wmat", "wmat dump mode")
            ("eta", po::value<double>(), "learning rate")
            ("bgrate", po::value<double>(), "PoissonGroup external firing rate")
            ("sparseness", po::value<double>(), "overall network sparseness")
            ("scale", po::value<double>(), "learning rate")
            ("tau_hom", po::value<double>(), "homeostatic time constant")
            ("kappa", po::value<double>(), "target rate")
            ("simtime", po::value<double>(), "simulation time")
            ("dir", po::value<string>(), "output dir")
            ("label", po::value<string>(), "output label")
            ("wee", po::value<double>(), "wee")
            ("wei", po::value<double>(), "wei")
            ("wie", po::value<double>(), "wie")
            ("wii", po::value<double>(), "wii")
            ("wmax", po::value<double>(), "wmax")
            ("ampa", po::value<double>(), "ampa nmda ratio")
            ("strengthen", po::value<int>(), "connections to strengthen by 10")
            ("ne", po::value<int>(), "no of exc units")
            ("stimfile", po::value<string>(), "stimulus ras file")
            ("wstim", po::value<double>(), "weight of stimulus connections")
            ("stimtime", po::value<double>(), "time of stimulus on")
            ("decay", "decay triplet connections")
            ("corr", "add correlated inputs")
            ("ampl", po::value<double>(), "recall stim amplitude")
            ("psize", po::value<int>(), "block size for correlated inputs")
            ("plen", po::value<int>(), "number of blocks feed-forward")
            ("hsize", po::value<int>(), "block size for correlated inputs (hebbian assembly)")
            ("hlen", po::value<int>(), "number of blocks Hebbian")
            ("offset", po::value<int>(), " offset for corr stim")
            ("recall", "add correlated inputs to first patterns")
            ("stimfreq", po::value<double>(), "CorrelatedPoissonGroup frequency default = 100")
            ("dconstant", po::value<double>(), "decay time constant for decaying triplet connections")
            ("wdecay", po::value<double>(), "wdecay for decay triplet connections")
            ("chk", po::value<double>(), "checker time constant")
            ("adapt", "adapting excitatory neurons")
            ("wall", "enable monitoring of wall clock time")
            ("noisyweights", "enables noisyweights for mean field checks")
            ("switchweights", "switches first weights in each weight matrix")
            ("fast", "turn off some of the monitors to run faster")
            ("eiplastic", "make EI connection plastic")
        ;

        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);    

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }

        if (vm.count("quiet")) {
			quiet = true;
        } 

        if (vm.count("scaling")) {
			scaling = true;
        } 

        if (vm.count("balance")) {
			cout << "load balancing active" << endl;
			loadbalance = true;
        } 

        if (vm.count("load")) {
            cout << "load from matrix " 
                 << vm["load"].as<string>() << ".\n";
			infilename = vm["load"].as<string>();
        } 

        if (vm.count("patfile")) {
            cout << "PatternFile is " 
                 << vm["patfile"].as<string>() << ".\n";
			patfile = vm["patfile"].as<string>();
        } 

        if (vm.count("prefile")) {
            cout << "Preload patternfile is " 
                 << vm["prefile"].as<string>() << ".\n";
			prefile = vm["prefile"].as<string>();
        } 

        if (vm.count("wmat")) {
			wmatdump = true;
			cout << "wmat dump mode" << endl;
        } 

        if (vm.count("onperiod")) {
            cout << "onperiod set to " 
                 << vm["onperiod"].as<double>() << ".\n";
			onperiod = vm["onperiod"].as<double>();
        } 

        if (vm.count("offperiod")) {
            cout << "offperiod set to " 
                 << vm["offperiod"].as<double>() << ".\n";
			offperiod = vm["offperiod"].as<double>();
        } 

        if (vm.count("eta")) {
            cout << "eta set to " 
                 << vm["eta"].as<double>() << ".\n";
			eta = vm["eta"].as<double>();
        } 

        if (vm.count("bgrate")) {
            cout << "bgrate set to " 
                 << vm["bgrate"].as<double>() << ".\n";
			bg_rate = vm["bgrate"].as<double>();
        } 

        if (vm.count("sparseness")) {
            cout << "sparseness set to " 
                 << vm["sparseness"].as<double>() << ".\n";
			sparseness = vm["sparseness"].as<double>();
        } 

        if (vm.count("scale")) {
            cout << "scale set to " 
                 << vm["scale"].as<double>() << ".\n";
			scale = vm["scale"].as<double>();
        } 

        if (vm.count("tau_hom")) {
            cout << "tau_hom set to " 
                 << vm["tau_hom"].as<double>() << ".\n";
			tau_hom = vm["tau_hom"].as<double>();
        } 

        if (vm.count("kappa")) {
            cout << "kappa set to " 
                 << vm["kappa"].as<double>() << ".\n";
			kappa = vm["kappa"].as<double>();
        } 

        if (vm.count("simtime")) {
            cout << "simtime set to " 
                 << vm["simtime"].as<double>() << ".\n";
			simtime = vm["simtime"].as<double>();
			stimtime = simtime;
        } 

        if (vm.count("dir")) {
            cout << "dir set to " 
                 << vm["dir"].as<string>() << ".\n";
			dir = vm["dir"].as<string>();
        } 

        if (vm.count("label")) {
            cout << "label set to " 
                 << vm["label"].as<string>() << ".\n";
			label = vm["label"].as<string>();
        } 

        if (vm.count("wee")) {
            cout << "wee set to " 
                 << vm["wee"].as<double>() << ".\n";
			w_ee = vm["wee"].as<double>();
        } 

        if (vm.count("wei")) {
            cout << "wei set to " 
                 << vm["wei"].as<double>() << ".\n";
			w_ei = vm["wei"].as<double>();
        } 

        if (vm.count("wie")) {
            cout << "wie set to " 
                 << vm["wie"].as<double>() << ".\n";
			w_ie = vm["wie"].as<double>();
        } 

        if (vm.count("wii")) {
            cout << "wii set to " 
                 << vm["wii"].as<double>() << ".\n";
			w_ii = vm["wii"].as<double>();
        } 

        if (vm.count("wmax")) {
            cout << "wmax set to " 
                 << vm["wmax"].as<double>() << ".\n";
			wmax = vm["wmax"].as<double>();
        } 

        if (vm.count("ampa")) {
            cout << "ampa set to " 
                 << vm["ampa"].as<double>() << ".\n";
			ampa_nmda_ratio = vm["ampa"].as<double>();
        } 

        if (vm.count("strengthen")) {
            cout << "strengthen set to " 
                 << vm["strengthen"].as<int>() << ".\n";
			n_strengthen = vm["strengthen"].as<int>();
        } 

        if (vm.count("ne")) {
            cout << "ne set to " 
                 << vm["ne"].as<int>() << ".\n";
			ne = vm["ne"].as<int>();
			ni = ne/4;
        } 

        if (vm.count("stimfile")) {
            cout << "stimfile set to " 
                 << vm["stimfile"].as<string>() << ".\n";
			stimfile = vm["stimfile"].as<string>();
        } 

        if (vm.count("wstim")) {
            cout << "wstim set to " 
                 << vm["wstim"].as<double>() << ".\n";
			wstim = vm["wstim"].as<double>();
        } 

        if (vm.count("stimtime")) {
            cout << "stimtime set to " 
                 << vm["stimtime"].as<double>() << ".\n";
			stimtime = vm["stimtime"].as<double>();
        } 

        if (vm.count("corr")) {
            cout << "enabling corr " << endl;
			corr = true;
        } 

        if (vm.count("ampl")) {
            cout << "ampl set to " 
                 << vm["ampl"].as<double>() << ".\n";
			ampl = vm["ampl"].as<double>();
        } 

        if (vm.count("psize")) {
            cout << "psize set to " 
                 << vm["psize"].as<int>() << ".\n";
			psize = vm["psize"].as<int>();
        } 

        if (vm.count("plen")) {
            cout << "plen set to " 
                 << vm["plen"].as<int>() << ".\n";
			plen = vm["plen"].as<int>();
        } 

        if (vm.count("hsize")) {
            cout << "hsize set to " 
                 << vm["hsize"].as<int>() << ".\n";
			hsize = vm["hsize"].as<int>();
        } 

        if (vm.count("hlen")) {
            cout << "hlen set to " 
                 << vm["hlen"].as<int>() << ".\n";
			hlen = vm["hlen"].as<int>();
        } 

        if (vm.count("offset")) {
            cout << "offset set to " 
                 << vm["offset"].as<int>() << ".\n";
			offset = vm["offset"].as<int>();
        } 

        if (vm.count("recall")) {
            cout << "enabling recall " << endl;
			recall = true;
        } 

        if (vm.count("stimfreq")) {
            cout << "stimfreq set to " 
                 << vm["stimfreq"].as<double>() << ".\n";
			stimfreq = vm["stimfreq"].as<double>();
        } 

        if (vm.count("decay")) {
            cout << "weight decay on " << endl;
			decay = true;
        } 

        if (vm.count("dconstant")) {
            cout << "dconstant set to " 
                 << vm["dconstant"].as<double>() << ".\n";
			tau_decay = vm["dconstant"].as<double>();
        } 

        if (vm.count("wdecay")) {
            cout << "wdecay set to " 
                 << vm["wdecay"].as<double>() << ".\n";
			wdecay = vm["wdecay"].as<double>();
        } 

        if (vm.count("chk")) {
            cout << "chk set to " 
                 << vm["chk"].as<double>() << ".\n";
			tau_chk = vm["chk"].as<double>();
        } 

        if (vm.count("adapt")) {
            cout << "adaptation on " << endl;
			adapt = true;
        } 

        if (vm.count("wall")) {
            cout << "real time wall clock monitoring enabled " << endl;
			wall = true;
        } 

        if (vm.count("noisyweights")) {
            cout << "noisyweights on " << endl;
			noisyweights = true;
        } 

        if (vm.count("switchweights")) {
            cout << "switchweights on " << endl;
			switchweights = true;
        } 

        if (vm.count("fast")) {
            cout << "fast on " << endl;
			fast = true;
        } 

        if (vm.count("eiplastic")) {
            cout << "eiplastic on " << endl;
			ei_plastic = true;
        } 
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }

	if (scale != 1.) {
		eta *= scale;
		tau_hom /= scale;
		simtime /= scale;
		cout << "scaled by " << scale << endl;
	}

	double primetime = 3*tau_hom;


	// BEGIN Global stuff
	mpi::environment env(ac, av);
	mpi::communicator world;
	communicator = &world;

	sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.log", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
	string logfile = strbuf;
	logger = new Logger(logfile,world.rank(),PROGRESS,EVERYTHING);

	sys = new System(&world);
	// boost::filesystem::path p = av[0];
	// string binaryname = p.stem().string();
	// sys->set_simulation_name(binaryname);
	// END Global stuff

	if (!infilename.empty()) {
		stringstream iss;
		iss << infilename << "." << world.rank();
		infilename = iss.str();
	}

	logger->msg("Setting up neuron groups ...",PROGRESS,true);


	NeuronGroup * neurons_e;
	if ( adapt ) {
		neurons_e = new AIFGroup(ne);
		((AIFGroup*)neurons_e)->set_ampa_nmda_ratio(ampa_nmda_ratio);
		((AIFGroup*)neurons_e)->dg_adapt1=1.0;
	} else {
		if ( loadbalance ) 
			neurons_e = new IFGroup(ne,1.2,(1.2*ne+ni));
		else
			neurons_e = new IFGroup(ne);
		((IFGroup*)neurons_e)->set_ampa_nmda_ratio(ampa_nmda_ratio);
	}
	IFGroup * neurons_i = new IFGroup(ni);

	// initialize membranes
	neurons_i->set_tau_mem(10e-3);
	neurons_e->random_mem(-60e-3,10e-3);
	neurons_i->random_mem(-60e-3,10e-3);

	((IFGroup*)neurons_i)->set_ampa_nmda_ratio(ampa_nmda_ratio);


	SpikingGroup * poisson = new PoissonGroup(2500,bg_rate);
	SparseConnection * con_exte = new SparseConnection(poisson, neurons_e, w_ext, sparseness, GLUT);

	msg = "Setting up I connections ...";
	logger->msg(msg,PROGRESS,true);
	SparseConnection * con_ie = new SparseConnection(neurons_i,neurons_e,
			w_ie,sparseness,GABA);
	SparseConnection * con_ii = new SparseConnection(neurons_i,neurons_i,
			w_ii,sparseness,GABA);

	msg =  "Setting up E connections ...";
	logger->msg(msg,PROGRESS,true);
	SparseConnection * con_ei;
	if ( !ei_plastic ) {
		con_ei = new SparseConnection(neurons_e,neurons_i,
				w_ei, sparseness,GLUT);
	} else {
		if (infilename.empty()) {
			con_ei = new TripletConnection(neurons_e,neurons_i,
				w_ei, sparseness,
				tau_hom, eta, kappa, wmax, GLUT);
		} else {
			string str;
			str = infilename;
			str += ".ei.wmat";
			stringstream oss;
			oss << "Loading weight matrix from " << str;
			logger->msg(oss.str(),PROGRESS,true);
			con_ei = new TripletConnection(neurons_e,neurons_i,
				str.c_str(),
				tau_hom, eta, kappa, wmax, GLUT);
		}
	}

	TripletConnection * con_ee;

	if (infilename.empty()) {
		if (decay)
			con_ee = new TripletDecayConnection(neurons_e,neurons_e,
				w_ee, sparseness,
				tau_hom, eta, tau_decay,
				kappa, wdecay, wmax, GLUT);
		else
			con_ee = new TripletConnection(neurons_e,neurons_e,
				w_ee, sparseness,
				tau_hom, eta, kappa, wmax, GLUT);

		// con_ee->set_min_weight(wmin);
		if ( noisyweights )
			con_ee->random_data(w_ee,w_ee/4);
		for ( int i = 0 ; i < n_strengthen ; ++i ) {
			con_ee->set_data(i,i*(wmax/n_strengthen));
		}
	}
	else {
		string str;
		str = infilename;
		str += ".wmat";
		stringstream oss;
		oss << "Loading weight matrix from " << str;
		logger->msg(oss.str(),PROGRESS,true);
		if (decay)
			con_ee = new TripletDecayConnection(neurons_e,neurons_e,
				str.c_str(),tau_hom,eta,tau_decay,kappa,wdecay,wmax,GLUT);
		else
			con_ee = new TripletConnection(neurons_e,neurons_e,
				str.c_str(),tau_hom,eta,kappa,wmax,GLUT);


		sprintf(strbuf, "%s.e.nstate", infilename.c_str());
		neurons_e->load_from_file(strbuf);
		sprintf(strbuf, "%s.i.nstate", infilename.c_str());
		neurons_i->load_from_file(strbuf);

		// primetime = 0;
	}

	// SparseConnection * con_exti = new SparseConnection(sys, poisson, neurons_i, w_ext, sparseness, GLUT);

	msg = "Initializing traces ...";
	logger->msg(msg,PROGRESS,true);
	con_ee->set_hom_trace(kappa);
	con_ee->seed(23);

	// TODO
	// con_ee->w->set_col(0,2*w_ee);

	msg = "Setting up monitors ...";
	logger->msg(msg,PROGRESS,true);

	if (wmatdump) {
		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.weight", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		WeightMatrixMonitor * wmatmon = new WeightMatrixMonitor( con_ee, strbuf , wmat_interval );
	}

	if ( !fast ) {
		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.syn", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		WeightMonitor * wmon = new WeightMonitor( con_ee, strbuf, 10 ); 
		for ( int i = 0 ; i < 5 ; ++i ) {
			for ( int j = 0 ; j < 5 ; ++j ) {
				vector<neuron_pair> sublist = con_ee->get_block(i*psize,(i+1)*psize,j*psize,(j+1)*psize);
				wmon->add_to_list(sublist);
			}
		}

		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.ras", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'e');
		SpikeMonitor * smon_e = new SpikeMonitor( neurons_e, strbuf , 2500);

		// sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.ras", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'p');
		// SpikeMonitor * smon_p = new SpikeMonitor( poisson, strbuf , 2500);
	}

	sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.prate", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'e');
	PopulationRateMonitor * pmon_e = new PopulationRateMonitor( neurons_e, strbuf, 1.0 );

	if ( wall ) {
		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.rt", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		RealTimeMonitor * rtmon = new RealTimeMonitor( strbuf );
	}

	// sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.prate", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'i');
	// PopulationRateMonitor * pmon_i = new PopulationRateMonitor( neurons_i, strbuf, 1.0 );

	// sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.mem", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'e');
	// VoltageMonitor * vmon_e = new VoltageMonitor( neurons_e, 33, strbuf );

	// sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.ras", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'i');
	// SpikeMonitor * smon_i = new SpikeMonitor( neurons_i, strbuf , 500);

	RateChecker * chk = new RateChecker( neurons_e , 0.1 , 20.*kappa , tau_chk);



	if ( scaling && (errcode==0) ) {
		stringstream oss;
		oss << "Changing cell input ... ";
		logger->msg(oss.str(),PROGRESS,true);

		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.pat", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		PatternMonitor * patmon = new PatternMonitor(neurons_e,strbuf,patfile.c_str(), 10, 1.);
		PatternStimulator * patstim = new PatternStimulator(neurons_e,currentfile.c_str(),patfile.c_str());

		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.scal", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		WeightMonitor * wmon_scal = new WeightMonitor( con_ee, 0, 0, strbuf, 1, ELEMENTLIST); 
		wmon_scal->add_to_list( con_ee->get_pre_partners(0) );
		wmon_scal->add_to_list( con_ee->get_pre_partners(10) );

	}

	// if ( corr ) {
	// 	sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.pat", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'c');
	// 	PatternMonitor * pmon_c = new PatternMonitor( neurons_e, strbuf,corr_pat_file.c_str() );
	// }

	const double amplitude_fwd = 9.0*ampl;
	const double amplitude_heb = 3.0*ampl;

	CorrelatedPoissonGroup * corr_e;
	CorrelatedPoissonGroup * corr_e2;

	NeuronID extsize = 500;
	double strength = 0.2;

	if ( corr ) {

		stringstream oss;
		oss << "Activating correlated input ... ";
		logger->msg(oss.str(),PROGRESS,true);

		if (eta > 0)
			con_ee->stdp_active = true;


		if ( psize*plen ) {
			corr_e = new CorrelatedPoissonGroup(extsize*plen,stimfreq,extsize,10e-3);
			corr_e->set_amplitude(amplitude_fwd);
			corr_e->set_timescale(5e-3);
			// IdentityConnection * con_corr_e = new IdentityConnection(corr_e,neurons_e,w);
			SparseConnection * con_corr_e = new SparseConnection(corr_e,neurons_e,GLUT);
			con_corr_e->allocate_manually( plen*extsize*psize*sparseness*1.05 );
			con_corr_e->w->clear();
			for ( int i = 0 ; i < plen ; ++i )
				con_corr_e->connect_block_random(w,sparseness,(i)*extsize,offset+(i+1)*extsize,(i)*psize,offset+(i+1)*psize);
			con_corr_e->finalize();
			corr_connections.push_back(con_corr_e);
			SparseConnection * con_corr_i = new SparseConnection(corr_e,neurons_i,w*strength,sparseness*plen*psize/ni,GLUT);

			// set up Weight monitor
			sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.ras", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'c');
			SpikeMonitor * smon_c = new SpikeMonitor( corr_e, strbuf , psize );
		}

		if ( hsize*hlen ) {
			corr_e2 = new CorrelatedPoissonGroup(extsize*hlen,stimfreq,extsize,10);
			corr_e2->set_amplitude(amplitude_heb);
			corr_e2->set_timescale(100e-3);
			SparseConnection * con_corr_e2 = new SparseConnection(corr_e2,neurons_e,GLUT);
			con_corr_e2->allocate_manually( hlen*extsize*hsize*sparseness*1.05 );
			con_corr_e2->w->clear();
			for ( int i = 0 ; i < hlen ; ++i )
				con_corr_e2->connect_block_random(w,sparseness,(i)*extsize,(i+1)*extsize,offset+(i)*hsize+plen*psize,offset+(i+1)*hsize+plen*psize);
			con_corr_e2->finalize();
			corr_connections.push_back(con_corr_e2);
			SparseConnection * con_corr_i2 = new SparseConnection(corr_e2,neurons_i,w*strength,sparseness*hlen*hsize/ni,GLUT);
		}

		// disabling external random input
		con_exte->set_block(0,2500,0,plen*psize+hlen*hsize,0.0);

	} else {
		if ( recall ) {

			stringstream oss;
			oss << "Activating recall input ... ";
			logger->msg(oss.str(),PROGRESS,true);

			if ( psize*plen ) {
				corr_e = new CorrelatedPoissonGroup(extsize*plen,stimfreq,extsize,10e-3);
				corr_e->set_amplitude(amplitude_fwd);
				corr_e->set_timescale(5e-3);
				// IdentityConnection * con_corr_e = new IdentityConnection(corr_e,neurons_e,w);
				SparseConnection * con_corr_e = new SparseConnection(corr_e,neurons_e,GLUT);
				con_corr_e->allocate_manually( plen*extsize*psize*sparseness*1.05 );
				con_corr_e->w->clear();
				for ( int i = 0 ; i < plen ; ++i )
					con_corr_e->connect_block_random(w,sparseness,(i)*extsize,offset+(i+1)*extsize,(i)*psize,offset+(i+1)*psize);
				con_corr_e->finalize();
				corr_connections.push_back(con_corr_e);
				// SparseConnection * con_corr_i = new SparseConnection(corr_e,neurons_i,w*strength,sparseness*plen*psize/ni,GLUT);

				// set up Weight monitor
				sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%c.ras", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank(), 'c');
				SpikeMonitor * smon_c = new SpikeMonitor( corr_e, strbuf , psize );
			}

			if ( hsize*hlen ) {
				corr_e2 = new CorrelatedPoissonGroup(extsize*hlen,stimfreq,extsize,10);
				corr_e2->set_amplitude(amplitude_heb);
				corr_e2->set_timescale(100e-3);
				SparseConnection * con_corr_e2 = new SparseConnection(corr_e2,neurons_e,GLUT);
				con_corr_e2->allocate_manually( hlen*extsize*hsize*sparseness*1.05 );
				con_corr_e2->w->clear();
				for ( int i = 0 ; i < hlen ; ++i )
					con_corr_e2->connect_block_random(w,sparseness,(i)*extsize,(i+1)*extsize,offset+(i)*hsize+plen*psize,offset+(i+1)*hsize+plen*psize);
				con_corr_e2->finalize();
				corr_connections.push_back(con_corr_e2);
				// SparseConnection * con_corr_i2 = new SparseConnection(corr_e2,neurons_i,w*strength,sparseness*hlen*hsize/ni,GLUT);
			}

			// disabling external random input
			// con_exte->set_block(0,2500,psize,2*psize,0.0);
		}
	}

// preload patterns
if ( prefile != "" ) {
	logger->msg("Preloading patterns ...",PROGRESS,true);
	// con_ee->random_data(w_ee,w_ee);
	con_ee->load_patterns(prefile, wmax, true, false);
	// con_ee->sparse_set_data(0.9,0);
}

// adding patterns
if ( patfile != "" ) {
	logger->msg("Preparing stimulus ...",PROGRESS,true);
	sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.stim", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
	StimulusGroup * stimgroup = new StimulusGroup(ne,patfile,strbuf);
	stimgroup->set_mean_on_period(onperiod);
	stimgroup->set_mean_off_period(offperiod);
	IdentityConnection * con_stim = new IdentityConnection(stimgroup,neurons_e,w_ext);
}

	if (primetime>0) {
		msg = "Priming ...";
		logger->msg(msg,PROGRESS,true);
		con_ee->stdp_active = false;
		sys->run(primetime,true);
	}

	if ( corr || recall ) {
		if ( psize*plen ) 
			corr_e->set_target_amplitude(amplitude_fwd);
		if ( hsize*hlen ) 
			corr_e2->set_target_amplitude(amplitude_heb);
	}

	logger->msg("Simulating ...",PROGRESS,true);
	if (eta > 0) {
		con_ee->stdp_active = true;
	}

	if (!sys->run(simtime,true)) 
			errcode = 1;



	if (!fast) {
		logger->msg("Saving neurons state ...",PROGRESS,true);
		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.e.nstate", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		neurons_e->write_to_file(strbuf);
		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.i.nstate", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		neurons_i->write_to_file(strbuf);

		logger->msg("Saving weight matrix ...",PROGRESS,true);
		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.wmat", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		con_ee->write_to_file(strbuf);

		sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.ei.wmat", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
		con_ei->write_to_file(strbuf);

		for ( int i = 0 ; i < corr_connections.size() ; ++i ) {
			sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.%d.wmat", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), i, world.rank());
			corr_connections[i]->write_to_file(strbuf);
		}
	}

	// save lifetime
	sprintf(strbuf, "%s/%s_e%.2et%.2f%s.%d.lifetime", dir.c_str(), file_prefix, eta, tau_hom, label.c_str(), world.rank());
	ofstream killfile;
	killfile.open(strbuf);
	killfile << sys->get_time()-primetime << endl;
	killfile.close();

	logger->msg("Freeing ...",PROGRESS,true);
	delete sys;

	if (errcode) {
		env.abort(errcode);
	}

	return errcode;
}
