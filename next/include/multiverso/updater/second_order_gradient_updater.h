#ifndef MULTIVERSO_UPDATER_SECOND_ORDER_GRADIENT_UPDATER_H_
#define MULTIVERSO_UPDATER_SECOND_ORDER_GRADIENT_UPDATER_H_

#include "updater.h"

#include <cmath>

namespace multiverso {

  // [TODO(qiwye)]:rename the class to Shuxin Zheng's algorithms
  template <typename T>
  class SecondOrderUpdater : public Updater<T> {
  public:
    explicit SecondOrderUpdater(size_t size) : 
      size_(size) {
      shadow_copies_.resize(MV_NumWorkers());
      for (auto s : shadow_copies_){
        s.resize(size);
      }
      historic_g_sqr_.resize(MV_NumWorkers());
      for (auto s : historic_g_sqr_){
        s.resize(size);
      }
    }
    void Update(size_t num_element, T*data, T*delta,
      UpdateOption* option, size_t offset) override {
      
      for (size_t index = 0; index < num_element; ++index) {
        // gsqr = (1 - r) * g^2 + r * gsqr  
        historic_g_sqr_[option->worker_id()][index + offset] = 
          (1 - option->rho()) * delta[index] * delta[index] 
          / option->learning_rate() / option->learning_rate() +
          option->rho() * historic_g_sqr_[option->worker_id()][index + offset];

        data[index + offset] -= delta[index] + option->lambda() * 
          std::sqrt(historic_g_sqr_[option->worker_id()][index + offset]) *
          (data[index + offset] - shadow_copies_[option->worker_id()][index + offset]);

        // caching each worker's latest version of parameter
        shadow_copies_[option->worker_id()][index + offset] = data[index + offset];
      }
    }
    ~SecondOrderUpdater() { 
      shadow_copies_.clear();
      historic_g_sqr_.clear();
    }
  protected:
    std::vector< std::vector<T>> shadow_copies_;
    std::vector< std::vector<T>> historic_g_sqr_;

    // move these parameter to UpdateOption
    size_t size_;
  };

}

#endif // MULTIVERSO_UPDATER_SECOND_ORDER_GRADIENT_UPDATER_H_