#include "EEPthreadScheduler.h"
#include <assert.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <snappy.h>
#include <map>

using namespace PinPthread;
using namespace std;


/* --------------------------------------------------------------------------- */
/* PthreadScheduler Constructor and Destructor                                 */
/* --------------------------------------------------------------------------- */

//PthreadScheduler::PthreadScheduler(uint32_t argc, char** argv) :
PthreadScheduler::PthreadScheduler(uint32_t port_num_):
  nactive(0), curr_time(0), pth_to_hth(), total_instrs(0),
  total_discarded_instrs(0), total_discarded_mem_rd(0),
  total_discarded_mem_wr(0), total_discarded_2nd_mem_rd(0),
  num_cond_broadcast(0), num_cond_signal(0), num_cond_wait(0),
  num_barrier_wait(0),
  port_num(port_num_), skip_first(0), first_instrs(0), agile_bank_th(0)
{
  pts          = new PthreadTimingSimulator(port_num);
  hth_to_pth   = vector<pthread_queue_t::iterator>(pts->num_hthreads);

  pthreads_dummy[0] = new Pthread(NULL, NULL, 0, 0, 0, pts);
  pthreads_dummy[0]->active = false;

  page_sz_log2       = pts->get_param_uint64("pts.mc.page_sz_base_bit", 12);
  ignore_skip_instrs = pts->get_param_bool("pts.ignore_skip_instrs", false);
  repeat_playing     = pts->get_param_bool("pts.repeat_playing", false);
  num_page_allocated = 0;
}


PthreadScheduler::~PthreadScheduler()
{
  KillThread(GetCurrentThread());

  cout << "  -- {" << setw(5) << port_num << "} total number of unsimulated (ins, rd, wr, rd_2nd): (" 
       << total_discarded_instrs << ", " << total_discarded_mem_rd << ", "
       << total_discarded_mem_wr << ", " << total_discarded_2nd_mem_rd << ")" << endl;
  cout << "  -- {" << setw(5) << port_num << "} (cond_broadcast, cond_signal, cond_wait, barrier) = ("
       << num_cond_broadcast << ", "
       << num_cond_signal << ", "
       << num_cond_wait << ", "
       << num_barrier_wait << ")" << endl;

  delete pts;
}



