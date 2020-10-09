# By default, the unit of timing parameters is 'tick', not 'cycle'.

# if the application does not finish until it executes 'max_total_instrs',
# the simulation quits.
max_total_instrs             = 2000000000000
# stack size per hardware thread
stack_sz                     = 2^23
addr_offset_lsb              = 48

# if true, none of the instructions executed on Pin is delivered to McSim.
pts.skip_all_instrs           = false
pts.simulate_only_data_caches = false
pts.show_l2_stat_per_interval = false

pts.num_hthreads             = 12
pts.num_hthreads_per_l1$     = 1
pts.num_l1$_per_l2$          = 12
pts.num_mcs                  = 1
# display simulation statistics when every pts.print_interval
# instruction is executed.
pts.print_interval           = 10^5
pts.is_race_free_application = true
pts.max_acc_queue_size       = 5
pts.use_o3core               = true
pts.use_rbol                 = false

pts.o3core.to_l1i_t_for_x87_op = 10
pts.o3core.to_l1i_t            = 2
pts.o3core.to_l1d_t            = 2
pts.o3core.branch_miss_penalty = 80             # unit: tick
pts.o3core.process_interval    = 10             # unit: tick
pts.o3core.bypass_tlb          = false
pts.o3core.consecutive_nack_threshold = 200000   # unit: instruction
# pts.o3core.num_bp_entries stands for the number of entries
# in a branch predictor.  currently it is assumed that each
# hardware thread has a branch predictor.
pts.o3core.num_bp_entries      = 256
# how many bits of global branch history information is XORed
# with branch instruction addresses.  Please check 
# 'Combining Branch Predictors' by McFarling, 1993 for 
# further information
pts.o3core.gp_size             = 60
pts.o3core.spinning_slowdown   = 10
pts.o3core.o3queue_max_size    = 128
pts.o3core.o3rob_max_size      = 64
pts.o3core.max_issue_width     = 8
pts.o3core.max_commit_width    = 8

pts.l1i$.num_sets           = 4096
pts.l1i$.num_ways           = 8
# which part of the address is mapped into a set
pts.l1i$.set_lsb            = 6
pts.l1i$.process_interval   = 10
pts.l1i$.to_lsu_t           = 2               # unit: tick
pts.l1i$.to_l2_t            = 20
# for how many ticks a cache is used per access
pts.l1i$.num_sets_per_subarray = 8
pts.l1i$.always_hit         = false

pts.l1d$.num_banks          = 4
pts.l1d$.num_sets           = 4096
pts.l1d$.num_ways           = 8
pts.l1d$.set_lsb            = 6
pts.l1d$.process_interval   = 10
pts.l1d$.to_lsu_t           = 4 
pts.l1d$.to_l2_t            = 40
pts.l1d$.num_sets_per_subarray = 8
pts.l1d$.always_hit         = false

pts.l2i$.num_sets            = 1
pts.l2i$.num_ways            = 1
pts.l2i$.set_lsb             = 6
pts.l2i$.process_interval    = 10
pts.l2i$.to_l1_t             = 40
pts.l2i$.to_dir_t            = 40
pts.l2i$.to_xbar_t           = 40
pts.l2i$.num_banks           = 4
pts.l2i$.num_flits_per_packet  = 3
pts.l2i$.num_sets_per_subarray = 16
pts.l2i$.always_hit          = false

pts.l2d$.num_sets            = 314572
pts.l2d$.num_ways            = 20
pts.l2d$.set_lsb             = 6
pts.l2d$.process_interval    = 10
pts.l2d$.to_l1_t             = 40
pts.l2d$.to_dir_t            = 40
pts.l2d$.to_xbar_t           = 40
pts.l2d$.num_banks           = 4
pts.l2d$.num_flits_per_packet  = 3
pts.l2d$.num_sets_per_subarray = 16
pts.l2d$.always_hit            = false

pts.dir.set_lsb              = 6
pts.dir.process_interval     = 10
pts.dir.to_mc_t              = 10
pts.dir.to_l2_t              = 20
pts.dir.to_xbar_t            = 20
pts.dir.cache_sz             = 8192
pts.dir.num_flits_per_packet = 3
pts.dir.num_sets             = 1024
pts.dir.num_ways             = 16
pts.dir.has_directory_cache  = false

# NoC type = ring/mesh/xbar
pts.noc_type                 = xbar
pts.xbar.to_dir_t            = 40
pts.xbar.to_l2_t             = 40
pts.xbar.process_interval    = 10

# please check 'Future Scaling of Processor-Memory
# Interfaces' by Jung Ho Ahn at el. SC09 for further
# details on the concept of VMDs.
pts.mc.process_interval      = 30
pts.mc.to_dir_t              = 430
pts.mc.interleave_base_bit     = 12
pts.mc.interleave_xor_base_bit = 18
pts.mc.num_ranks_per_mc      = 1
pts.mc.num_banks_per_rank    = 8
# parameters that start with 'pts.mc.t[capital letter]'
# have the unit of 'pts.mc.process_interval' ticks.
pts.mc.tRCD         = 14
pts.mc.tRAS         = 34
pts.mc.tRP          = 14
pts.mc.tRR          = 1
pts.mc.tCL          = 14
pts.mc.tBL          = 4
pts.mc.tWRBUB       = 0
pts.mc.tRWBUB       = 0
pts.mc.tRRBUB       = 0
pts.mc.tXP          = 0
pts.mc.tEP          = 0
pts.mc.tWTR         = 0
pts.mc.mini_rank    = false
pts.mc.req_window_sz = 32
pts.mc.rank_interleave_base_bit = 14
pts.mc.bank_interleave_base_bit = 14
pts.mc.page_sz_base_bit         = 12
pts.mc.scheduling_policy = closed
pts.mc.refresh_interval  = 800000
pts.mc.num_pages_per_bank = 8192
pts.mc.par_bs = true
pts.mc.full_duplex = false
pts.mc.is_fixed_latency = false
pts.mc.display_os_page_usage = false

pts.l1dtlb.num_entries  = 64
pts.l1dtlb.process_interval   = 10
pts.l1dtlb.to_lsu_t     = 2
pts.l1dtlb.page_sz_log2 = 22
pts.l1dtlb.miss_penalty = 80
pts.l1dtlb.speedup      = 4

pts.l1itlb.num_entries  = 64
pts.l1itlb.process_interval   = 10
pts.l1itlb.to_lsu_t     = 2
pts.l1itlb.page_sz_log2 = 12
pts.l1itlb.miss_penalty = 80
pts.l1itlb.speedup      = 4

print_md = false
