#ifndef LOCARNA_SCORING_HH
#define LOCARNA_SCORING_HH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <vector>

#include "aux.hh"

#include "scoring_fwd.hh"
#include "matrix.hh"
#include "sequence.hh"
#include "arc_matches.hh"

namespace LocARNA {

    //#define MEA_SCORING_OLD

    class RibosumFreq;
    class Ribofit;
    class Scoring;
    class Sequence;
    class BasePairs;
    class BasePairs__Arc;
    class ArcMatch;
    class MatchProbs;
    class RnaData;

    //! matrix of scores supporting infinity
    typedef std::vector<infty_score_t> ScoreVector;

    //! matrix of scores supporting infinity
    typedef Matrix<infty_score_t> ScoreMatrix;

    //! Matrix of probabilities
    typedef Matrix<double> ProbMatrix;

    /**
     * \brief Parameters for scoring
     *
     * Contains all parameters for doing the scoring of alignments in
     * class Scoring.  The class encapsulates the configuration of
     * the score.
     *
     * @see Scoring
     */
    class ScoringParams {
    public:
        /**
         * constant bonus for a base match.
         * together with basemismatch yields the simplest form
         * of base match/mismatch scoring.
         * Can be replaced by RIBOSUM scores.
         */
        const score_t basematch;

        //! constant cost of a base mismatch
        const score_t basemismatch;

        //! cost per indel (for linear or affine gap cost).
        const score_t indel;

        //! cost per indel for loops (for linear or affine gap cost).
        const score_t indel_loop;

        //! cost per gap (for affine gap-cost). Use affine gap cost if non-zero.
        const score_t indel_opening;

        //! cost per gap for loops(for affine gap-cost). Use affine gap cost if
        //! non-zero.
        const score_t indel_opening_loop;

        /**
         * the ribosum matrix, if non-null (and ribofit==null) it is used
         * for base match/mismatch instead of constant values
         * and as contribution for arc-matchs (tau_factor)
         */
        const RibosumFreq *ribosum;

        /**
         * the ribofit matrix, if non-null it is used
         * for base match/mismatch instead of constant values
         * and as contribution for arc-matchs (tau_factor).
         * Overrides ribosum
         */
        const Ribofit *ribofit;

        //! penalty/cost for unpaired bases matched/mismatched/gapped
        const score_t unpaired_penalty;

        /**
         * Factor for structure contribution in the classic score.
         * Maximal contribution of 1/2 arc match
         */
        const score_t struct_weight;

        /**
         * Factor for the contribution of sequence score or
         * ribosum score to arc matchs
         */
        const score_t tau_factor;

        //! cost of one exclusion.
        const score_t exclusion;

        const double exp_probA;

        const double exp_probB;

        const double temperature_alipf;

        //! turn on/off stacking terms
        const bool stacking;

        //! turn on/off new stacking terms
        const bool new_stacking;

        //! turn on/off mea scoring
        const bool mea_scoring;

        //! weight for mea contribution "unstructured"
        const score_t alpha_factor;

        //! weight for mea contribution "structure"
        const score_t beta_factor;

        //! weight for mea contribution "consensus"
        const score_t gamma_factor;

        /**
         * resolution of the mea score.
         * Since the mea score is composed from
         * probabilities and scores are integral
         * there must be some factor to scale the probabilities
         * in order to get scores.
         */
        const score_t probability_scale;

        /*
          The mea score is composed in the following way:

          params->probability_scale *
          (
          sum_basematchs (i,j)
          P(i~j)
          + alpha_factor/100 * (P(i unstructured) + P(j unstructured))
          +
          sum_arcmatchs (a,b)
          beta_factor/100 * (P(a)+P(b)) * P(al~bl) * P(ar~br) *
          ribosum_arcmatch(a,b)
          )
        */