void PthreadScheduler::PlayTraces(const string& trace_list, uint64_t trace_skip_first)
{
  std::cout << "  ==============[ Playing traces ]==============" << std::endl;
  uint64_t num_sent_instrs = 0;
  do
    {
    std::vector<string> trace_names;
    std::vector<ifstream*> trace_files;


    {
      istringstream strm(trace_list);
      string temp;
      while (getline(strm, temp, ';')) {
        trace_names.push_back(temp);
      }
    }

    // ifstream trace_file(trace_name.c_str(), ios::binary);
    for (int i = 0; i < trace_names.size(); ++i) {
      trace_files.push_back(new ifstream(trace_names[i].c_str(), ios::binary));
      std::cout << "### " << trace_names[i] << "###" << std::endl;
    }
    
    // ifstream page_acc_file;
    std::vector<ifstream> page_acc_files;
    addr_perc.clear();

    for (int i = 0; i < trace_files.size(); ++i) {
      if (trace_files[i]->fail())
      {
        cout << "failed to open " << trace_names[i] << endl;
        return;
      }
      
      if (agile_bank_th > 0 && agile_bank_th < 1)
      {
        std::cerr << "agile_bank_th != 0 not supported... exit" << std::endl;
        exit(-1);
        /*
        string page_acc_file_name(trace_names[i]);
        page_acc_file_name = page_acc_file_name.substr(0, page_acc_file_name.rfind("."));
        page_acc_file_name += ".page.acc.sorted";
        page_acc_files[i].open(page_acc_file_name.c_str());
        if (page_acc_files[i].fail())
        {
          cout << "failed to open " << page_acc_file_name << endl;
          return;
        }
        string line;
        istringstream sline;
        uint32_t num_line = 0;
        uint64_t addr;
        while (getline(page_acc_files[i], line))
        {
          if (line.empty() == true || line[0] == '#') continue;
          sline.clear();
          sline.str(line);
          sline >> hex >> addr;
          addr_perc.insert(pair<uint64_t, double>(addr >> page_sz_log2, ++num_line));
        }
        for (map<uint64_t, double>::iterator iter = addr_perc.begin(); iter != addr_perc.end(); ++iter)
        {
          iter->second = iter->second / num_line;
        }
        */
      }
    }

    const size_t block_length = 500;
    while (!trace_files.empty()) {
      for(int i = 0; i < trace_files.size(); ++i) {
        for (size_t j = 0; j < block_length; ++j) {
          if (trace_files[i]->eof()) {
            delete trace_files[i];
            trace_files.erase(trace_files.begin() + i);
            --i;
            break;
          }
          else {
            PTSInstrTrace curr_instr;
            trace_files[i]->read((char*)&curr_instr, sizeof(PTSInstrTrace));
            if (agile_bank_th >= 1.0)
            {
              if (curr_instr.raddr  != 0) curr_instr.raddr  |= ((uint64_t)1 << 63);
              if (curr_instr.raddr2 != 0) curr_instr.raddr2 |= ((uint64_t)1 << 63);
              if (curr_instr.waddr  != 0) curr_instr.waddr  |= ((uint64_t)1 << 63);
            }
            else if (agile_bank_th > 0)
            {
              if (curr_instr.raddr != 0 &&
                  addr_perc.find(curr_instr.raddr >> page_sz_log2) != addr_perc.end() &&
                  addr_perc[curr_instr.raddr >> page_sz_log2] < agile_bank_th)
              {
                curr_instr.raddr |= ((uint64_t)1 << 63);
              }
              if (curr_instr.raddr2 != 0 &&
                  addr_perc.find(curr_instr.raddr2 >> page_sz_log2) != addr_perc.end() &&
                  addr_perc[curr_instr.raddr2 >> page_sz_log2] < agile_bank_th)
              {
                curr_instr.raddr2 |= ((uint64_t)1 << 63);
              }
              if (curr_instr.waddr != 0 &&
                  addr_perc.find(curr_instr.waddr >> page_sz_log2) != addr_perc.end() &&
                  addr_perc[curr_instr.waddr >> page_sz_log2] < agile_bank_th)
              {
                curr_instr.waddr |= ((uint64_t)1 << 63);
              }
              if (curr_instr.ip != 0 &&
                  addr_perc.find(curr_instr.ip >> page_sz_log2) != addr_perc.end() &&
                  addr_perc[curr_instr.ip >> page_sz_log2] < agile_bank_th)
              {
                curr_instr.ip |= ((uint64_t)1 << 63);
              }
            }

            if (num_sent_instrs++ >= trace_skip_first)
            {
              pts->internal_pid = i;
              process_ins(
                NULL,
                curr_instr.ip,
                curr_instr.raddr,
                curr_instr.raddr2,
                curr_instr.rlen,
                curr_instr.waddr,
                curr_instr.wlen,
                curr_instr.isbranch,
                curr_instr.isbranchtaken,
                curr_instr.category,
                curr_instr.rr0,
                curr_instr.rr1,
                curr_instr.rr2,
                curr_instr.rr3,
                curr_instr.rw0,
                curr_instr.rw1,
                curr_instr.rw2,
                curr_instr.rw3);
            }
          }
        }
      }
    }
  } while (repeat_playing == true);
}


uint64_t PthreadScheduler::GetPhysicalAddr(uint64_t vaddr)
{
  uint64_t paddr;

  if (v_to_p.find(vaddr >> page_sz_log2) == v_to_p.end())
  {
    v_to_p.insert(std::pair<uint64_t, uint64_t>(vaddr >> page_sz_log2, num_page_allocated++));
  }
  paddr = v_to_p[vaddr >> page_sz_log2];
  paddr <<= page_sz_log2;
  paddr += (vaddr % (1 << page_sz_log2));

  return paddr;
}



/* --------------------------------------------------------------------------- */
/* AddThread:                                                                  */
/* add an active thread to the queue                                           */
/* --------------------------------------------------------------------------- */

