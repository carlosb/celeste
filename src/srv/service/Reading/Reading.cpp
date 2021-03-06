#include <string>

#include "srv/service/common.hpp"

#include "Reading.hpp"


using namespace celeste::resource;
using namespace std;


namespace nlohmann
{
    void adl_serializer<Reading>::to_json(json& j, const Reading& obj)
    {
        j = json {
            {"t",       obj.t},
            {"sf",      obj.sf},
            {"value",   obj.value}
        };
    }

    void adl_serializer<Reading>::from_json(const json& j, Reading& obj)
    {
        obj.t      = j.at("t").get<boost::posix_time::ptime>();
        obj.sf      = j.at("sf").get<double>();
        obj.value  = j.at("value").get<Reading::point_type>();
    }

    void adl_serializer<Reading::point_type>::to_json(json& j, const Reading::point_type& obj)
    {
        switch(obj.which())
        {
            case INTEGER:
            {
                j = boost::get<int>(obj);
                break;
            }
            case FLOAT:
            {
                j = boost::get<float>(obj);
                break;
            }
            case STRING:
            {
                j = boost::get<string>(obj);
                break;
            }
            default:
            {
                throw runtime_error("Type mismatch while converting point_type to json: "
                                    "No such conversion.");
            }
        }
    }

    void adl_serializer<Reading::point_type>::from_json(const json& j, Reading::point_type& obj)
    {
        switch (j.type())
        {
            case json::value_t::number_integer:
            {
                obj = j.get<int>();
            }
            case json::value_t::number_float:
            {
                obj = j.get<float>();
            }
            case json::value_t::string:
            {
                obj = j.get<string>();
            }
            default:
            {
                throw runtime_error("Type mismatch while converting json to point_type: "
                                    "No such conversion.");
            }
        }
    }
}

namespace soci
{
    void type_conversion<Reading>::from_base(values const& v,
                                             indicator,
                                             Reading& p)
    {
        p.sf = v.get<double>("sf");
        p.t  = v.get<boost::posix_time::ptime>("t");
        p.value  = v.get<string>("v");
    }

    void type_conversion<Reading>::to_base(const Reading& p,
                                           values& v,
                                           indicator& ind)
    {
        v.set("sf", p.sf);
        v.set("t", p.t);
        v.set("v", p.value);
    }

    void type_conversion<Reading::point_type>::from_base(string const& v,
                                                         indicator,
                                                         Reading::point_type& p)
    {
        p = v;
    }

    void type_conversion<Reading::point_type>::to_base(const Reading::point_type& p,
                                                       string& v,
                                                       indicator& ind)
    {
        v = boost::get<string>(p);
    }
}