    public:
        /**
         * Construct with all scoring parameters
         *
         * @param basematch_
         * @param basemismatch_
         * @param indel_
         * @param indel_loop_
         * @param indel_opening_
         * @param indel_opening_loop_
         * @param ribosum_
         * @param ribofit_
         * @param unpaired_penalty_
         * @param struct_weight_
         * @param tau_factor_
         * @param exclusion_
         * @param exp_probA_
         * @param exp_probB_
         * @param temp_
         * @param stacking_
         * @param new_stacking_
         * @param mea_scoring_
         * @param alpha_factor_
         * @param beta_factor_
         * @param gamma_factor_
         * @param probability_scale_
         *
         * @note Store only pointers to objects passed by pointer; the objects
         * must be kept alive by the caller
         */
        ScoringParams(score_t basematch_,
                      score_t basemismatch_,
                      score_t indel_,
                      score_t indel_loop_,
                      score_t indel_opening_,
                      score_t indel_opening_loop_,
                      RibosumFreq *ribosum_,
                      Ribofit *ribofit_,
                      score_t unpaired_penalty_,
                      score_t struct_weight_,
                      score_t tau_factor_,
                      score_t exclusion_,
                      double exp_probA_,
                      double exp_probB_,
                      double temp_,
                      bool stacking_,
                      bool new_stacking_,
                      bool mea_scoring_,
                      score_t alpha_factor_,
                      score_t beta_factor_,
                      score_t gamma_factor_,
                      score_t probability_scale_)
            : basematch(basematch_),
              basemismatch(basemismatch_),
              indel(indel_),
              indel_loop(indel_loop_),
              indel_opening(indel_opening_),
              indel_opening_loop(indel_opening_loop_),
              ribosum(ribosum_),
              ribofit(ribofit_),
              unpaired_penalty(unpaired_penalty_),
              struct_weight(struct_weight_),
              tau_factor(tau_factor_),
              exclusion(exclusion_),
              exp_probA(exp_probA_),
              exp_probB(exp_probB_),
              temperature_alipf(temp_),
              stacking(stacking_),
              new_stacking(new_stacking_),
              mea_scoring(mea_scoring_),
              alpha_factor(alpha_factor_),
              beta_factor(beta_factor_),
              gamma_factor(gamma_factor_),
              probability_scale(probability_scale_) {}
    };

    /**
     * \brief Provides methods for the scoring of alignments
     *
     * Maintains and provides the scores for the alignment of two specific
     * Rnas.
     * Configurable via class ScoringParams, supports
     * the classic LocARNA-score as well as the LocARNA MEA-Score.
     * for "Classic Score" it supports
     *  * match/mismatch cost or RIBOSUM for bases
     *  * linear or affine gap cost
     *  * arc match scores build of arc probabilitiesmatch_probs
     *    and optionally base match contribution or RIBOSUM
     * for the "MEA Score" it supports
     *  * base match/mismatch scores derived from base match probabilities
     *  * RIBOSUM for base and arc match contribution
     *  * arc match score contributions from arc probabilities
     *  * weighting of single score contributions
     *
     * The scoring class supports only characters ACGU, gap -, and N for ribosum
     * scoring.
     * ALL other characters are treated as unknown characters and are basically
     * ignored.
     * In consequence, IUPAC codes are handled badly. Even more important,
     * for ribosum scoring, sequences have to be upper case and Ts should be
     * converted to Us!!!
     *
     * @note The class implements two versions of the stacking score
     * (controlled by ScoringParams flags stacking and
     * new_stacking. The first flag lets the class derive the arc
     * weights for stacked arcs from the conditional probability of
     * the base pair (i,j) if (i+1,j-1) exists. The new_stacking flag
     * adds a log odd contribution derived from the joint probability
     * of (i,j) and (i+1,j-1) to the weight of (i,j).
     *
     * @todo check proper use of unpaired penalty; e.g. is correction
     * needed for arc match (like for lambda)??
     *
     * @note holds only pointers and references to passed objects; since no
     * copies are made,
     * the caller is responsible to keep the objects alive
     *
     * @note the set of possible arc matches is considered part of the scoring
     * information for alignments
     */
    class Scoring {
    public:
        typedef BasePairs__Arc Arc; //!< arc

    protected:
        const ScoringParams *params; //!< a collection of parameters for scoring

        const ArcMatches *arc_matches_; //!< arc matches

        const MatchProbs *match_probs; //!< base match probabilities

        const RnaData &rna_dataA; //!< rna data for RNA A
        const RnaData &rna_dataB; //!< rna data for RNA B
        const Sequence &seqA;     //!< sequence A
        const Sequence &seqB;     //!< sequence B

        /**
         * parameter for modified scoring in normalized local
         * alignment
         */
        score_t lambda_;

