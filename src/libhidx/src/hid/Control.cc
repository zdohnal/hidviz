#include "libhidx/hid/Control.hh"

#include <algorithm>

namespace libhidx {
namespace hid {
    Item* Control::clone(Item* parent, Item* dst) const {
        if(!dst){
            dst = new Control{};
        }
        auto it = static_cast<Control*>(Item::clone(parent, dst));
        it->m_offset = m_offset;
        it->m_usages = m_usages;
        it->m_flags = m_flags;
        it->m_reportType = m_reportType;
        it->m_reportSize = m_reportSize;
        it->m_reportCount = m_reportCount;
        it->m_logicalMinimum = m_logicalMinimum;
        it->m_logicalMaximum = m_logicalMaximum;
        it->m_physicalMinimum = m_physicalMinimum;
        it->m_physicalMaximum = m_physicalMaximum;
        it->m_unitExponent = m_unitExponent;
        it->m_unit = m_unit;

        return it;
    }

    void Control::update(const std::vector<unsigned char>& rawData) {
        if(!m_usages.size()){
            return;
        }
        auto data = extractData(rawData);
        //TODO: magic constant!
        if(m_flags & 2){
            // variable
            for(unsigned i = 0; i < m_reportCount; ++i){
                uint32_t usageData = extractVariableUsageData(data, i);
                m_usages[i].setData(usageData);
            }
        } else {
            // array
            for(auto& usage: m_usages){
                usage.setData(0);
            }

            for(unsigned i = 0; i < m_reportCount; ++i){
                uint32_t usageIndex = extractVariableUsageData(data, i);

                auto usage = findUsageByValue(usageIndex);
                if(!usage){
                    continue;
                }

                usage->setData(1);
            }
        }
    }

    uint32_t Control::extractData(const std::vector<unsigned char> &vector) {
        auto length = m_reportCount * m_reportSize;
        uint32_t data = 0;

        for(unsigned i = 0; i < length; ++i){
            auto originPos = m_offset + i;
            auto bytePos = originPos / 8;
            auto bitPos = originPos % 8;

            unsigned char byte = vector[bytePos];
            byte >>= bitPos;
            byte &= 1;
            data |= byte << i;
        }

        return data;
    }

    uint32_t Control::extractVariableUsageData(uint32_t data, unsigned index) {
        auto i = index * m_reportSize;
        auto mask = (1U << m_reportSize) - 1U;

        return (data >> i) & mask;
    }

    Usage * Control::findUsageByValue(uint32_t value) {
        auto it = std::find_if(begin(m_usages), end(m_usages), [&value](const auto &usage){
            return (usage.getValue() & 0xff) == value;
        });

        if(it == end(m_usages)){
            return nullptr;
        }

        return &*it;
    }
}
}
