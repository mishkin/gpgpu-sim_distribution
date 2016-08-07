#include "read_tracking.h"

//Cosntructor
Read_tracking::Read_tracking( unsigned n_warps, warmode release_mode, unsigned bloom_size, unsigned counter_max ) : m_release_mode(release_mode), m_bloom_size(bloom_size), m_counter_max(counter_max)
{
  m_reg_table.resize(n_warps);
  for(unsigned i=0;i<n_warps; i++)
    m_reg_table[i] = new std::map<unsigned,unsigned>;
}

 
unsigned Read_tracking::hash_reg(unsigned wid, unsigned regnum) const 
{
  if(m_bloom_size != 0)
    return regnum % m_bloom_size;
  else
    return regnum;
}

void Read_tracking::Read_reserve_reg(unsigned wid, unsigned regnum)
{
  if(m_release_mode != no_read_tracking) {
    std::map<unsigned,unsigned>& warp_reg_table = *m_reg_table[wid];

    unsigned key = hash_reg(wid,regnum);

    if(!warp_reg_table.count(key))
      warp_reg_table[key] = 1;
    else if(m_aa_key_set.find(key) == m_aa_key_set.end())
      warp_reg_table[key] = warp_reg_table[key] + 1;
    else
      return;

    m_aa_key_set.insert(key);
  }
}

void Read_tracking::Read_release_reg(unsigned wid, unsigned regnum)
{
  if(regnum > 0 && m_release_mode != no_read_tracking) {
    std::map<unsigned, unsigned>& warp_reg_table = *m_reg_table[wid];
    unsigned key = hash_reg(wid, regnum);
    if(!warp_reg_table.count(key))
      abort();

    if(m_aa_key_set.find(key) == m_aa_key_set.end()) {
      warp_reg_table[key] = warp_reg_table[key] - 1;
      m_aa_key_set.insert(key);
    }
  }
}

void Read_tracking::Read_release(const class warp_inst_t *inst, warmode mode) 
{
  if(mode != m_release_mode)
    return;

  assert(inst != NULL);

  unsigned wid = inst->warp_id();

  for( unsigned r=0; r < 4; r++)
    Read_release_reg(wid, inst->in[r]);
    
  Read_release_reg(wid, inst->ar1);
  Read_release_reg(wid, inst->ar2);
  m_aa_key_set.clear();
}

void Read_tracking::Read_reserve(const warp_inst_t *inst)
{
  if (m_release_mode != no_read_tracking) {
    for( unsigned r=0; r < 4; r++) {
        if(inst->in[r] > 0) {
            Read_reserve_reg(inst->warp_id(), inst->in[r]);
        }
    }
    if(inst->ar1 > 0) {
      Read_reserve_reg(inst->warp_id(), inst->ar1);
    }
    if(inst->ar2 > 0) {
      Read_reserve_reg(inst->warp_id(), inst->ar2);
    }
    m_aa_key_set.clear();
  }
}

bool Read_tracking::checkReadCollision(unsigned wid, unsigned regnum) const
{
  if((m_release_mode != no_read_tracking) && (m_counter_max > 0)) {
    std::map<unsigned,unsigned>::iterator it;
    unsigned key = hash_reg(wid,regnum);
    it = m_reg_table[wid]->find(key);
    if ( it != m_reg_table[wid]->end() && it->second >= m_counter_max )
      return true;
  }
  return false;
}

bool Read_tracking::checkWriteCollision(unsigned wid, unsigned regnum) const
{
  if(m_release_mode != no_read_tracking) {
    std::map<unsigned,unsigned>::iterator it;
    unsigned key = hash_reg(wid,regnum);
    it = m_reg_table[wid]->find(key);
    if ( it != m_reg_table[wid]->end() && it->second != 0 )
      return true;
  }
  return false;
}