    public:
        /**
         * @brief construct scoring object
         *
         * @param seqA first sequence
         * @param seqB second sequence
         * @param rna_dataA probability data of first sequence
         * @param rna_dataB probability data of second sequence
         * @param arc_matches the (significant) arc matches between the
         * sequences
         * @param match_probs pointer to base match probabilities (can be nullptr for
         * non-mea scores)
         * @param params a collection of parameters for scoring
         */
        Scoring(const Sequence &seqA,
                const Sequence &seqB,
                const RnaData &rna_dataA,
                const RnaData &rna_dataB,
                const ArcMatches &arc_matches,
                const MatchProbs *match_probs,
                const ScoringParams &params);

        /**
         * modify scoring by a parameter lambda. Used in the
         * computational of normalized local alignment by fractional
         * programming in the form of the Dinkelbach algorithm.  The
         * modification is destructive.  modify_by_parameter(0) results
         * in unmodified scoring. Repeated modifications are valid.  The
         * Outcome is *undefined* when the Scoring object is used to
         * obtain exponentiated scores (methods exp_...) as in partition
         * function alignment.  @param lambda the parameter
         */
        void
        modify_by_parameter(score_t lambda);

        /**
         * subtract the fixed unpaired_penalty from base match and base indel
         * scores
         * it is similar to @modify_by_parameter method
         * Please note that the base match and gap scores of ALL bases including
         * the paired ones will be modified!
         */
        void
        apply_unpaired_penalty();

        /**
         * @brief Get factor lambda for normalized alignment
         *
         * @return lambda
         */
        score_t
        lambda() const {
            return lambda_;
        }

    protected:
        // ------------------------------
        // tables for precomputed score contributions
        //
        Matrix<score_t>
            sigma_tab; //!< precomputed table of base match similarities

        std::vector<score_t> gapcost_tabA; //!< table for gapcost in A
        std::vector<score_t> gapcost_tabB; //!< table for gapcost in B

        std::vector<score_t> weightsA; //<! weights of base pairs in A
        std::vector<score_t> weightsB; //<! weights of base pairs in B

        std::vector<score_t> stack_weightsA; //<! weights of stacked base
        //<! pairs in A
        std::vector<score_t> stack_weightsB; //<! weights of stacked base
        //<! pairs in B

        Matrix<size_t> identity; //!< sequence identities in percent

        void
        precompute_sequence_identities();

        /**
         * \brief Round a double to score_t.
         *
         * Proper rounding is more robust than truncate
         *
         * @param d score of type double
         * @return d rounded to integral type score_t
         */
        score_t
        round2score(double d) const {
            return (score_t)((d < 0) ? (d - 0.5) : (d + 0.5));
        }

        /**
         * \brief Compute base similarity
         *
         * @param i position in A
         * @param j position in B
         *
         * @return similarity of i and j
         * @note used for precomputing the score, which is then tabellized
         */
        score_t
        sigma_(int i, int j) const;

        /**
         * \brief Precompute all base similarities
         *
         * Precomputed similarities are stored in the sigma table.
         */
        void
        precompute_sigma();

        //! \brief Precompute the tables for gapcost
        void
        precompute_gapcost();

        //! \brief Precompute weights/stacked weights for all arcs in A and B
        void
        precompute_weights();

        /**
         *  \brief Helper for precompute_weights (does job for one rna)
         *
         * @param rna_data rna probability data
         * @param bps base pairs
         * @param exp_prob background probability
         * @param weights[out]        base pair score weights
         * @param stack_weights[out]  base pair stacking score weights
         */
        void
        precompute_weights(const RnaData &rna_data,
                           const BasePairs &bps,
                           double exp_prob,
                           std::vector<score_t> &weights,
                           std::vector<score_t> &stack_weights);

        /**
         * @brief convert probability to weight for scoring
         *
         * @param p probability
         * @param exp_prob background probability
         *
         * @return In standard case, normlized log of probability; in case
         * of mea, score_res*probability.
         */
        score_t
        probToWeight(double p, double prob_exp) const;

        /**
         * returns probability of matching the concrete bases in an arcmatch,
         * probability is based on ribosum data
         */
        double
        ribosum_arcmatch_prob(const Arc &arcA, const Arc &arcB) const;

