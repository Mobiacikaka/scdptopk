#include "party.hpp"

Party::Party() {
    data_set.Init();
}

Party::~Party() {

}

void Party::SetParameters(e_role role, std::string address, uint16_t port, seclvl seclevel, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg)
{
	this->role = role;
	this->address = address;
	this->port = port;
	this->seclevel = seclevel;
	this->bitlen = bitlen;
	this->nthreads = nthreads;
	this->mt_alg = mt_alg;
}

void Party::Run() {
    this->Prune();
    std::cout << "Pruning Finished Successfully!" << std::endl;

	this->data_set.PrintAllElement();

    this->MergeAndShare();
    std::cout << "Merge and Share Finished Successfully!" << std::endl;

	this->SelectionProbability();
	std::cout << "Selection Probability Finished Successfully!" << std::endl;

	// std::cout << std::endl << "Share Dataset" << std::endl;
	// PrintElements(this->shr_dataset);
	// std::cout << std::endl << "Share Gap" << std::endl;
	// PrintElements(this->shr_gap);
	// std::cout << std::endl << "Share Mass" << std::endl;
	// PrintElements(this->shr_mass);

	this->MedianSelection();
	std::cout << "Median Selection Finished Successfully!" << std::endl;
}

std::vector<uint32_t> Party::PutVectorCondSwapGate(uint32_t a, uint32_t b, uint32_t s, BooleanCircuit* circ) {
	std::vector<uint32_t> avec(1, a);
	std::vector<uint32_t> bvec(1, b);
	std::vector<uint32_t> out(2);
	//uint32_t svec = circ->PutRepeaterGate(s, 32);
	std::vector<std::vector<uint32_t>> temp = circ->PutCondSwapGate(avec, bvec, s, true);
	out[0] = temp[0][0];
	out[1] = temp[1][0];
	return out;
}

std::vector<uint32_t> Party::PutVectorBitonicSortGate(share** srv_set, share** cli_set, uint32_t neles, uint32_t bitlen, BooleanCircuit* circ) {

	uint32_t seqsize = 2*neles;
	uint32_t selbitsvec;
	uint32_t i, k, ctr;
	int32_t j;

	std::vector<uint32_t> compa(seqsize / 2);
	std::vector<uint32_t> compb(seqsize / 2);
	std::vector<uint32_t> posa(seqsize / 2);
	std::vector<uint32_t> posb(seqsize / 2);
	//share **c, *selbits;

	std::vector<uint32_t> selbits;
	std::vector<uint32_t> c(seqsize);
	std::vector<uint32_t> temp;
	std::vector<uint32_t> tempcmpveca(bitlen);
	std::vector<uint32_t> tempcmpvecb(bitlen);

	std::vector<uint32_t> parenta(seqsize / 2);
	std::vector<uint32_t> parentb(seqsize / 2);


	//c = (share**) malloc(sizeof(share*) * seqsize);

	//Combine all values of a and b into a single vector c
	for (i = 0; i < neles; i++) {
		c[i] = srv_set[i]->get_wire_id(0);
		c[i + neles] = cli_set[i]->get_wire_id(0);
	}

	//Build bitonic sort gate for all values in C
	for (i = 1 << floor_log2(seqsize - 1); i > 0; i >>= 1) {
		ctr = 0;
		for (j = seqsize - 1, ctr = 0; j >= 0; j -= 2 * i) {
			for (k = 0; k < i && j - i - k >= 0; k++) {
				compa[ctr] = j - i - k;
				compb[ctr] = j - k;
				ctr++;
			}
		}

		//TODO: Introduce specific gate that allows the permutation of vector gates from different input gates + bit positions

		for (uint32_t l = 0; l < bitlen; l++) {
			//cout << "l = " << l << endl;
			for (k = 0; k < ctr; k++) {
				parenta[k] = c[compa[k]];
				parentb[k] = c[compb[k]];
				posa[k] = l;
				posb[k] = l;
			}
			tempcmpveca[l] = circ->PutCombineAtPosGate(parenta, l);
			tempcmpvecb[l] = circ->PutCombineAtPosGate(parentb, l);
		}

		selbitsvec = circ->PutGTGate(tempcmpveca, tempcmpvecb);

		selbits = circ->PutSplitterGate(selbitsvec);
		for (k = 0; k < ctr; k++) {
			temp = PutVectorCondSwapGate(c[compa[k]], c[compb[k]], selbits[k], circ);
			c[compa[k]] = temp[0];
			c[compb[k]] = temp[1];
		}
	}

	return c;
}

