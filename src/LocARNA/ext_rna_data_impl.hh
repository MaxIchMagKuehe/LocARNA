#ifndef LOCARNA_EXT_RNA_DATA_IMPL_HH
#define LOCARNA_EXT_RNA_DATA_IMPL_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iosfwd>
#include "ext_rna_data.hh"
#include "sequence.hh"
#include "sparse_vector.hh"

namespace LocARNA {

    // class Sequence;
    // class RnaEnsemble;
    // class PFoldParams;

    /**
     * @brief Implementation of ExtRnaData
     */
    struct ExtRnaDataImpl {

	// ----------------------------------------
	// TYPES
	
	//! vector of arc probabilities
	typedef SparseVector<double> arc_prob_vector_t;
	
	//! matrix of arc probabilities
	typedef RnaDataImpl::arc_prob_matrix_t arc_prob_matrix_t;
	
	//! matrix of arc probability vectors
	typedef SparseMatrix<arc_prob_vector_t> arc_prob_vector_matrix_t;

	//! matrix of arc probability matrices
	typedef SparseMatrix<arc_prob_matrix_t> arc_prob_matrix_matrix_t;
	
	
	// ----------------------------------------
	// ATTRIBUTES

	ExtRnaData *self_; //!<- pointer to corresponding non-impl object
	
	//! cutoff probabilitiy for base pair in loop
	double p_bpilcut_;
	
	//! cutoff probabilitiy for unpaired base in loop
	double p_uilcut_;
	
	//! in loop probabilities of base pairs
	arc_prob_matrix_matrix_t arc_in_loop_probs_;

	//! in loop probabilities of unpaired bases
	arc_prob_vector_matrix_t unpaired_in_loop_probs_;

	//! used in initialization, to check whether in loop probs
	//! still have to be computed
	bool has_in_loop_probs_;
	
	// ----------------------------------------
	// CONSTRUCTORS
	
	/** 
	 * @brief Construct from RnaEnsemble with cutoff probability
	 * 
	 * @param self self pointer
	 * @param rna_data data about RNA ensemble
	 * @param p_bpcut cutoff probability
	 * @param p_bpilcut cutoff probability base pair in loop
	 * @param p_uilcut cutoff probability unpaired in loop
	 */
	ExtRnaDataImpl(RnaData *self,
		       const RnaEnsemble &rna_data,
		       double p_bpcut,
		       double p_bpilcut,
		       double p_uilcut);
	
	/** 
	 * @brief Construct from input file
	 * 
	 * @param self self pointer
	 * @param filename input file name
	 * @param p_bpilcut probability cutoff base pair in loop 
	 * @param p_uilcut  probability cutoff unpaired in loop 
	 * @param pfoldparams partition folding parameters
	 *
	 * @note autodetects file format, see corresponding RnaDataImpl constructor
	 */
	ExtRnaDataImpl(ExtRnaData *self, 
		       const std::string &filename,
		       double p_bpilcut,
		       double p_uilcut,
		       const PFoldParams &pfoldparams);

	/** 
	 * @brief Construct from rna ensemble
	 * 
	 * @param self self pointer
	 * @param rna_ensemble rna ensemble
	 * @param p_bpilcut probability cutoff base pair in loop 
	 * @param p_uilcut  probability cutoff unpaired in loop 
	 *
	 * @note autodetects file format, see corresponding RnaDataImpl constructor
	 */
	ExtRnaDataImpl(ExtRnaData *self,
		       const RnaEnsemble &rna_ensemble,
		       double p_bpilcut,
		       double p_uilcut);
	
	// ----------------------------------------
	// METHODS
	

	/** 
	 * @brief initialize from rna ensemble 
	 * 
	 * @param rna_ensemble rna ensemble
	 * 
	 * @note overloaded to initialize with additional
	 * information (in loop probabilities)
	 * @note rna_ensemble must have in loop probabilities 
	 */
	void
	init_from_ext_rna_ensemble(const RnaEnsemble &rna_ensemble);	

	/**
	 * @brief read in loop probability section of pp-format
	 *
	 * @param in input stream
	 * @return stream
	 *
	 * Reads only base pairs with probabilities greater than
	 * p_bpcut_; base pairs in loops, p_bpilcut_; unpaired
	 * bases in loops, p_uilcut_
	 */
	std::istream &
	read_pp_in_loop_probabilities(std::istream &in);

	
	/** 
	 * @brief Write in loop probability section of pp 2.0
	 * 
	 * @param out output stream 
	 * @param p_outbpcut base pair probability cutoff
	 * @param p_outbpilcut base pair in loop probability cutoff
	 * @param p_outuilcut unpaired in loop probability cutoff
	 * 
	 * @return output stream
	 */
	std::ostream &
	write_pp_in_loop_probabilities(std::ostream &out,
				       double p_outbpcut,
				       double p_outbpilcut,
				       double p_outuilcut
				       ) const;
	
	/** 
	 * @brief Write in loop base pair probabilities for a specific base pair
	 * 
	 * @param out output stream 
	 * @param probs in loop probability matrix of a base pair
	 * @param p_cut base pair in loop probability cutoff
	 * 
	 * @return output stream
	 */
	std::ostream &
	write_pp_basepair_in_loop_probabilities(std::ostream &out,
						const arc_prob_matrix_t &probs,
						double p_cut) const;
	
	/** 
	 * @brief Write in loop unpaired probabilities for a specific base pair
	 * 
	 * @param out output stream 
	 * @param probs in loop probability vector of a base pair
	 * @param p_cut unpaired in loop probability cutoff
	 * 
	 * @return output stream
	 */
	std::ostream &
	write_pp_unpaired_in_loop_probabilities(std::ostream &out,
						const arc_prob_vector_t &probs,
						double p_cut) const;
    }; // end ExtRnaDataImpl


} //end namespace LocARNA


#endif // LOCARNA_EXT_RNA_DATA_IMPL_HH