        /**
         * @brief ribofit or ribosum arcmatch score contribution
         *
         * Sequence-based score contribution to match of the arcs due
         * to ribofit matrix (if ribofit enabled) or ribosum matrix.
         * The score contribution is calculated as sum-of-pairs; gaps
         * and non-standard symbols (different from ACGU) are ignored!
         *
         * @param arcA arc A
         * @param arcB arc B
         *
         * @return score contribution
         */
        score_t
        riboX_arcmatch_score(const Arc &arcA, const Arc &arcB) const;

        //! subtract from each element of a score_t vector v a value x
        void
        subtract(std::vector<score_t> &v, score_t x) const;

        //! subtract from each element of a score_t Matrix m a value x
        void
        subtract(Matrix<score_t> &m, score_t x) const;

    public:
        // ------------------------------------------------------------
        // SCORE CONTRIBUTIONS

        /**
         * \brief Score of a match of bases (without structure)
         *
         * @param i position in A
         * @param j position in B
         *
         * @return score of base match i~j
         */
        score_t
        basematch(size_type i, size_type j) const {
            return sigma_tab(i, j);
        }

        /**
         * @brief Score of arc match, support explicit arc match scores
         *
         * @param am arc match
         * @param stacked is stacked? (optional parameter)
         *
         * @return Score of arc match am (potentially stacked)
         *
         * @note This method supports explicit arc match scores. Such
         * scores are required for the probabilistic mode of
         * (m)locarna.
         *
         * @note This method should be prefered over arcmatch(const
         * Arc &arcA, const Arc &arcB, bool stacked=false) unless
         * explicit arc match scores are not needed and the use of
         * this method would be too inefficient.
         */
        score_t
        arcmatch(const ArcMatch &am, bool stacked = false) const;

        /**
         * @brief Score of arc match, given two arcs.
         *
         * @param arcA base pair in A
         * @param arcB base pair in B
         * @param stacked is stacked? (optional parameter)
         *
         * @return Score of arc match am (potentially stacked)
         *
         * @note This method does not support explicit arc match
         * scores, since this would require to know the arcmatch
         * object.
         *
         * @note Using this method is disallowed if
         * arc_matches->explicit_scores()==true (This results in a
         * run-time error if !NDEBUG).
         */
        score_t
        arcmatch(const BasePairs__Arc &arcA,
                 const BasePairs__Arc &arcB,
                 bool stacked = false) const;

        /** Read access to the stored arc_matches pointer
         */
        const ArcMatches *
        arc_matches() const {
            return arc_matches_;
        }

        /**
         * @brief Very basic interface, score of aligning a basepair to gap
         *
         * @note gapAorB whether deleting element in A or B (true for A)
         * @param arc the base pair
         * @param stacked whether the base pair is stacked
         *
         * @return
         */
        template <bool gapAorB>
        score_t
        arcDel(const BasePairs__Arc &arc, bool stacked = false) const;

        /**
         * @brief Score of stacked arc match
         *
         * @param am arc match
         *
         * @return Score of arc match am when stacked
         */
        score_t
        arcmatch_stacked(const ArcMatch &am) const {
            return arcmatch(am, true);
        }

        /**
         * Score of deletion
         *
         * @param alignedToGap position in A or B
         * @param gapInA whether symbol in A (true) or B (false) is aligned to
         * gap
         *
         * @return score of deletion : posA <--> posB
         */
        template <bool gapInA>
        inline
        score_t
        gapX(size_type alignedToGap) const;

        /**
         * Score of deletion
         *
         * @param posA position in A
         *
         * @return score of deletion of posA after posB
         */
        score_t
        gapA(size_type posA) const {
            assert(1 <= posA && posA <= seqA.length());

            return gapcost_tabA[posA];
        }

        /**
         * Score of insertion
         *
         * @param posB position in B
         *
         * @return score of insertion of posB after posA
         */
        score_t
        gapB(size_type posB) const {
            assert(1 <= posB && posB <= seqB.length());

            return gapcost_tabB[posB];
        }

        //! cost of an exclusion
        score_t
        exclusion() const {
            return params->exclusion;
        }

        //! cost to begin a new indel
        score_t
        indel_opening() const {
            return params->indel_opening;
        }