std::vector<share*>
Party::BuildMergeAndSortCircuit(share** srv_set, share** cli_set, share** shr_rnd_srv_set, uint32_t neles, uint32_t bitlen, BooleanCircuit* circ)
{
    std::vector<uint32_t> merge_out = PutVectorBitonicSortGate(srv_set, cli_set, neles, bitlen, circ);

    std::vector<share*> sub_wire(2 * neles);
    for(size_t i = 0; i < 2 * neles; i ++) {
        boolshare tmp_share(1, circ);
        tmp_share.set_wire_id(0, merge_out[i]);
        sub_wire[i] = circ->PutSUBGate(&tmp_share, shr_rnd_srv_set[i]);
    }

    return sub_wire;
}

template<class T>
share* Party::PutINGate(BooleanCircuit* circ, uint32_t bitlen, e_role role, T data) {
	if(role == this->role) {
		return circ->PutINGate(data, bitlen, role);
	}
	else {
		return circ->PutDummyINGate(bitlen);
	}
}

void Party::MedianSelection() {
    uint64_t R = this->generate_R();
    uint64_t r = this->RandomDraw(R+1, this->nonces1);

    size_t length = this->shr_dataset.size();
    // size_t bitlen(64);
    typedef uint32_t inputtype;
    typedef uint32_t outputtype;

    ABYParty* party = new ABYParty(role, address, port, seclevel, bitlen, nthreads, mt_alg, 4000000);
	std::vector<Sharing*>& sharings = party->GetSharings();
	BooleanCircuit* bcirc = (BooleanCircuit*) sharings[S_BOOL]->GetCircuitBuildRoutine();

    share *tmp_srv, *tmp_cli;

/**
 * @param shr_cmb_dataset share combine dataset
 * @param shr_cmb_gap share combine gap 
 * @param shr_cmb_mass share combine mass
 * @param shr_no share vector of {0, 1, 2, 3, 4, 5, 6...}
 * @brief correspond to operation 5-7
*/
    share** shr_cmb_dataset = (share**)malloc(sizeof(share*) * length);
    for(size_t i = 0; i < length; i ++) {
        // tmp_srv = bcirc->PutINGate(this->shr_dataset[i], bitlen, role);
        // tmp_cli = bcirc->PutDummyINGate(bitlen);
		tmp_srv = PutINGate(bcirc, this->bitlen, SERVER, this->shr_dataset[i]);
		tmp_cli = PutINGate(bcirc, this->bitlen, CLIENT, this->shr_dataset[i]);
        shr_cmb_dataset[i] = bcirc->PutADDGate(tmp_srv, tmp_cli);
        // bcirc->PutPrintValueGate(shr_cmb_dataset[i], "dataset");
        delete tmp_srv, tmp_cli;
    }

    share** shr_cmb_gap = (share**)malloc(sizeof(share*) * length);
    for(size_t i = 0; i < length; i ++) {
        // tmp_srv = bcirc->PutINGate((uint64_t)this->shr_gap[i], bitlen, role);
        // tmp_cli = bcirc->PutDummyINGate(bitlen);
		tmp_srv = PutINGate(bcirc, bitlen, SERVER, (inputtype)this->shr_gap[i]);
		tmp_cli = PutINGate(bcirc, bitlen, CLIENT, (inputtype)this->shr_gap[i]);
        shr_cmb_gap[i] = bcirc->PutADDGate(tmp_srv, tmp_cli);
        // bcirc->PutPrintValueGate(shr_cmb_gap[i], "gap");
        delete tmp_srv, tmp_cli;
    }

    share** shr_cmb_mass = (share**)malloc(sizeof(share*) * length);
    for(size_t i = 0; i < length; i++) {
        // tmp_srv = bcirc->PutINGate((uint64_t)this->shr_mass[i], bitlen, role);
        // tmp_cli = bcirc->PutDummyINGate(bitlen);
		tmp_srv = PutINGate(bcirc, bitlen, SERVER, (inputtype)this->shr_mass[i]);
		tmp_cli = PutINGate(bcirc, bitlen, CLIENT, (inputtype)this->shr_mass[i]);
        shr_cmb_mass[i] = bcirc->PutADDGate(tmp_srv, tmp_cli);
        // bcirc->PutPrintValueGate(shr_cmb_mass[i], "mass");
        delete tmp_srv, tmp_cli;
    }

    // std::cout << "exit" << std::endl;
	// std::exit(1);

    share** shr_no = (share**) malloc(sizeof(share*) * length);
    for(size_t i = 0; i < length; i ++) {
        shr_no[i] = PutINGate(bcirc, sizeof(size_t), SERVER, (inputtype)i);
        // bcirc->PutPrintValueGate(shr_no[i], "number");
    }

    share *shr_r = bcirc->PutCONSGate(r, bitlen);

/**
 * @param shr_cond1 represents condition value (r < mass[i])
 * @brief 
*/
    share** shr_cond1 = (share**)malloc(sizeof(share*) * length); // r < mass[i]
    for(size_t i = 0; i < length; i ++) {
        // shr_cond1[i] = bcirc->PutGTGate(shr_r, shr_cmb_mass[i]);
        shr_cond1[i] = bcirc->PutGTGate(shr_cmb_mass[i], shr_r);
        bcirc->PutPrintValueGate(shr_cmb_mass[i], "mass");
        // bcirc->PutPrintValueGate(shr_cond1[i], "condition");
    }

/**
 * @param shr_sel select bits - select which  
*/
    share *shr_new, *shr_prev, *shr_inv;
    shr_prev = PutINGate(bcirc, 1, SERVER, (inputtype)0);
    share** shr_sel = (share**) malloc(sizeof(share*) * length);
    for(size_t i = 0; i < length; i ++) {
        shr_inv = bcirc->PutINVGate(shr_prev);
        shr_sel[i] = bcirc->PutANDGate(shr_inv, shr_cond1[i]);
        shr_new = bcirc->PutORGate(shr_prev, shr_sel[i]);
        shr_prev = shr_new;
        bcirc->PutPrintValueGate(shr_sel[i], "selection");
    }

    share*  shr_zero = bcirc->PutCONSGate(0UL, bitlen);
    share*  shr_one = bcirc->PutCONSGate((inputtype)(-1), bitlen);
    // bcirc->PutPrintValueGate(shr_zero, "zero");
    // bcirc->PutPrintValueGate(shr_one, "one");
    share** shr_mask = (share**) malloc(sizeof(share*) * length);
    share** shr_cmb_dataset_masked = (share**) malloc(sizeof(share*) * length);
    share** shr_cmb_gap_masked = (share**) malloc(sizeof(share*) * length);
    share** shr_no_masked = (share**) malloc(sizeof(share*) * length);
    for(size_t i = 0; i < length; i ++) {
        shr_mask[i] = bcirc->PutMUXGate(shr_one, shr_zero, shr_sel[i]);
        // bcirc->PutPrintValueGate(shr_mask[i], "mask");
        shr_cmb_dataset_masked[i] = bcirc->PutANDGate(shr_mask[i], shr_cmb_dataset[i]);
        shr_cmb_gap_masked[i] = bcirc->PutANDGate(shr_mask[i], shr_cmb_gap[i]);
        shr_no_masked[i] = bcirc->PutANDGate(shr_mask[i], shr_no[i]);
        // bcirc->PutPrintValueGate(shr_cmb_dataset_masked[i], "dataset masked");
        // bcirc->PutPrintValueGate(shr_cmb_gap_masked[i], "gap masked");
        // bcirc->PutPrintValueGate(shr_no_masked[i], "number masked");
    }

    share *shr_d = shr_cmb_dataset_masked[0];
    share *shr_g = shr_cmb_gap_masked[0];
    share *shr_j = shr_no_masked[0];
    for(size_t i = 1; i < length; i ++) {
        shr_d = bcirc->PutADDGate(shr_d, shr_cmb_dataset_masked[i]);
        shr_g = bcirc->PutADDGate(shr_g, shr_cmb_gap_masked[i]);
        shr_j = bcirc->PutADDGate(shr_j, shr_no_masked[i]);
    }
    
    shr_d = bcirc->PutOUTGate(shr_d, ALL);
    shr_g = bcirc->PutOUTGate(shr_g, ALL);
    shr_j = bcirc->PutOUTGate(shr_j, ALL);

    party->ExecCircuit();

    outputtype d = shr_d->get_clear_value<outputtype>();
    outputtype g = shr_g->get_clear_value<outputtype>();
    outputtype j = shr_j->get_clear_value<outputtype>();

    std::cout << "d: " << d << ", g: " << g << ", j: " << j << std::endl;

    uint64_t x = this->RandomDraw(g, nonces2);
    if(j < length/2 - 1) {
        std::cout << "Computation Result:" << std::endl;
        std::cout << d + x << std::endl;
        return /*d + x*/;
    }
    else {
        std::cout << "Computation Result:" << std::endl;
        std::cout << d - x << std::endl;
        return /*d - x*/;
    }

    std::cerr << "party execute error" << std::endl;
}

uint64_t Party::RandomDraw(uint64_t M, std::vector<data_t>& nonces)
{
    uint64_t c = 0;
    uint64_t mask = 0;
    for(size_t i = this->bitlen-1; i >= 0; i --) {
        c = (M >> i);
        if(c != 0) {
            mask = (1 << (i+1)) - 1;
            break;
        }
    }

    bool flag(false); // "s" in paper
    uint64_t r;
    for(size_t i = 1; i < m_k; i ++) {
        r = this->xor_nonces(nonces[i]);
		// r = 1000;

        r = r & mask;
        if(r < M) {
            flag = true;
            // std::cout << r << " " << M << std::endl;
            std::clog << "r: " << r << std::endl;
            std::clog << "M: " << M << std::endl;
            break;
        }
    }

    if(flag == false) {
        std::cerr << "RANDOMDRAW ABORT!" << std::endl;
        exit(-1);
    }

    return r;
}
