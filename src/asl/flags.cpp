/**
 * @file flags.cpp
 * @brief 程序选项解析
 * @author 程行通
 */

#include "flags.hpp"
#include "utils.hpp"
#include <cstring>

namespace ASL_NAMESPACE {
    FlagBase::FlagBase(const char* szKey, const char* szShortKey, const char* szUsage)
        : m_strKey(szKey), m_strShortKey(szShortKey), m_strUsage(szUsage) {
    }

    FlagBase::~FlagBase() {
    }

    void FlagBase::PrintUsage(int nKeyLen) const {
        std::string strKey = _PrintKey(nKeyLen);
        printf("    %s  %s\n", strKey.c_str(), GetUsage().c_str());
    }

    std::string FlagBase::_PrintKey(int nKeyLen) const {
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

        size_t cnt = nKeyLen - GetKey().length() - GetShortKey().length();
        for(size_t i = 0; i < cnt; ++i) {
            ret += " ";
        }

        return ret;
    }


    StringFlag::StringFlag(std::string& strValue, const char* szKey, const char* szShortKey,
            const char* szUsage, std::string strDefaultValue)
            : FlagBase(szKey, szShortKey, szUsage), m_strValue(strValue) {
        m_strValue = strDefaultValue;
    }

    StringFlag::~StringFlag() {
    }

    bool StringFlag::ParseFlag(const char* szValue) {
        if(szValue) {
            m_strValue = szValue;
        } else {
            return false;
        }

        return true;
    }


    IntFlag::IntFlag(int& nValue, const char* szKey, const char* szShortKey,
            const char* szUsage, int nDefaultValue) : FlagBase(szKey, szShortKey, szUsage), m_nValue(nValue) {
        m_nValue = nDefaultValue;
    }

    IntFlag::~IntFlag() {
    }

    bool IntFlag::ParseFlag(const char* szValue) {
        if(szValue) {
            m_nValue = atoi(szValue);
        } else {
            return false;
        }

        return true;
    }


    BoolFlag::BoolFlag(bool& bValue, const char* szKey, const char* szShortKey,
            const char* szUsage, bool bDefaultValue) : FlagBase(szKey, szShortKey, szUsage), m_bValue(bValue) {
        m_bValue = bDefaultValue;
    }

    BoolFlag::~BoolFlag() {
    }

    bool BoolFlag::ParseFlag(const char* szValue) {
        if(szValue) {
            if(strcasecmp(szValue, "true") == 0) {
                m_bValue = true;
            } else if(strcasecmp(szValue, "false") == 0) {
                m_bValue = false;
            } else if(strcasecmp(szValue, "yes") == 0) {
                m_bValue = true;
            } else if(strcasecmp(szValue, "no") == 0) {
                m_bValue = false;
            } else if(strcasecmp(szValue, "on") == 0) {
                m_bValue = true;
            } else if(strcasecmp(szValue, "off") == 0) {
                m_bValue = false;
            } else {
                return false;
            }
        } else {
            m_bValue = true;
        }

        return true;
    }


    FloatFlag::FloatFlag(float& fValue, const char* szKey, const char* szShortKey,
            const char* szUsage, float fDefaultValue) : FlagBase(szKey, szShortKey, szUsage), m_fValue(fValue) {
        m_fValue = fDefaultValue;
    }

    FloatFlag::~FloatFlag() {
    }

    bool FloatFlag::ParseFlag(const char* szValue) {
        if(szValue) {
            m_fValue = (float)atof(szValue);
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
            int nSize = iter->second->GetKey().length() + iter->second->GetShortKey().length();
            nMaxSize = asl_max(nSize, nMaxSize);
        }

        printf("Usage:\n");
        for(auto iter = m_mpValues.begin(); iter != m_mpValues.end(); ++iter) {
            iter->second->PrintUsage(nMaxSize);
        }
    }

    void Flags::StringValue(std::string& strValue, const char* szKey, const char* szShortKey,
            const char* szUsage, std::string strDefaultValue) {
        _AddValue(szKey, szShortKey, new StringFlag(strValue, szKey, szShortKey, szUsage, strDefaultValue));
    }

    void Flags::IntValue(int& nValue, const char* szKey, const char* szShortKey,
            const char* szUsage, int nDefaultValue) {
        _AddValue(szKey, szShortKey, new IntFlag(nValue, szKey, szShortKey, szUsage, nDefaultValue));
    }

    void Flags::BoolValue(bool& bValue, const char* szKey, const char* szShortKey,
            const char* szUsage, bool bDefaultValue) {
        _AddValue(szKey, szShortKey, new BoolFlag(bValue, szKey, szShortKey, szUsage, bDefaultValue));
    }

    void Flags::FloatValue(float& fValue, const char* szKey, const char* szShortKey,
            const char* szUsage, float fDefaultValue) {
        _AddValue(szKey, szShortKey, new FloatFlag(fValue, szKey, szShortKey, szUsage, fDefaultValue));
    }

    void Flags::_AddValue(const char* szKey, const char* szShortKey, FlagBase* pValue) {
        FlagPtr_t value(pValue);
        if(szKey) {
            m_mpKeyValueMap[szKey] = value;
        }
        if(szShortKey) {
            m_mpShortKeyValueMap[szShortKey] = value;
        }
        if(szKey) {
            m_mpValues[szKey] = value;
        } else if(szShortKey) {
            m_mpValues[szShortKey] = value;
        }
    }
}
