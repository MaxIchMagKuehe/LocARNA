#ifndef LOCARNA_FIT_ON_OFF_HH
#define LOCARNA_FIT_ON_OFF_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string>
#include <vector>

#include <cmath>

namespace LocARNA {

    typedef std::vector<double> numseq_t;
    typedef std::vector<double>::size_type size_type;

    typedef long double pf_t;

    /**
     * \brief Implements fitting of a two-step function to a number sequence
     *
     */
    class FitOnOff {
        double delta_01;
        double delta_10;
        numseq_t x;

        double beta;

        pf_t exp_delta_01; //!< exp(-beta*delta_01)
        pf_t exp_delta_10; //!< exp(-beta*delta_10)

        std::vector<std::vector<pf_t> > v; //!< score/pf vectors 0 and 1

        std::vector<std::vector<bool> > t; //!< trace vectors 0 and 1

        std::vector<bool> trace;


    public:

        /**
         * construct with parameters
         * @param x_ number sequence that we want to fit
         * @param delta_01_ penalty for change from a to b
         * @param delta_10_ penalty for change from b to a
         * @param beta_ is the inverse temperature
         */
        FitOnOff(numseq_t &x_, double delta_01_, double delta_10_, double beta_)
            : delta_01(delta_01_),
              delta_10(delta_10_),
              x(x_),
              beta(beta_)
        {
            v.resize(2);
            v[0].resize(x.size()+1);
            v[1].resize(x.size()+1);

            t.resize(2);
            t[0].resize(x.size()+1);
            t[1].resize(x.size()+1);
            trace.resize(x.size()+1);

            exp_delta_01 = exp(-beta*delta_01);
            exp_delta_10 = exp(-beta*delta_10);

        }

        /**
         * compute the viterbi score (and optionally path)
         * fills tables v, optionally compute t and trace
         * @param c0 off-value
         * @param c1 on-value
         * @param traceback whether to perform traceback
         */
        double
        viterbi(double c0, double c1, bool traceback);

        /**
         * best path that is "on" (=c1) exactly once
         * @param c0 off-value
         * @param c1 on-value
         * @return score of best path
         * post: best path is in trace
         */
        double
        best_once_on(double c0, double c1);


        /**
         * compute forward partition functions
         * fills tables v
         */
        pf_t
        forward(double c0, double c1);


        /**
         * optimize c0 and c1 by gradient optimization
         * @return optimal c0 and c1
         */
        std::pair<double,double>
        optimize(double c0, double c1);

        //! writes the ranges in the viterbi path
        void
        write_viterbi_path_compact(std::ostream &out,double c0, double c1);

        //! writes the viterbi path
        void
        write_viterbi_path(std::ostream &out,double c0, double c1) const;

        // DEBUGGING

        /**
         * Print boolean vector to cout
         *
         * @param name Identifier name to be printed
         * @param v    Vector
         */
        void
        print_table(const std::string &name, const std::vector<bool> &v) const;

        /**
         * Print vector of partition functions (pf_t) to cout
         *
         * @param name Identifier name to be printed
         * @param v    Vector
         */
        void
        print_table(const std::string &name, const std::vector<pf_t> &v) const;

        /**
         * Print DP-tables for debugging to cout
         * @see print_table()
         */
        void
        print_tables() const;


    };

} // END namespace LocARNA

#endif //LOCARNA_FIT_ON_OFF_HH
