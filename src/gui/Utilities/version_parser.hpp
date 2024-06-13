//
// version_parser.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <vector>

namespace utils{
    class parsed_version{
        std::vector<int> units;
    public:
        parsed_version() = delete;
        parsed_version(const wchar_t* version);
        parsed_version(const parsed_version&) = default;
        parsed_version& operator = (const parsed_version&) = default;
        bool operator == (const parsed_version& rval){
            return compare(rval) == 0;
        }
        bool operator != (const parsed_version &rval){
            return compare(rval) != 0;
        }
        bool operator < (const parsed_version &rval){
            return compare(rval) < 0;
        }
        bool operator <= (const parsed_version &rval){
            return compare(rval) <= 0;
        }
        bool operator > (const parsed_version &rval){
            return compare(rval) > 0;
        }
        bool operator >= (const parsed_version &rval){
            return compare(rval) >= 0;
        }
    protected:
        int unit_at(int pos) const{
            return pos < units.size() ? units[pos] : 0;
        }
        int compare(const parsed_version &rval) const;
    };

    extern const parsed_version this_version;
}
