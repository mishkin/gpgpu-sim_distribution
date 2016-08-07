#ifndef READ_TRACKING_H_
#define READ_TRACKING_H_

#include <vector>
#include <map>
#include <set>
#include "../abstract_hardware_model.h"

enum warmode {
  no_read_tracking = 0,
  release_on_commit = 1,
  release_on_dispatch = 2,
};

class Read_tracking {
 public:
  Read_tracking(unsigned n_warps, warmode release_mode, unsigned bloom_size, unsigned counter_max);
  ~Read_tracking();

  void Read_release(const warp_inst_t *inst, warmode mode);
  void Read_reserve(const warp_inst_t *inst);
  bool checkReadCollision(unsigned wid, unsigned regnum) const;
  bool checkWriteCollision(unsigned wid, unsigned regnum) const;
 private:
  void Read_release_reg(unsigned wid, unsigned regnum);
  void Read_reserve_reg(unsigned wid, unsigned regnum);
  unsigned hash_reg(unsigned wid, unsigned regnum) const;

  warmode m_release_mode;
  unsigned m_bloom_size;
  unsigned m_counter_max;

  std::vector< std::map<unsigned, unsigned>* > m_reg_table;
  std::set<unsigned> m_aa_key_set;
};

#endif
