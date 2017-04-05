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

static struct plugin_info warn_unused_plugin_info = {"1.0", "This plugin warns the unused function values of a given C++ program"};

namespace{
	const pass_data warn_unused_pass_data = {
		GIMPLE_PASS,
		"warn_unused_pass", /* name */
		OPTGROUP_NONE, /* optinfo_flags */
		TV_NONE, /* tv_id */
		PROP_gimple_any, /* properties required */
		0, 	/* properties provided */
		0, 	/* properties destroyed */
		0, 	/* todo flags start */
		0, 	/* todo flags finish */
	};

	struct warn_unused_pass : gimple_opt_pass{
		/* Class constructor */
		warn_unused_pass(gcc::context *ctx):gimple_opt_pass(warn_unused_pass_data, ctx){

		}

		virtual unsigned int execute(function * fun) override{
			std::set<tree> unused_lhs = gather_unused_lhs(fun);
			warn_unused_result_lhs(unused_lhs, fun);

			return 0;
		}

		virtual warn_unused_pass *clone() override {
			return this;
		}
		
		static void insert_potentially_unused_lhs(std::set<tree>& potential_unused_lhs, tree t){
			if (t == NULL){
				return ;
			}

			if ( TREE_CODE(t) == VAR_DECL && DECL_ARTIFICIAL(t)){
				potential_unused_lhs.insert(t);
			}
		}

		static void erase_if_used_lhs(std::set<tree>& potential_unused_lhs, tree t){
			if (t == NULL)
				return;
			switch(TREE_CODE(t)){
				case VAR_DECL:
				{
					if (DECL_ARTIFICIAL(t) && potential_unused_lhs.find(t) != potential_unused_lhs.end()){
						potential_unused_lhs.erase(t);
					}
					break;
				}
				case COMPONENT_REF:
				{
					erase_if_used_lhs(potential_unused_lhs, TREE_OPERAND(t, 0));
					break;
				}
				default:
				break;
			}
		}
		std::set<tree> gather_unused_lhs(function *fun){
			std::set<tree> potential_unused_lhs;

			basic_block bb;
			FOR_ALL_BB_FN(bb, fun){
				gimple_stmt_iterator gsi;
				for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)){
					gimple stmt = gsi_stmt(gsi);

					switch(gimple_code(stmt)){
						case GIMPLE_CALL:
						{
							tree lhs = gimple_call_lhs(stmt);
							insert_potentially_unused_lhs(potential_unused_lhs, lhs);
							unsigned nargs = gimple_call_num_args(stmt);
							for (unsigned i = 0; i < nargs; i++){
								tree arg = gimple_call_arg(stmt, i);
								erase_if_used_lhs(potential_unused_lhs, arg);
							}
							break;
						}
						case GIMPLE_ASSIGN:
						{
							tree lhs = gimple_assign_lhs(stmt);
							erase_if_used_lhs(potential_unused_lhs, lhs);

							tree rhs1 = gimple_assign_rhs1(stmt);
							erase_if_used_lhs(potential_unused_lhs, rhs1);

							tree rhs2 = gimple_assign_rhs2(stmt);
							if (rhs2 != NULL)
								erase_if_used_lhs(potential_unused_lhs, rhs2);

							tree rhs3 = gimple_assign_rhs3(stmt);
							if (rhs3 != NULL)
								erase_if_used_lhs(potential_unused_lhs, rhs3);
							
							break;
						}
						break;
						case GIMPLE_RETURN:
						{
							erase_if_used_lhs(potential_unused_lhs, gimple_return_retval(as_a<greturn *>(stmt)));
							break;
						}
						default:
							break;
					}
				}
			}

			std::set<tree> unused_lhs(potential_unused_lhs);
			return unused_lhs;
		}

		void warn_unused_result_lhs(const std::set<tree> &unused_lhs, function * fun){
			basic_block bb;
			FOR_ALL_BB_FN(bb, fun){
				gimple_stmt_iterator gsi;
				for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)){
					gimple stmt = gsi_stmt(gsi);
					switch(gimple_code(stmt)){
						case GIMPLE_CALL:
						{
							tree lhs = gimple_call_lhs(stmt);
							if(unused_lhs.find(lhs) != unused_lhs.end()){
								tree fdecl = gimple_call_fndecl(stmt);
								tree ftype = gimple_call_fntype(stmt);
								if(lookup_attribute("warn_unused_result", TYPE_ATTRIBUTES(ftype))){
									location_t loc = gimple_location(stmt);

									if (fdecl){
										warning_at(loc, OPT_Wunused_result,
											"ignoring return value of %qD",
											"declared with attribute warn_unused_result",
											fdecl);
									}
									else
										warning_at(loc, OPT_Wunused_result,
											"ignoring return value of function",
											"declared with attribute warn_unused_result");
								}
							}
						break;
						}
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
			/* userdata */ &warn_unused_plugin_info);

	struct register_pass_info pass_info;

	pass_info.pass = new warn_unused_pass(g);
	pass_info.reference_pass_name = "cfg";
	pass_info.ref_pass_instance_number = 1;
	pass_info.pos_op = PASS_POS_INSERT_AFTER;

	register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);


	return 0;
}