void PthreadScheduler::AddThread(
    pthread_t thread, pthread_attr_t* attr,
    CONTEXT* startctxt,
    ADDRINT func, ADDRINT arg)
{
  ASSERTX(pthreads.find(thread) == pthreads.end());
  pthreads[thread] = new Pthread(attr, startctxt, func, arg, curr_time, pts);
  if (pthreads.size() == 1)
  {
    current = pthreads.begin();
  }
  else if (pthreads.size() > pts->num_hthreads)
  {
    std::cout << "  -- (# of pthreads) > (# of hthreads) is not supported yet" << std::endl;
    exit(1);
  }
  nactive++;

  // create a mapping between pthread and hthread
  // currently, it is assumed that #(pthread) == #(hthread)
  hth_to_pth[pthreads.size()-1] = pthreads.find(thread);
  pth_to_hth[pthreads[thread]]  = pthreads.size()-1;
  pts->set_stack_n_size(pthreads.size()-1, 
                        (ADDRINT)pthreads.find(thread)->second->stack,
                        (ADDRINT)pthreads.find(thread)->second->stacksize);
  pts->set_active(pthreads.size()-1, pthreads.find(thread)->second->active);

  cout << "  -- [" << std::setw(12) << pts->get_curr_time() << "]: {"
       << setw(5) << port_num 
       << "} thread " << pth_to_hth[pthreads[thread]] << " is created" << std::endl;

  if (pthreads.size() > 1)
  {
    // instead of switching context, let the method resume_simulation()
    // find the newly added thread at the next time the method is called.
    pts->add_instruction(pthreads.size()-1, curr_time, 0, 0, 0, 0, 0, 0, 0, 
        false, false, false, false, false,
        0, 0, 0, 0, 0, 0, 0, 0);
  }
}



/* --------------------------------------------------------------------------- */
/* KillThread:                                                                 */
/* destroy the given thread                                                    */
/* --------------------------------------------------------------------------- */

void PthreadScheduler::KillThread(pthread_t thread) 
{
  if (thread == GetCurrentThread()) 
  {
    ASSERTX(IsActive(thread));
  }

  pts->set_stack_n_size(pth_to_hth[pthreads[thread]], 0, 0);
  pts->set_active(pth_to_hth[pthreads[thread]], false);
  cout << "  -- [" << std::setw(12) << pts->get_curr_time() << "]: {"
       << setw(5) << port_num
       << "} thread " << pth_to_hth[pthreads[thread]] << " is killed : ";
  delete pthreads[thread];
  pthreads.erase(thread);
  nactive--;
}



/* --------------------------------------------------------------------------- */
/* BlockThread:                                                                */
/* deschedule the given thread                                                 */
/* --------------------------------------------------------------------------- */

void PthreadScheduler::BlockThread(pthread_t thread, const CONTEXT * ctxt)
{
  ASSERTX(IsActive(thread));
  SetActiveState(thread, false);
  nactive--;

  ASSERT(nactive > 0, "[ERROR] Deadlocked!\n");

  /*for (int i = REG_GR_BASE; i <= REG_LAST; i++)
  {
    if (i >= REG_XMM_BASE && i <= REG_YMM_LAST) continue;
    current->second->registers[i] = PIN_GetContextReg(ctxt, (REG)i);
  }*/
  PIN_SaveContext(ctxt, GetCurrentContext());
  PIN_GetContextFPState(ctxt, current->second->fpstate);
  current->second->executed = false;

  resume_simulation(true);
}

/* --------------------------------------------------------------------------- */
/* UnblockThread:                                                              */
/* enable the given thread to be scheduled again                               */
/* --------------------------------------------------------------------------- */

void PthreadScheduler::UnblockThread(pthread_t thread, bool isbarrier) 
{
  ASSERTX(!IsActive(thread));
  SetActiveState(thread, true);
  //if (curr_time >= 8266900) cout << "AA" << endl;
  pts->add_instruction(pth_to_hth[GetThreadPtr(thread)->second], curr_time, 0, 0, 0, 0, 0, 0, 0,
      false, false, false, false, isbarrier,
      0, 0, 0, 0, 0, 0, 0, 0);
  nactive++;
}



