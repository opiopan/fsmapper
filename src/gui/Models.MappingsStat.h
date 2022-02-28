//
// Models.MappingsStat.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "Models.MappingsStat.g.h"

namespace winrt::gui::Models::implementation
{
    struct MappingsStat : MappingsStatT<MappingsStat>
    {
        MappingsStat() = default;

        int32_t Primery(){return primery;}
        void Primery(int32_t value){primery = value;}
        int32_t Secondary(){return secondary;}
        void Secondary(int32_t value){secondary = value;}
        int32_t Viewports(){return viewports;}
        void Viewports(int32_t value){viewports = value;}
        int32_t Views(){return views;}
        void Views(int32_t value){views = value;}
        
    protected:
        int32_t primery{0};
        int32_t secondary{0};
        int32_t viewports{0};
        int32_t views{0};
    };
}
namespace winrt::gui::Models::factory_implementation
{
    struct MappingsStat : MappingsStatT<MappingsStat, implementation::MappingsStat>
    {
    };
}
