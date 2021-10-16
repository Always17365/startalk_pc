//
// Created by cc on 18-12-3.
//

#ifndef STALK_V2_IM_CONFIG_H
#define STALK_V2_IM_CONFIG_H

#include <string>

namespace st {
    namespace entity {
        struct ImConfig
        {
            ImConfig()
            {
                ConfigKey = "";
                ConfigSubKey = "";
                ConfigValue = "";
                Version = 0;
                ExtendedFlag = "";
            }

            std::string ConfigKey;
            std::string ConfigSubKey;
            std::string ConfigValue;
            int Version;
            std::string ExtendedFlag;
        };
    }
}

#endif //STALK_V2_IM_CONFIG_H
