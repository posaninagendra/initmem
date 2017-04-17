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
		private: 
			std::set<tree> uninitialised_vars;
		
		public:
		/* Class constructor */
		find_uninitialised_pass(gcc::context *ctx) 
			: gimple_opt_pass(find_uninitialised_pass_data, ctx){}

		virtual unsigned int execute(function * fun) override{
			gather_uninitialised_vars(fun);
			return 0;
		}

		virtual find_uninitialised_pass *clone() override { return this; }

		/* Insert tree nodes of type VAR_DECL in the set */
		static void insert_vars(std::set<tree>& var_set, tree t){
			if (t == NULL || TREE_CODE(t) != VAR_DECL || DECL_ARTIFICIAL(t))
				return ;

			var_set.insert(t);
		}
		/* Erase a tree-node form the set, if it is initialized */
		static void erase_if_initialised_lhs(std::set<tree>& var_set, tree t){
			if (t == NULL)
				return;

			switch(TREE_CODE(t)){
				case VAR_DECL:
					{
						if (DECL_ARTIFICIAL(t))
							break;
						var_set.erase(t);
						break;
					}
				default:
					break;
			}
		}

		static tree vars_callback_stmt(gimple_stmt_iterator *gsi, 
						bool *handled_all_ops, 
						struct walk_stmt_info *wi){
			*handled_all_ops = true;
			
			find_uninitialised_pass *this_ = reinterpret_cast<find_uninitialised_pass *>(wi->info);

			this_->do_traverse_stmt(gsi);

			return nullptr;
		}

		void traverse_stmt(gimple_seq gseq){
			struct walk_stmt_info wi;
			memset(&wi, 0, sizeof(wi));

			wi.info = this;

			walk_gimple_seq(gseq, vars_callback_stmt, nullptr, &wi);
		}

		tree do_traverse_stmt(gimple_stmt_iterator *gsi){
			
			gimple stmt = gsi_stmt(*gsi);

			switch(gimple_code(stmt)){
				/* For each GIMPLE_BIND, find vars and find initialization */
				case GIMPLE_BIND:
					{
						gbind *gb_stmt = as_a <gbind *> (stmt);
						for (tree var = gimple_bind_vars(gb_stmt); var != NULL;
							var = DECL_CHAIN(var)){
							insert_vars(uninitialised_vars, var);	
						}
						/* Get the GIMPLE_BIND body and traverse the gimple sequence to find initialization statements */
						gimple_seq bind_body = gimple_bind_body(gb_stmt);
						traverse_stmt(bind_body);
						break;
					}
				case GIMPLE_ASSIGN:
					{
						tree lhs = gimple_assign_lhs(stmt);
						erase_if_initialised_lhs(uninitialised_vars, lhs);
						break;
					}
				default:
					break;
			}
			return nullptr;
		}

		void gather_uninitialised_vars(function *fun){
			uninitialised_vars.clear();

			std::cerr << "Function --> " << IDENTIFIER_POINTER(DECL_NAME(fun->decl)) << "\n";

			gimple_seq gseq = fun->gimple_body;

			traverse_stmt(gseq);
			for (tree t: uninitialised_vars){
				std::cerr << "Variables '" << IDENTIFIER_POINTER(DECL_NAME(t)) << "' looks uninitialised \n";
			}
			std::cerr << "\n";
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
	std::cerr << "===============================\n\n";
	register_callback(plugin_info->base_name,
			/* event */PLUGIN_INFO,
			/* callback */	NULL,
			/* userdata */ &find_uninitialised_plugin_info);

	struct register_pass_info pass_info;

	pass_info.pass = new find_uninitialised_pass(g);
	pass_info.reference_pass_name = "lower";
	pass_info.ref_pass_instance_number = 1;
	pass_info.pos_op = PASS_POS_INSERT_BEFORE;

	register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

	return 0;
}