        //! multiply an score by the ratio of indel_loop/indel
        score_t
        loop_indel_score(const score_t score) const {
            return round2score(score * params->indel_loop / params->indel);
        }
        //! cost to begin a new indel
        score_t
        indel_opening_loop() const {
            return params->indel_opening_loop;
        }

        //
        // ------------------------------------------------------------

        /**
         * @brief Expected base pair probability
         *
         * @param len length of RNA sequence
         *
         * @return expected base pair probability for sequence length
         *
         * @note influences computation of weights for base pairs
         */
        double
        prob_exp(size_type len) const;

        /**
         * @brief Query stacking flag
         *
         * @return flag, whether stacking is used (old or new stacking terms)
         */
        bool
        stacking() const {
            return params->stacking || params->new_stacking;
        }

        /**
         * @brief Is arc of A stackable
         *
         * @param a Arc
         *
         * @return whether stackable
         */
        bool
        is_stackable_arcA(const Arc &a) const;

        /**
         * @brief Is arc of B stackable
         *
         * @param a Arc
         *
         * @return whether stackable
         */
        bool
        is_stackable_arcB(const Arc &a) const;

        /**
         * @brief Is arc match stackable
         *
         * @param am Arc match
         *
         * @return whether stackable
         */
        bool
        is_stackable_am(const ArcMatch &am) const;

    }; // end class Scoring

    template <bool isA>
    score_t
    Scoring::arcDel(const Arc &arcX,
                    bool stacked) const { // TODO Important Scoring scheme for
                                          // aligning an arc to a gap is not
                                          // defined and implemented!

        if (arc_matches_
                ->explicit_scores()) { // will not take stacking into account!!!
            std::cerr
                << "ERROR sparse explicit scores is not supported!"
                << std::endl; // TODO: Supporting explicit scores for arcgap
            assert(!arc_matches_->explicit_scores());
        }

        if (!params->mea_scoring) {
            return
                // base pair weights
                loop_indel_score(
                    gapX<isA>(arcX.left()) +
                    gapX<isA>(arcX.right())) + // score of aligining base-pair
                                               // ends wo gap, such that it is
                                               // multiply by gamma_loop ratio
                (stacked ? (isA ? stack_weightsA[arcX.idx()]
                                : stack_weightsB[arcX.idx()])
                         : (isA ? weightsA[arcX.idx()] : weightsB[arcX.idx()]));
        } else {
            std::cerr << "ERROR sparse mea_scoring is not supported!"
                      << std::endl; // TODO: Supporting mea_scoring for arcgap
            assert(!params->mea_scoring);
            return 0;
        }
    }

    template <>
    inline
    score_t
    Scoring::gapX<true>(size_type alignedToGap) const {
        return gapA(alignedToGap);
    }

    template <>
    inline
    score_t
    Scoring::gapX<false>(size_type alignedToGap) const {
        return gapB(alignedToGap);
    }


    /**
     * Scoring for partition function alignment
     *
     * @param T type of the partition functions (like float, double, or
     * long double)
     */
    template<typename T>
    class PFScoring : public Scoring {
        public:
        using pf_score_t = T;

        //! Vector of partition functions
        using PFScoreVector = std::vector<pf_score_t>;

        //! Matrix of partition functions
        using PFScoreMatrix = Matrix<pf_score_t>;


        /**
         * @brief construct scoring object
         *
         * @param seqA first sequence
         * @param seqB second sequence
         * @param rna_dataA probability data of first sequence
         * @param rna_dataB probability data of second sequence
         * @param arc_matches the (significant) arc matches between the
         * sequences
         * @param match_probs pointer to base match probabilities (can be nullptr for
         * non-mea scores)
         * @param params a collection of parameters for scoring
         */
        PFScoring(const Sequence &seqA,
                const Sequence &seqB,
                const RnaData &rna_dataA,
                const RnaData &rna_dataB,
                const ArcMatches &arc_matches,
                const MatchProbs *match_probs,
                const ScoringParams &params);

        /**
         * \brief Boltzmann weight of score of a base match (without structure)
         *
         * @param i position in A
         * @param j position in B
         *
         * @return Boltzmann weight of score of base match i~j
         */
        pf_score_t
        exp_basematch(size_type i, size_type j) const {
            return exp_sigma_tab(i, j);
        }

