#ifndef __APPS_REPORTS__
#define __APPS_REPORTS__

#include "external/inja/inja.hpp"
#include <optional>

namespace ltsy {
    
    class AppReport {
    
        private:
            inja::Environment env;

        public:

            AppReport() {
                env.set_expression("(|{","}|)");
            }

            std::string produce(const std::string& template_source,
                    const std::map<std::string, std::string>& data,
                    bool from_file=true,
                    std::optional<std::string> dest_path=std::nullopt) {
                nlohmann::json json_data;
                for (const auto& [k, v] : data)
                    json_data[k] = v;
                std::string result;
                inja::Template temp;
                if (from_file)
                    temp = env.parse_template(template_source);
                else 
                    temp = env.parse(template_source);
                if (dest_path) {
                    env.write(temp, json_data, *dest_path);
                    result = *dest_path;
                } else {
                    result = env.render(temp, json_data);
                }
                return result;
            }
    
    };

};

#endif
