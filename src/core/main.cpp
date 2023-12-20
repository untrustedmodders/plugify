#include <nlohmann/json.hpp>

#include "plugin_info.h"

namespace nlohmann {
    void from_json(const json& j, wizard::PluginInfo& info) {
        j.at("name").get_to(info.name);
        j.at("description").get_to(info.description);
        j.at("author").get_to(info.author);
        j.at("version").get_to(info.version);
        j.at("url").get_to(info.url);
        j.at("dependencies").get_to(info.dependencies);
    }
}

using namespace wizard;

using PluginInfoList = std::vector<std::unique_ptr<PluginInfo>>;

namespace wizard::dfs {
    /** https://iq.opengenus.org/topological-sorting-dfs/ **/

    using VisitedMap = std::unordered_map<std::string, std::pair<bool, bool>>;

    void TopSort(const std::string& pluginName, PluginInfoList& sourceList, PluginInfoList& targetList) {
        auto it = std::find_if(sourceList.begin(), sourceList.end(), [&pluginName](const std::unique_ptr<PluginInfo>& i) {
            return i->name == pluginName;
        });
        if (it != sourceList.end()) {
            auto index = static_cast<size_t>(std::distance(sourceList.begin(), it));
            auto info = std::move(sourceList[index]);
            sourceList.erase(it);
            for (const auto& name : info->dependencies) {
                TopSort(name, sourceList, targetList);
            }
            targetList.push_back(std::move(info));
        }
    }

    bool IsCyclicUtil(const std::unique_ptr<PluginInfo>& info, PluginInfoList& sourceList, VisitedMap& data) {
        auto& [visited, recursive] = data[info->name];
        if (!visited) {
            // Mark the current node as visited
            // and part of recursion stack
            visited = true;
            recursive = true;

            // Recur for all the vertices adjacent to this vertex
            for (const auto& name : info->dependencies) {
                auto it = std::find_if(sourceList.begin(), sourceList.end(), [&name](const std::unique_ptr<PluginInfo>& i) {
                    return i->name == name;
                });

                if (it != sourceList.end()) {
                    auto [vis, rec] = data[name];
                    if ((!vis && IsCyclicUtil(*it, sourceList, data)) || rec)
                        return true;
                }
            }
        }

        // Remove the vertex from recursion stack
        recursive = false;
        return false;
    }

    bool IsCyclic(PluginInfoList& sourceList) {
        // Mark all the vertices as not visited
        // and not part of recursion stack
        VisitedMap data; /* [visited, recursive] */

        // Call the recursive helper function
        // to detect cycle in different DFS trees
        for (const auto& obj : sourceList) {
            if (!data[obj->name].first && IsCyclicUtil(obj, sourceList, data))
                return true;
        }

        return false;
    }
}

int main() {
    PluginInfoList pluginInfos;

    for (auto const& entry : fs::directory_iterator("../examples/")) {
        const auto& path = entry.path();
        if (fs::is_regular_file(path) && path.extension() == ".json") {
            std::ifstream ifs{path};
            nlohmann::json jf = nlohmann::json::parse(ifs);
            pluginInfos.push_back(std::make_unique<PluginInfo>(jf.get<wizard::PluginInfo>()));
        }
    }

    if (pluginInfos.empty())
        return 0;

    // Find plugins with missing dependencies
    for (size_t j = pluginInfos.size() - 1; j != static_cast<size_t>(-1); --j) {
        const auto& info = pluginInfos[j];
        for (const auto& name : info->dependencies) {
            auto it = std::find_if(pluginInfos.begin(), pluginInfos.end(), [&name](const std::unique_ptr<PluginInfo>& i) {
                return i->name == name;
            });
            if (it == pluginInfos.end()) {
                std::cout << "Could not load '" << info->name << "' plugin. It require dependency: '" << name << "' which not exist!" << std::endl;
                pluginInfos.erase(pluginInfos.begin() + static_cast<int64_t>(j));
            }
        }
    }

    // Sort plugins by dependencies
    PluginInfoList sortedInfos;
    sortedInfos.reserve(pluginInfos.size());
    while (!pluginInfos.empty()) {
        dfs::TopSort(pluginInfos.back()->name, pluginInfos, sortedInfos);
    }

    // Function call
    if (dfs::IsCyclic(sortedInfos))
        std::cerr << "Plugins contains cycle dependencies" << std::endl;
    else
        std::cout << "Plugins doesn't contain cycle dependencies" << std::endl;

    for (auto& info : sortedInfos) {
        std::cout << info->name << std::endl;
    }

    return 0;
}