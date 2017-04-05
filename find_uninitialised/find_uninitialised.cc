#include<iostream>
#include<cassert>
#include<set>
#include<utility>
#include<algorithm>

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"
#include "basic-block.h"
#include "gimple-pretty-print.h"
#include "cp/cp-tree.h"
#include "function.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-ssa.h"
#include "tree-pretty-print.h"
#include "tree-pass.h"
#include "tree-ssa-operands.h"
#include "tree-phinodes.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "diagnostic.h"
#include "stringpool.h"

#include "ssa-iterators.h"

int plugin_is_GPL_compatible;

static struct plugin_info find_uninitialised_plugin_info = {"1.0", "This plugin initialises uninitialised variables of a given C++ program"};

namespace{
	const pass_data find_uninitialised_pass_data = {
		GIMPLE_PASS,
		"find_uninitialised_pass", /* name */
		OPTGROUP_NONE, /* optinfo_flags */
		TV_NONE, /* tv_id */
		PROP_gimple_any, /* properties required */
		0, 	/* properties provided */
		0, 	/* properties destroyed */
		0, 	/* todo flags start */
		0, 	/* todo flags finish */
	};

	struct find_uninitialised_pass : gimple_opt_pass{
		/* Class constructor */
		find_uninitialised_pass(gcc::context *ctx):gimple_opt_pass(find_uninitialised_pass_data, ctx){

		}

		virtual unsigned int execute(function * fun) override{
			gather_uninitialised_vars(fun);
			return 0;
		}

		virtual find_uninitialised_pass *clone() override {
			return this;
		}
		static void insert_vars(std::set<tree>& lhs, tree t){
			if (t == NULL){
				return ;
			}

			if ( TREE_CODE(t) == VAR_DECL){
				lhs.insert(t);
			}
		}
		static void erase_if_initialised_lhs(std::set<tree>& lhs, tree t){
			if (t == NULL)
				return;
			switch(TREE_CODE(t)){
				case VAR_DECL:
				{
					lhs.erase(t);
					print_generic_decl(stderr, t, 0);
					break;
				}
				default:
				break;
			}
		}
		
		void gather_uninitialised_vars(function *fun){
			std::set<tree> lhs;
			std::cerr << "1";
			basic_block bb;
			FOR_ALL_BB_FN(bb, fun){
				gimple_stmt_iterator gsi;
				for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)){
					gimple stmt = gsi_stmt(gsi);
					switch(gimple_code(stmt)){
						case GIMPLE_BIND:
						{
							std::cerr << "in bind";
							gbind *gb_stmt = as_a <gbind *> (gsi_stmt(gsi));
							tree vars = gimple_bind_vars(gb_stmt);
						  	insert_vars(lhs, vars);

							gimple_seq bind_body = gimple_bind_body(gb_stmt);
							gimple_stmt_iterator gsi_bind;
							for (gsi_bind = gsi_start(bind_body); !gsi_end_p(gsi_bind); gsi_next(&gsi_bind)){
								gimple stmt_bind = gsi_stmt(gsi_bind);
								switch(gimple_code(stmt_bind)){
									case GIMPLE_ASSIGN:
									{
										tree nouse_lhs = gimple_assign_lhs(stmt_bind);
										erase_if_initialised_lhs(lhs, nouse_lhs);
										break;
									}
									default:
										break;
								}
							}
							break;
						}
						break;
						default:
							break;
					}
				}
			}

		}
	};


}

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version){
	if(!plugin_default_version_check(version, &gcc_version)){
		std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
	}

	std::cerr << "Plugin info: \n";
	std::cerr << "==================\n";
	std::cerr << "Base name: " << plugin_info->base_name << "\n";
	std::cerr << "Full name: " << plugin_info->full_name << "\n";
	std::cerr << "Number of arguments of this plugin: " << plugin_info->argc << "\n";

	for (int i = 0; i < plugin_info->argc; i++){
		std::cerr << "Argument " << i << ": Key:" << plugin_info->argv[i].key << ". Value:" << plugin_info->argv[i].value << "\n";
	}

	if (plugin_info->version != NULL){
		std::cerr << "Version string of the plugin " << plugin_info->version << "\n";
	}
	if(plugin_info->help != NULL){
		std::cerr << "Help string of the plugin " << plugin_info->help << "\n";
	}

	std::cerr << "\n";
	std::cerr << "Version info\n";
	std::cerr << "============\n\n";
	std::cerr << "Base version: " << version->basever << "\n";
	std::cerr << "Date stamp: " << version->datestamp << "\n";
	std::cerr << "Dev phase: " << version->devphase << "\n";
	std::cerr << "Revision: " << version->devphase << "\n";
	std::cerr << "Configuration arguments: " << version->configuration_arguments << "\n";
	std::cerr << "\n\n";

	std::cerr << "Plugin successfully initialised\n";

	register_callback(plugin_info->base_name,
			/* event */PLUGIN_INFO,
			/* callback */	NULL,
			/* userdata */ &find_uninitialised_plugin_info);

	struct register_pass_info pass_info;

	pass_info.pass = new find_uninitialised_pass(g);
	pass_info.reference_pass_name = "cfg";
	pass_info.ref_pass_instance_number = 1;
	pass_info.pos_op = PASS_POS_INSERT_AFTER;

	register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
	//register_callback(plugin_info->base_name, PLUGIN_FINISH, finish_gcc, NULL);
	
	return 0;
}
