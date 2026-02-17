/* Early Loop Detector GCC Plugin
 * Detects loops right after CFG construction
 * Usage: gcc -fplugin=./early_loop_detector.so your_file.c
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "basic-block.h"
#include "cfgloop.h"
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "gimple-pretty-print.h"
#include "diagnostic.h"
#include "tree-cfg.h"

// Declare this to run with gcc as a plugin
int plugin_is_GPL_compatible;


/* Plugin information */
static struct plugin_info early_loop_detector_info = {
  .version = "1.0",
  .help = "Detects loops early in the compilation pipeline\n"
          "Options:\n"
          "  -fplugin-arg-early_loop_detector-verbose : Enable verbose output\n"
};

/* Plugin arguments */
static bool verbose_mode = false;

/* ===================================================================
 * PASS IMPLEMENTATION
 * =================================================================== */

namespace {

const pass_data pass_data_early_loop_detect = {
  GIMPLE_PASS,                    /* type */
  "early_loop_detect",            /* name */
  OPTGROUP_LOOP,                  /* optinfo_flags */
  TV_NONE,                        /* tv_id */
  PROP_cfg,                       /* properties_required - need CFG */
  0,                              /* properties_provided */
  0,                              /* properties_destroyed */
  0,                              /* todo_flags_start */
  0,                              /* todo_flags_finish */
};

class pass_early_loop_detect : public gimple_opt_pass {
public:
  pass_early_loop_detect(gcc::context *ctxt)
    : gimple_opt_pass(pass_data_early_loop_detect, ctxt)
  {}

  /* Clone method required for pass manager */
  pass_early_loop_detect *clone() override {
    return new pass_early_loop_detect(m_ctxt);
  }

  /* Gate: always run if CFG exists */
  bool gate(function *fun) override {
    return fun->cfg != NULL;
  }

  /* Main execution */
  unsigned int execute(function *fun) override;

private:
  void report_loop_info(class loop *loop, int depth);
  void analyze_loop_body(class loop *loop);
};

/* Report information about a detected loop */
void
pass_early_loop_detect::report_loop_info(class loop *loop, int depth)
{
  fprintf(stderr, "[Early Loop Detector] ");
  for (int i = 0; i < depth; i++)
    fprintf(stderr, "  ");
  
  fprintf(stderr, "Loop %d: header=bb%d", 
          loop->num, loop->header->index);
  
  if (loop->latch)
    fprintf(stderr, ", latch=bb%d", loop->latch->index);
  
  fprintf(stderr, ", depth=%d", loop_depth(loop));
  
  /* Report number of basic blocks */
  fprintf(stderr, ", num_nodes=%u", loop->num_nodes);

  auto_vec<edge> exits = get_loop_exit_edges(loop);

  /* Report loop exits */
  fprintf(stderr, ", exits=[");
  for (auto exit : exits) {
    fprintf(stderr, "bb%d->bb%d", 
            exit->src->index, exit->dest->index);
  }
  fprintf(stderr, "]");


  fprintf(stderr, "\n");
}

/* Analyze loop body (optional detailed analysis) */
void
pass_early_loop_detect::analyze_loop_body(class loop *loop)
{
  if (!verbose_mode)
    return;
    
  basic_block *bbs = get_loop_body(loop);
  unsigned n_bbs = loop->num_nodes;
  
  fprintf(stderr, "[Early Loop Detector]   Loop body: %u basic blocks\n", n_bbs);
  
  /* Count statements in loop */
  unsigned stmt_count = 0;
  for (unsigned i = 0; i < n_bbs; i++) {
    for (gimple_stmt_iterator gsi = gsi_start_bb(bbs[i]);
         !gsi_end_p(gsi); gsi_next(&gsi)) {
      if (!is_gimple_debug(gsi_stmt(gsi)))
        stmt_count++;
    }
  }
  
  fprintf(stderr, "[Early Loop Detector]   Statements: %u\n", stmt_count);
  free(bbs);
}

/* Main pass execution */
unsigned int
pass_early_loop_detect::execute(function *fun)
{
  fprintf(stderr, "\n[Early Loop Detector] ========================================\n");
  fprintf(stderr, "[Early Loop Detector] Analyzing function: %s\n", 
          function_name(fun));
  fprintf(stderr, "[Early Loop Detector] ========================================\n");

  /* Discover loops using GCC's infrastructure */
  struct loops *loops = NULL;
  bool cleanup_loops = false;
  
  /* Check if loops already exist (shouldn't at this point) */
  if (fun->x_current_loops) {
    fprintf(stderr, "[Early Loop Detector] WARNING: Loops already discovered!\n");
    loops = fun->x_current_loops;
  } else {
    /* Discover loops - this is the key function! */
    fprintf(stderr, "[Early Loop Detector] Discovering loops...\n");
    loops = flow_loops_find(NULL);
    
    /* Temporarily set current_loops so we can use loop utilities */
    set_loops_for_fn(fun, loops);
    cleanup_loops = true;
  }
  
  if (!loops || !loops->larray) {
    fprintf(stderr, "[Early Loop Detector] No loops found.\n");
  }
  else {
    /* Report discovered loops */
    unsigned n_loops = loops->larray->length();
    fprintf(stderr, "[Early Loop Detector] Found %u loops (including root)\n", n_loops);
    
    /* Traverse loop tree and report each loop */
    unsigned loop_count = 0;
    for (auto loop : loops_list (fun, 0)) {
      /* Skip the artificial root loop (loop 0) */
      if (loop->num == 0)
        continue;
        
      int depth = loop_depth(loop);
      report_loop_info(loop, depth);
      analyze_loop_body(loop);
      loop_count++;
    }
    
    fprintf(stderr, "[Early Loop Detector] Reported %u natural loops\n", loop_count);
  }
  
  fprintf(stderr, "[Early Loop Detector] ========================================\n\n");

  // TODO(avinash): 
  // 1. Check if the loops are Scops.
  // 2. Transform to a suitable polyhedral representation.
  // 3. Pass it through SOTA poly tools.
  // 4. Convert back into gimple IR.

  /* Clean up: free loops structure since we're early in pipeline */
  if (cleanup_loops && loops) {
    flow_loops_free(loops);
    ggc_free(loops);
    set_loops_for_fn(fun, NULL);
  }
  
  return 0;
}

} // anonymous namespace

