#ifndef __YAML_CPP_PARSER__
#define __YAML_CPP_PARSER__

#include "yaml-cpp/yaml.h"
#include "core/exception/exceptions.h"

namespace ltsy {

    class YAMLCppParser {
        
        public:

        YAML::Node load_from_file(const std::string& path) const {
            return YAML::LoadFile(path);
        }

        /**
         * Check if a node exists in the current one. If not, throws
         * an error.
         *
         * @param curr_node the current node
         * @param node_name the name of the node
         * @return if the node exists in the current one
         * */
        YAML::Node hard_require(const YAML::Node & curr_node, const std::string& node_name) const {
            if (!curr_node[node_name]) {
                throw ParseException("missing node " + node_name, curr_node.Mark().line, curr_node.Mark().column);
            }
            return curr_node[node_name]; 
        }

        /**
         * Check if a node exists in the current one. If not, print a warning.
         *
         * @param curr_node the current node
         * @param node_name the name of the node
         * @return if the node exists in the current one
         * */
        bool soft_require(const YAML::Node & curr_node, const std::string& node_name) const {
            return curr_node[node_name];
        }

    };

};

#endif