void PthreadScheduler::add_synch_instruction(pthread_t thread, bool islock, bool isunlock, bool isbarrier)
{
  pts->add_instruction(pth_to_hth[GetThreadPtr(thread)->second], curr_time, 0, 0, 0, 0, 0, 0, 0,
      false, false, islock, isunlock, isbarrier,
      0, 0, 0, 0, 0, 0, 0, 0);
}



/* --------------------------------------------------------------------------- */
/* GetCurrentThread:                                                           */
/* return the id of the current thread running                                 */
/* --------------------------------------------------------------------------- */

pthread_t PthreadScheduler::GetCurrentThread() 
{
  return current->first;
}

/* --------------------------------------------------------------------------- */
/* IsThreadValid:                                                              */
/* determine whether the given thread is valid (active or inactive)            */
/* --------------------------------------------------------------------------- */

bool PthreadScheduler::IsThreadValid(pthread_t thread) 
{
  return (pthreads.find(thread) != pthreads.end());
}

/* --------------------------------------------------------------------------- */
/* GetAttr:                                                                    */
/* return the given thread's attribute fields relevant to the scheduler        */
/* --------------------------------------------------------------------------- */

void PthreadScheduler::GetAttr(pthread_t thread, pthread_attr_t* attr) 
{
  pthread_queue_t::iterator threadptr = pthreads.find(thread);
  ADDRINT stacksize = (threadptr->second)->stacksize;
  ADDRINT* stack = (threadptr->second)->stack;
  if (stack == NULL) 
  {
    PthreadAttr::_pthread_attr_setstack(attr, (void*)0xbfff0000, 0x10000);
  }
  else 
  {
    PthreadAttr::_pthread_attr_setstack(attr, (void*)stack, stacksize);
  }
}

/* --------------------------------------------------------------------------- */
/* GetNumActiveThreads:                                                        */
/* return the number of currently active threads                               */
/* --------------------------------------------------------------------------- */

UINT32 PthreadScheduler::GetNumActiveThreads() 
{
  return nactive;
}

/* --------------------------------------------------------------------------- */
/* Scheduling Functions:                                                       */
/* --------------------------------------------------------------------------- */

void PthreadScheduler::process_ins(
    const CONTEXT * context, ADDRINT ip,
    ADDRINT raddr, ADDRINT raddr2, UINT32 rlen,
    ADDRINT waddr, UINT32 wlen,
    bool isbranch, bool isbranchtaken, uint32_t category,
    uint32_t rr0, uint32_t rr1, uint32_t rr2, uint32_t rr3,
    uint32_t rw0, uint32_t rw1, uint32_t rw2, uint32_t rw3)
{
  Pthread * pthread = current->second;

  if (pthread->executed == false && context != NULL)
  {
    pthread->executed = true;
    return;
  }

  //first_instrs++;
  if ((ignore_skip_instrs == false && pthread->skip_instrs > 0 && pthread->spinning <= 0)/* ||
      (skip_first > first_instrs + total_discarded_instrs)*/)
  {
    return;
  }

  pthread->num_ins_mem_rd     += (raddr)  ? 1 : 0;
  pthread->num_ins_mem_wr     += (waddr)  ? 1 : 0;
  pthread->num_ins_2nd_mem_rd += (raddr2) ? 1 : 0;
  pthread->num_ins++;
  pthread->num_ins_for_spinning += (pthread->spinning > 0) ? 1 : 0;
  total_instrs++;

  if (agile_bank_th >= 1.0)
  {
    if (raddr  != 0) raddr  |= ((uint64_t)1 << 63);
    if (raddr2 != 0) raddr2 |= ((uint64_t)1 << 63);
    if (waddr  != 0) waddr  |= ((uint64_t)1 << 63);
    if (ip     != 0) ip     |= ((uint64_t)1 << 63);
  }
  else if (agile_bank_th > 0)
  {
    if (raddr != 0 &&
        addr_perc.find(raddr >> page_sz_log2) != addr_perc.end() &&
        addr_perc[raddr >> page_sz_log2] < agile_bank_th)
    {
      raddr |= ((uint64_t)1 << 63);
    }
    if (raddr2 != 0 &&
        addr_perc.find(raddr2 >> page_sz_log2) != addr_perc.end() &&
        addr_perc[raddr2 >> page_sz_log2] < agile_bank_th)
    {
      raddr2 |= ((uint64_t)1 << 63);
    }
    if (waddr != 0 &&
        addr_perc.find(waddr >> page_sz_log2) != addr_perc.end() &&
        addr_perc[waddr >> page_sz_log2] < agile_bank_th)
    {
      waddr |= ((uint64_t)1 << 63);
    }
    if (ip != 0 &&
        addr_perc.find(ip >> page_sz_log2) != addr_perc.end() &&
        addr_perc[ip >> page_sz_log2] < agile_bank_th)
    {
      ip |= ((uint64_t)1 << 63);
    }
  }
  bool must_resume = pts->add_instruction(pth_to_hth[current->second], curr_time,
      waddr, wlen, raddr, raddr2, rlen, ip, category,
      isbranch, isbranchtaken, false, false, false,
      rr0, rr1, rr2, rr3, rw0, rw1, rw2, rw3, true);
  if (must_resume)
  {
    if (nactive > 1)
    {
      /*for (int i = REG_GR_BASE; i <= REG_LAST; i++)
      {
        if (i >= REG_XMM_BASE && i <= REG_YMM_LAST) continue;
        current->second->registers[i] = PIN_GetContextReg(context, (REG)i);
      }*/
      PIN_SaveContext(context, GetCurrentContext());
      PIN_GetContextFPState(context, current->second->fpstate);
      pthread->executed = false;
    }

    resume_simulation();
  }
}



