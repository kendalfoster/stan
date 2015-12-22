#include <gtest/gtest.h>
#include <stan/interface_callbacks/writer/stream_writer.hpp>
#include <stan/services/optimize/do_bfgs_optimize.hpp>
#include <stan/optimization/bfgs.hpp>
#include <test/test-models/good/optimization/rosenbrock.hpp>
#include <boost/random/additive_combine.hpp>
#include <boost/random/uniform_real_distribution.hpp>

typedef rosenbrock_model_namespace::rosenbrock_model Model;
typedef boost::ecuyer1988 rng_t; // (2**50 = 1T samples, 1000 chains)
typedef stan::interface_callbacks::writer::noop_writer writer_t;

class mock_interrupt: public stan::interface_callbacks::interrupt::base_interrupt {
public:
  mock_interrupt(): n_(0) {}

  void operator()() {
    n_++;
  }

  void clear() {
    n_ = 0;
  }

  int n() {
    return n_;
  }

private:
  int n_;
};

TEST(Services, do_bfgs_optimize__bfgs) {
  typedef stan::optimization::BFGSLineSearch
    <Model, stan::optimization::BFGSUpdate_HInv<> > Optimizer_BFGS;

  std::vector<double> cont_vector(2);
  cont_vector[0] = -1; cont_vector[1] = 1;
  std::vector<int> disc_vector;

  static const std::string DATA("");
  std::stringstream data_stream(DATA);
  stan::io::dump dummy_context(data_stream);
  Model model(dummy_context);

  Optimizer_BFGS bfgs(model, cont_vector, disc_vector, 0);

  double lp = 0;
  bool save_iterations = true;
  int refresh = 0;
  int return_code;
  unsigned int random_seed = 0;
  rng_t base_rng(random_seed);

  mock_callback callback;

  stan::interface_callbacks::writer::stream_writer writer(out);
  std::stringstream info_ss;
  stan::interface_callbacks::writer::stream_writer info(info_ss);
  return_code = stan::services::optimize::do_bfgs_optimize(model, bfgs, base_rng,
                                                           lp, cont_vector, disc_vector,
                                                           writer, info,
                                                           save_iterations, refresh,
                                                           callback);
  EXPECT_EQ("initial log joint probability = -4\nOptimization terminated normally: \n  Convergence detected: relative gradient magnitude is below tolerance\n", info_ss.str());
  EXPECT_FLOAT_EQ(return_code, 0);
  EXPECT_EQ(33, interrupt.n());
}

TEST(Services, do_bfgs_optimize__lbfgs) {
  std::vector<double> cont_vector(2);
  cont_vector[0] = -1; cont_vector[1] = 1;
  std::vector<int> disc_vector;

  static const std::string DATA("");
  std::stringstream data_stream(DATA);
  stan::io::dump dummy_context(data_stream);
  Model model(dummy_context);

  typedef stan::optimization::BFGSLineSearch<Model,stan::optimization::LBFGSUpdate<> > Optimizer_LBFGS;
  Optimizer_LBFGS lbfgs(model, cont_vector, disc_vector, &std::cout);


  double lp = 0;
  bool save_iterations = true;
  int refresh = 0;
  int return_code;
  unsigned int random_seed = 0;
  rng_t base_rng(random_seed);

  mock_callback callback;

  stan::interface_callbacks::writer::stream_writer writer(out);
  std::stringstream info_ss;
  stan::interface_callbacks::writer::stream_writer info(info_ss);
  return_code = stan::services::optimize::do_bfgs_optimize(model, lbfgs, base_rng,
                                                           lp, cont_vector, disc_vector,
                                                           writer, info,
                                                           save_iterations, refresh,
                                                           callback);
  EXPECT_EQ("initial log joint probability = -4\nOptimization terminated normally: \n  Convergence detected: relative gradient magnitude is below tolerance\n", info_ss.str());
  EXPECT_FLOAT_EQ(return_code, 0);
  EXPECT_EQ(35, callback.n);
}