        /**
         * @brief Boltzmann weight of score of arc match
         *
         * @param am arc match
         *
         * @return Boltzmann weight of score of arc match am
         */
        pf_score_t
        exp_arcmatch(const ArcMatch &am) const {
            return boltzmann_weight(arcmatch(am));
        }

        /**
         * @brief Boltzmann weight of score of deletion
         *
         * @param posA position in A
         *
         * @return Boltzmann weight of score of deletion of posA after posB
         */
        pf_score_t
        exp_gapA(size_type posA) const {
            assert(1 <= posA && posA <= seqA.length());
            return exp_gapcost_tabA[posA];
        }

        /**
         * @brief Boltzmann weight of score of insertion
         *
         * @param posB position in B
         *
         * @return Boltzmann weight of score of insertion of posB after posA
         */
        pf_score_t
        exp_gapB(size_type posB) const {
            assert(1 <= posB && posB <= seqB.length());

            return exp_gapcost_tabB[posB];
        }

        //! exp of cost to begin a new indel
        pf_score_t
        exp_indel_opening() const {
            return exp_indel_opening_score;
        }

        //! exp of cost to begin a new indel in loops
        pf_score_t
        exp_indel_opening_loop() const {
            return exp_indel_opening_loop_score;
        }


    protected:
        //! \brief Precompute the tables for Boltzmann weights of gapcost
        void
        precompute_exp_gapcost();


        /**
         * \brief Precompute all Boltzmann weights of base similarities
         *
         * Precomputed similarities are stored in the exp_sigma table.
         */
        void
        precompute_exp_sigma();


        pf_score_t
        boltzmann_weight(score_t s) const {
            return exp(s / (pf_score_t)params->temperature_alipf);
        }


    private:
        // ------------------------------
        // tables for precomputed exp score contributions for partition function
        //
        Matrix<pf_score_t>
            exp_sigma_tab; //!< precomputed table of exp base match similarities
        pf_score_t exp_indel_opening_score; //!< precomputed value for exp of
                                            //! indel opening cost
        pf_score_t exp_indel_opening_loop_score; //!< precomputed value for exp
                                                 //! of indel opening cost for
        //! loops
        std::vector<pf_score_t>
            exp_gapcost_tabA; //!< table for exp gapcost in A
        std::vector<pf_score_t>
            exp_gapcost_tabB; //!< table for exp gapcost in B

    };

    template <typename T>
    PFScoring<T>::PFScoring(const Sequence &seqA,
                         const Sequence &seqB,
                         const RnaData &rna_dataA,
                         const RnaData &rna_dataB,
                         const ArcMatches &arc_matches,
                         const MatchProbs *match_probs,
                         const ScoringParams &params)
        : Scoring(seqA,seqB,rna_dataA,rna_dataB,
                  arc_matches,match_probs,params) {

        exp_indel_opening_score = boltzmann_weight(params.indel_opening);
        exp_indel_opening_loop_score =
            boltzmann_weight(params.indel_opening_loop);
        precompute_exp_sigma();
        precompute_exp_gapcost();
    }

    template <typename T>
    void
    PFScoring<T>::precompute_exp_sigma() {
        size_type lenA = seqA.length();
        size_type lenB = seqB.length();

        exp_sigma_tab.resize(lenA + 1, lenB + 1);

        for (size_type i = 1; i <= lenA; ++i) {
            for (size_type j = 1; j <= lenB; ++j) {
                exp_sigma_tab(i, j) = boltzmann_weight(sigma_tab(i, j));
            }
        }
    }

    template <typename T>
    void
    PFScoring<T>::precompute_exp_gapcost() {
        size_type lenA = seqA.length();
        size_type lenB = seqB.length();

        // resize and create tables
        exp_gapcost_tabA.resize(lenA + 1);
        exp_gapcost_tabB.resize(lenB + 1);

        for (size_type i = 1; i < lenA + 1; i++) {
            exp_gapcost_tabA[i] = boltzmann_weight(gapcost_tabA[i]);
        }

        for (size_type i = 1; i < lenB + 1; i++) {
            exp_gapcost_tabB[i] = boltzmann_weight(gapcost_tabB[i]);
        }
    }

} // end namespace LocARNA

#endif // LOCARNA_SCORING_HH