void PthreadScheduler::resume_simulation(bool must_switch)
{
  pair<uint32_t, uint64_t> ret_val;

  ret_val   = pts->resume_simulation(must_switch);
  curr_time = ret_val.second;
  /*if (curr_time >= 8266000)
  {
    cout << ret_val.first << " --- " << ret_val.second << " --- "
         << (hth_to_pth[ret_val.first] == current) << " --- "
         << HasStarted(current) << endl;
  }*/

  if (hth_to_pth[ret_val.first] == current && HasStarted(current)) 
  {
    current->second->executed = true;
    return;
  }
  current   = hth_to_pth[ret_val.first];


  if (nactive > 1 || must_switch)
  {
    if (!HasStarted(current))
    {
      StartThread(current);
    }

    PIN_SetContextFPState(GetCurrentStartCtxt(), current->second->fpstate);
    /*for (int i = REG_GR_BASE; i <= REG_LAST; i++)
    {
      if (i >= REG_XMM_BASE && i <= REG_YMM_LAST) continue;
      if (current->second->registers[i] != PIN_GetContextReg(GetCurrentStartCtxt(), (REG)i))
      {
        PIN_SetContextReg(GetCurrentStartCtxt(), (REG)i, current->second->registers[i]);
      }
    }*/
    PIN_ExecuteAt(GetCurrentStartCtxt());
  }
}



void PthreadScheduler::set_stack(CONTEXT * ctxt)
{
  if (current->second->stack == NULL)
  {
    current->second->stacksize = pts->get_param_uint64("stack_sz", def_stack_sz);
    current->second->stack     = (ADDRINT *)(PIN_GetContextReg(ctxt, REG_STACK_PTR) -
        current->second->stacksize + sizeof(ADDRINT));
    pts->set_stack_n_size(current->first, 
                          (ADDRINT) current->second->stack,
                          (ADDRINT) current->second->stacksize);
  }
}



/* --------------------------------------------------------------------------- */
/* Pthread Constructor and Destructor:                                         */
/* --------------------------------------------------------------------------- */

