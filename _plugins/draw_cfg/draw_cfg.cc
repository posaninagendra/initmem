#include<iostream>

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"
#include "basic-block.h"
#include "gimple-pretty-print.h"

int plugin_is_GPL_compatible;

static struct plugin_info draw_cfg_plugin_info = {"1.0", "This plugin will draw the control flow graph of the c/cpp program."};

namespace{
	const pass_data draw_cfg_pass_data = {
		GIMPLE_PASS,
		"draw_cfg_pass", /* name */
		OPTGROUP_NONE, /* optinfo_flags */
		TV_NONE, /* tv_id */
		PROP_gimple_any, /* properties required */
		0, 	/* properties provided */
		0, 	/* properties destroyed */
		0, 	/* todo flags start */
		0, 	/* todo flags finish */
	};

struct draw_cfg_pass : gimple_opt_pass{
	draw_cfg_pass(gcc::context *ctx):gimple_opt_pass(draw_cfg_pass_data, ctx){
	
	}

	virtual unsigned int execute(function * fun) override{
		basic_block bb;
		std::cerr << "subgraph fun_" << fun << " {\n";
		FOR_ALL_BB_FN(bb, fun){
			gimple_bb_info *bb_info = &bb->il.gimple;

			std::cerr << "bb_" << fun << "_" << bb->index << "[label=\"";
			if(bb->index == 0){
				std::cerr << "ENTRY: "
					  << function_name(fun) << "\n"
					  << (LOCATION_FILE(fun->function_start_locus) ? : "<unknown>")
					  << ":" << LOCATION_LINE(fun->function_start_locus);
			} else if (bb->index == 1){
				std::cerr << " EXIT: "
					  << function_name(fun) << "\n"
					  << (LOCATION_FILE(fun->function_end_locus) ? : "<unknown>")
					  << ":" << LOCATION_LINE(fun->function_end_locus);
			}else{
				print_gimple_seq(stderr, bb_info->seq, 0, 0);
			}
			std::cerr << "\"];\n";

			edge e;
			edge_iterator ei;

			FOR_EACH_EDGE(e, ei, bb->succs){
				basic_block dest = e->dest;
				std::cerr << "bb_" << fun << "_" << bb->index << " -> bb_" << fun << "_" << dest->index << ";\n";
			}
		}

		std::cerr << "}\n";

		return 0;
	}

	virtual draw_cfg_pass *clone() override {
		return this;
	}
	};
}
namespace{
	void start_graph(){
		std::cerr << "digraph cfg {\n";
	}

	void end_graph(){
		std::cerr << "}\n";	
	}
	
	void finish_gcc(void *gcc_data, void *user_data){
		end_graph();
	}
}

int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version){
	if(!plugin_default_version_check(version, &gcc_version)){
		std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
	}
	/* Printing the Plugin information

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
	*/
	register_callback(plugin_info->base_name,
			/* event */PLUGIN_INFO,
			/* callback */	NULL,
			/* userdata */ &draw_cfg_plugin_info);

	struct register_pass_info pass_info;

	pass_info.pass = new draw_cfg_pass(g);
	pass_info.reference_pass_name = "cfg";
	pass_info.ref_pass_instance_number = 1;
	pass_info.pos_op = PASS_POS_INSERT_AFTER;

	register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
	register_callback(plugin_info->base_name, PLUGIN_FINISH, finish_gcc, NULL);

	start_graph();

	return 0;
}
