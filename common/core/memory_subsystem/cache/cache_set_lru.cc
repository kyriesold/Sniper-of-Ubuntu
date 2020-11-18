#include "cache_set_lru.h"
#include "log.h"
#include "stats.h"
#include <fstream>
#include <iostream>   //箭头一般是系统库
#include <vector>
#include "cache.h"
using namespace std;
//定义一个检测未来序列的条数
int num_lookahead=8000;
int counter_access=0;//记录当前访问的指令是第几条
UInt32 set_index=Cache::future_list[counter_access][0];  //当前指令set_index
IntPtr tag=Cache::future_list[counter_access][1];  //当前指令的tag

// Implements LRU replacement, optionally augmented with Query-Based Selection [Jaleel et al., MICRO'10]

CacheSetLRU::CacheSetLRU(
      CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize, CacheSetInfoLRU* set_info, UInt8 num_attempts)
   : CacheSet(cache_type, associativity, blocksize)
   , m_num_attempts(num_attempts)
   , m_set_info(set_info)
{
   m_opt_bits = new UInt32[m_associativity]; //m_associativity应该代表m_cache_block_info_array的大小
   for (UInt32 i = 0; i < m_associativity; i++)
      m_opt_bits[i] = i+num_lookahead;   //给一个初值
      //printf(" %d %d %d %d %d %d %d %d |||\n",m_lru_bits[0],m_lru_bits[1],m_lru_bits[2],m_lru_bits[3],m_lru_bits[4],m_lru_bits[5],m_lru_bits[6],m_lru_bits[7]);
}

CacheSetLRU::~CacheSetLRU()
{
   delete [] m_opt_bits;   //及时释放以避免内存泄露
}
static long long int all_set_opt_bits[512][8]={0}; //初始化


//int count_get=0;    //用于调试的时候计数
UInt32
CacheSetLRU::getReplacementIndex(CacheCntlr *cntlr)   //所以这个函数的作用是找到需要替换的Cache block的index
{  
   //count_get++;
   //找到空的块就可以直接返回了
   
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (!m_cache_block_info_array[i]->isValid())   //IsValid应该代表这个块刚被插进来或者使用 is newly-inserted line
      {
         //*****写find the next   记录一下这个块的next访问
         m_opt_bits[i]=FindtheNextAccess(counter_access);  //记录这个块的next  我相当于把lru的这个数组改成了next数组 使用现成的
         all_set_opt_bits[set_index][i]=m_opt_bits[i];   //大数组的也要更新
         return i;
      }
   }
   //更新这个即将被替换的block的位置的m_lru_bits[i]
   for(UInt8 attempt = 0; attempt < m_num_attempts; ++attempt)
   {
      UInt32 index = 0;
      UInt8 max_bits = 0;
      for (UInt32 i = 0; i < m_associativity; i++)  
      {
         if (m_opt_bits[i] > max_bits && isValidReplacement(i))
         {
            index = i;
            max_bits = m_opt_bits[i];
         }
      } //经过测试 这个找最大next的函数是没问题的
      LOG_ASSERT_ERROR(index < m_associativity, "Error Finding OPT bits");

      bool qbs_reject = false;
      if (attempt < m_num_attempts - 1)
      {
         LOG_ASSERT_ERROR(cntlr != NULL, "CacheCntlr == NULL, QBS can only be used when cntlr is passed in");
         qbs_reject = cntlr->isInLowerLevelCache(m_cache_block_info_array[index]);
      }
      //qbs_reject=false;
      if (qbs_reject)
      {
         // Block is contained in lower-level cache, and we have more tries remaining.
         // Move this block to MRU and try again
         cntlr->incrementQBSLookupCost();
         continue;
      }
      else
      {
         //printf("called getRep %lld %d %d %d %d %d %d",count_get,m_opt_bits[index],m_lru_bits[0],m_lru_bits[1],m_lru_bits[2],m_lru_bits[3],index);
         m_opt_bits[index]=FindtheNextAccess(counter_access);
         m_set_info->incrementAttempt(attempt);
         return index;
      }

   }
   LOG_PRINT_ERROR("Should not reach here");
}

void
CacheSetLRU::updateReplacementIndex(UInt32 accessed_index)
{
   //printf(" %d %d %d %d %d %d %d %d |||",m_lru_bits[0],m_lru_bits[1],m_lru_bits[2],m_lru_bits[3],m_lru_bits[4],m_lru_bits[5],m_lru_bits[6],m_lru_bits[7]);
   for(UInt32 j=0;j<512;j++){      //total 512 set
      for(UInt32 i=0;i<m_associativity;i++){
         if(all_set_opt_bits[j][i]>0)   //通过打印发现有些值没有访问过，初值是0,此时--会发生数组越界
            all_set_opt_bits[j][i]--;   //每一次access,都会让整个访问序列往前，next就会减小
      }
   }
   //如果m_lru_bits[i]=0 说明已经access到这个指令了 此时会进行next的更新
   //printf("called updateRep %lld %d %d %d %d %d %d %d %d %d %d %d ",counter_access,accessed_index,m_lru_bits[accessed_index],m_lru_bits[0],m_lru_bits[1],m_lru_bits[2],m_lru_bits[3],m_lru_bits[4],m_lru_bits[5],m_lru_bits[6],m_lru_bits[7]);
   m_opt_bits[accessed_index]=FindtheNextAccess(counter_access);
   all_set_opt_bits[set_index][accessed_index]=m_opt_bits[accessed_index];  //update the all array
   counter_access++;
}


UInt32
CacheSetLRU::FindtheNextAccess(UInt32 counter_access){  //找到当前指令的next访问

   UInt32 i;
   //printf("called FindtheNextAccess %lld",counter_access);
   for(i=counter_access+1;i<counter_access+num_lookahead+1;i++) //找未来的1000个序列中是否有能够匹配上的
   {
      if(set_index==Cache::future_list[i][0])
      {
         if(tag==Cache::future_list[i][1])
         {
            return i-counter_access;  //返回步长
         }
      }
   }
   return i-counter_access;  //没找到 返回步长1000
}

CacheSetInfoLRU::CacheSetInfoLRU(String name, String cfgname, core_id_t core_id, UInt32 associativity, UInt8 num_attempts)
   : m_associativity(associativity)
   , m_attempts(NULL)
{
   //printf("called CacheSetInfoLRU");
   m_access = new UInt64[m_associativity];
   for(UInt32 i = 0; i < m_associativity; ++i)
   {
      m_access[i] = 0;
      registerStatsMetric(name, core_id, String("access-mru-")+itostr(i), &m_access[i]);
   }

   if (num_attempts > 1)
   {
      m_attempts = new UInt64[num_attempts];
      for(UInt32 i = 0; i < num_attempts; ++i)
      {
         m_attempts[i] = 0;
         registerStatsMetric(name, core_id, String("qbs-attempt-")+itostr(i), &m_attempts[i]);
      }
   }
};

CacheSetInfoLRU::~CacheSetInfoLRU()
{
   delete [] m_access;
   if (m_attempts)
      delete [] m_attempts;
}