Pthread::Pthread(
    pthread_attr_t* attr, CONTEXT* _startctxt, ADDRINT func,
    ADDRINT arg, uint64_t curr_time_, PthreadTimingSimulator * const pts_) :
  active(true), executed(true), curr_time(curr_time_), num_ins(0), num_ins_for_spinning(0),
  num_ins_mem_rd(0), num_ins_mem_wr(0), num_ins_2nd_mem_rd(0), 
  skip_instrs(0), spinning(0), num_mutex_lock(0), num_mutex_trylock(0)
{
  if (_startctxt != NULL)   // new threads
  {
    started = false;
    stacksize = pts_->get_param_uint64("stack_sz", def_stack_sz);
    if (((stacksize / sizeof(ADDRINT)) % 2) == 0)       // align stack
    {
      stacksize += sizeof(ADDRINT);
    }
    stack = (ADDRINT*)mmap(
        0, stacksize,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANON,
        -1, 0);
    ASSERTX(stack != MAP_FAILED);
    mprotect(stack, sizeof(ADDRINT), PROT_NONE);        // delineate top of stack
    ADDRINT* sp = &(stack[stacksize/sizeof(ADDRINT) - 1]);
    ASSERTX(((ADDRINT)sp & 0x7) == 0);

#ifdef TARGET_IA32E
    ASSERTX(((ADDRINT)sp & 0x8) == 0);
    *(--sp) = (ADDRINT)StartThreadFunc;
    PIN_SaveContext(_startctxt, &startctxt);
    PIN_SetContextReg(&startctxt, REG_STACK_PTR, (ADDRINT)sp);
    PIN_SetContextReg(&startctxt, REG_GDI, (ADDRINT)arg);
    PIN_SetContextReg(&startctxt, REG_INST_PTR, (ADDRINT)func);
#else
    *(sp--) = arg;
    *(sp--) = func;
    PIN_SaveContext(_startctxt, &startctxt);
    PIN_SetContextReg(&startctxt, REG_STACK_PTR, (ADDRINT)sp);
    PIN_SetContextReg(&startctxt, REG_INST_PTR, (ADDRINT)StartThreadFunc);
#endif
    registers = new ADDRINT[REG_LAST + 1];
    fpstate   = new FPSTATE;
    PIN_GetContextFPState(_startctxt, fpstate);
  }
  else                      // initial thread
  {
    stack     = NULL;
    stacksize = 0;
    started   = true;

    registers = new ADDRINT[REG_LAST + 1];
    fpstate   = new FPSTATE;
  }
}



Pthread::~Pthread() 
{
  delete [] registers;
  //CHAR * fpstate_char = reinterpret_cast<CHAR *>(fpstate);
  munmap(stack, stacksize);

  std::cout << "  -- num_ins : (mem_rd, mem_wr, 2nd_mem_rd, spin, lock, trylock, all)=";
  std::cout << "( " << dec << setw(10) << num_ins_mem_rd << ", "
            << setw(10) << num_ins_mem_wr << ", "
            << setw(8) << num_ins_2nd_mem_rd << ", "
            << setw(8) << num_ins_for_spinning << ", "
            << setw(8) << num_mutex_lock << ", "
            << setw(8) << num_mutex_trylock << ", "
            << setw(10) << num_ins << ")" << std::endl;
}



/* --------------------------------------------------------------------------- */
/* Functions for Manipulating STL Structure(s):                                */
/* --------------------------------------------------------------------------- */

pthread_queue_t::iterator PthreadScheduler::GetThreadPtr(pthread_t thread) 
{
  pthread_queue_t::iterator threadptr = pthreads.find(thread);
  ASSERTX(threadptr != pthreads.end());
  return threadptr;
}



bool PthreadScheduler::IsActive(pthread_t thread) 
{
  return IsActive(GetThreadPtr(thread));
}



bool PthreadScheduler::IsActive(pthread_queue_t::iterator threadptr) 
{
  return ((threadptr->second)->active);
}



void PthreadScheduler::SetActiveState(pthread_t thread, bool active) 
{
  pthread_queue_t::iterator threadptr = GetThreadPtr(thread);
  (threadptr->second)->active = active;
  pts->set_active(threadptr->first, (threadptr->second)->active);
}



bool PthreadScheduler::HasStarted(pthread_queue_t::iterator threadptr) 
{
  return ((threadptr->second)->started);
}



void PthreadScheduler::StartThread(pthread_queue_t::iterator threadptr) 
{
  (threadptr->second)->started = true;
}



CONTEXT* PthreadScheduler::GetCurrentContext() 
{
  return (&((current->second)->startctxt));
}



CONTEXT* PthreadScheduler::GetCurrentStartCtxt() 
{
  return (&((current->second)->startctxt));
}

