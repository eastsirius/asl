/**
 * @file flags.cpp
 * @brief 程序选项解析
 * @author 程行通
 */

#include "flags.hpp"
#include "utils.hpp"
#include <cstring>

namespace ASL_NAMESPACE {
    FlagBase::FlagBase(const char* key, const char* short_key, const char* usage)
        : m_strKey(key), m_strShortKey(short_key), m_strUsage(usage) {
    }

    FlagBase::~FlagBase() {
    }

    void FlagBase::PrintUsage(int key_len) const {
        std::string strKey = _PrintKey(key_len);
        printf("    %s  %s\n", strKey.c_str(), GetUsage().c_str());
    }

    std::string FlagBase::_PrintKey(int key_len) const {
        std::string ret;
        if(GetKey().length() > 0) {
            ret += "--";
            ret += GetKey();
            if(GetShortKey().length() > 0) {
                ret += ",";
            }
        }
        if(GetShortKey().length() > 0) {
            ret += "-";
            ret += GetShortKey();
        }

        size_t cnt = key_len - GetKey().length() - GetShortKey().length();
        for(size_t i = 0; i < cnt; ++i) {
            ret += " ";
        }

        return ret;
    }


    StringFlag::StringFlag(std::string& value, const char* key, const char* short_key,
            const char* usage, std::string default_value)
            : FlagBase(key, short_key, usage), m_strValue(value) {
        m_strValue = default_value;
    }

    StringFlag::~StringFlag() {
    }

    bool StringFlag::ParseFlag(const char* value) {
        if(value) {
            m_strValue = value;
        } else {
            return false;
        }

        return true;
    }


    IntFlag::IntFlag(int& value, const char* key, const char* short_key,
            const char* usage, int default_value) : FlagBase(key, short_key, usage), m_nValue(value) {
        m_nValue = default_value;
    }

    IntFlag::~IntFlag() {
    }

    bool IntFlag::ParseFlag(const char* value) {
        if(value) {
            m_nValue = atoi(value);
        } else {
            return false;
        }

        return true;
    }


    BoolFlag::BoolFlag(bool& value, const char* key, const char* short_key,
            const char* usage, bool default_value) : FlagBase(key, short_key, usage), m_bValue(value) {
        m_bValue = default_value;
    }

    BoolFlag::~BoolFlag() {
    }

    bool BoolFlag::ParseFlag(const char* value) {
        if(value) {
            if(strcasecmp(value, "true") == 0) {
                m_bValue = true;
            } else if(strcasecmp(value, "false") == 0) {
                m_bValue = false;
            } else if(strcasecmp(value, "yes") == 0) {
                m_bValue = true;
            } else if(strcasecmp(value, "no") == 0) {
                m_bValue = false;
            } else if(strcasecmp(value, "on") == 0) {
                m_bValue = true;
            } else if(strcasecmp(value, "off") == 0) {
                m_bValue = false;
            } else {
                return false;
            }
        } else {
            m_bValue = true;
        }

        return true;
    }


    FloatFlag::FloatFlag(float& value, const char* key, const char* short_key,
            const char* usage, float default_value) : FlagBase(key, short_key, usage), m_fValue(value) {
        m_fValue = default_value;
    }

    FloatFlag::~FloatFlag() {
    }

    bool FloatFlag::ParseFlag(const char* value) {
        if(value) {
            m_fValue = (float)atof(value);
        } else {
            return false;
        }

        return true;
    }


    Flags::Flags() {
    }

    bool Flags::Parse(int argc, const char** args) {
        int i = 1;
        while(i < argc) {
            if(args[i][0] != '-') {
                return false;
            }
            if(strlen(args[i]) < 2) {
                return false;
            }

            FlagPtr_t pValue;
            if(args[i][1] == '-') {
                if(strlen(args[i]) < 3) {
                    return false;
                }

                auto iter = m_mpKeyValueMap.find(args[i] + 2);
                if(iter != m_mpKeyValueMap.end()) {
                    pValue = iter->second;
                }
            } else {
                auto iter = m_mpShortKeyValueMap.find(args[i] + 1);
                if(iter != m_mpShortKeyValueMap.end()) {
                    pValue = iter->second;
                }
            }

            if(!pValue) {
                return false;
            }
            ++i;

            const char* szValue = NULL;
            if(i < argc) {
                if(args[i][0] != '-') {
                    szValue = args[i];
                    ++i;
                }
            }

            if(!pValue->ParseFlag(szValue)) {
                return false;
            }
        }

        return true;
    }

    void Flags::Usage() {
        int nMaxSize = 0;
        for(auto iter = m_mpValues.begin(); iter != m_mpValues.end(); ++iter) {
            int nSize = (int)(iter->second->GetKey().length() + iter->second->GetShortKey().length());
            nMaxSize = asl_max(nSize, nMaxSize);
        }

        printf("Usage:\n");
        for(auto iter = m_mpValues.begin(); iter != m_mpValues.end(); ++iter) {
            iter->second->PrintUsage(nMaxSize);
        }
    }

    void Flags::StringValue(std::string& value, const char* key, const char* short_key,
            const char* usage, std::string default_value) {
        _AddValue(key, short_key, new StringFlag(value, key, short_key, usage, default_value));
    }

    void Flags::IntValue(int& value, const char* key, const char* short_key,
            const char* usage, int default_value) {
        _AddValue(key, short_key, new IntFlag(value, key, short_key, usage, default_value));
    }

    void Flags::BoolValue(bool& value, const char* key, const char* short_key,
            const char* usage, bool default_value) {
        _AddValue(key, short_key, new BoolFlag(value, key, short_key, usage, default_value));
    }

    void Flags::FloatValue(float& value, const char* key, const char* short_key,
            const char* usage, float default_value) {
        _AddValue(key, short_key, new FloatFlag(value, key, short_key, usage, default_value));
    }

    void Flags::_AddValue(const char* key, const char* short_key, FlagBase* value) {
        FlagPtr_t v(value);
        if(key) {
            m_mpKeyValueMap[key] = v;
        }
        if(short_key) {
            m_mpShortKeyValueMap[short_key] = v;
        }
        if(key) {
            m_mpValues[key] = v;
        } else if(short_key) {
            m_mpValues[short_key] = v;
        }
    }
}