/* ===================================================================
 * PASS REGISTRATION
 * =================================================================== */

int
plugin_init(struct plugin_name_args *plugin_info,
            struct plugin_gcc_version *version)
{
  /* Check GCC version compatibility */
  if (!plugin_default_version_check(version, &gcc_version)) {
    fprintf(stderr, "[Early Loop Detector Plugin] ERROR: Incompatible GCC version\n");
    return 1;
  }
  
  fprintf(stderr, "[Early Loop Detector Plugin] Initializing...\n");
  
  /* Parse plugin arguments */
  for (int i = 0; i < plugin_info->argc; i++) {
    if (strcmp(plugin_info->argv[i].key, "verbose") == 0) {
      verbose_mode = true;
      fprintf(stderr, "[Early Loop Detector Plugin] Verbose mode enabled\n");
    }
  }
  
  /* Register plugin information */
  register_callback(plugin_info->base_name, PLUGIN_INFO, NULL, 
                    &early_loop_detector_info);
  
  /* Register our pass  */
  struct register_pass_info pass_info;
  pass_info.pass = new pass_early_loop_detect(g);
  pass_info.reference_pass_name = "cfg";  /* After pass_build_cfg */
  pass_info.ref_pass_instance_number = 1;
  pass_info.pos_op = PASS_POS_INSERT_AFTER;
  
  /* For PLUGIN_PASS_MANAGER_SETUP:
   * - callback MUST be NULL
   * - user_data is pointer to register_pass_info
   */
  register_callback(plugin_info->base_name, 
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,           /* callback = NULL! */
                    &pass_info);    /* user_data = pass_info */
  
  fprintf(stderr, "[Early Loop Detector Plugin] Pass registered after 'cfg'\n");
  fprintf(stderr, "[Early Loop Detector Plugin] Initialization complete\n");
  return 0;
}

