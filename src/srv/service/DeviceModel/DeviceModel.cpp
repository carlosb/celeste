/**
 * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 7/Nov/17 Carlos Brito *
 * * * * * * * * * * * * * * * * * * * *
 * 
 * @file
 */
#include <soci/mysql/soci-mysql.h>

#include "DeviceModel.hpp"
#include "srv/service/common.hpp"
#include "srv/service/device/device.hpp"
#include "srv/service/model/model.hpp"

using namespace std;
using namespace soci;

namespace celeste
{
namespace resource
{   

    // --- CLASS DEFINITIONS ---------
    DeviceModelAssocs<nlohmann::json>::DeviceModelAssocs(const std::string& dbSettings)
    {
        set_path("/device_model");
        set_method_handler("GET", [this] (const std::shared_ptr<restbed::Session> session) {GET(session);});
        set_method_handler("POST",   [this] (const std::shared_ptr<restbed::Session> session) {POST(session);});
        set_method_handler("DELETE", [this] (const std::shared_ptr<restbed::Session> session) {DELETE(session);});
        
        for (int i = 0; i < 10; ++i)
            sqlPool.emplace(mysql, dbSettings);
    }

    std::vector<DeviceModelAssoc> DeviceModelAssocs<nlohmann::json>::get(const std::string& deviceId)
    {
        auto sql = sqlPool.acquire_wait();

        rowset<DeviceModelAssoc> res = (sql->prepare << "select * from Device_Model where Device_id = :DeviceId", use(deviceId));
        
        std::vector<DeviceModelAssoc> assocs;
        assocs.reserve(50);
        for (auto it = res.begin(); it != res.end(); ++it)
            assocs.push_back(*it);
        return assocs;
    }

    std::vector<DeviceModelAssoc> DeviceModelAssocs<nlohmann::json>::get(const std::string& deviceId, const std::string& modelId)
    {
        auto sql = sqlPool.acquire_wait();

        rowset<DeviceModelAssoc> res = (sql->prepare 
                                        << "select * from Device_Model where Device_id = :DeviceId and Model_id = :ModelId",
                                        use(deviceId), use(modelId));
        
        std::vector<DeviceModelAssoc> assocs;
        assocs.reserve(50);
        for (auto it = res.begin(); it != res.end(); ++it)
            assocs.push_back(*it);
        return assocs;
    }


    DeviceModelAssoc DeviceModelAssocs<nlohmann::json>::get(const std::string& deviceId, const std::string& modelId, int idx)
    {   
        auto sql = sqlPool.acquire_wait();
        
        DeviceModelAssoc assoc;
        *sql    << "select * from Device_Model"
                << "where Device_id = :DeviceId and Model_id = :ModelId and idx = :idx",
                use(deviceId), use(modelId), use(idx), into(assoc);

        if (sql->got_data())
            return assoc;
        else
            throw status::DEVICE_MODEL_NOT_FOUND;
    }

    int DeviceModelAssocs<nlohmann::json>::associate(const std::string& deviceId, const std::string& modelId, const boost::optional<std::string>& note)
    {
        auto sql = sqlPool.acquire_wait();
        *sql    << "insert into Device_Model(Device_id, Model_id, note) values(:DeviceId, :ModelId, :note)",
                use(deviceId), use(modelId), use(note);

        long idx;
        sql->get_last_insert_id("PointRecord", idx);
        return idx;
    }

    void DeviceModelAssocs<nlohmann::json>::dissasociate(const std::string& deviceId, const std::string& modelId, int idx)
    {
        auto sql = sqlPool.acquire_wait();
        
        *sql    << "delete from Device_Model where"
                << "Device_id = :DeviceId and Model_id = :ModelId and idx = :idx",
                use(deviceId), use(modelId), use(idx);
    }

    void DeviceModelAssocs<nlohmann::json>::GET(const std::shared_ptr<restbed::Session> session)
    {   
        // get request
        const auto request = session->get_request();

        // get headers
        size_t content_length = (size_t) request->get_header("Content-Length", 0);

        // fetch data to access later
        session->fetch(content_length, [] (const shared_ptr<restbed::Session> session, const restbed::Bytes &bytes) {});

        // get json from request
        json_type data = get_json<json_type>(*request);

        // validate data
        if (data["DeviceId"].is_null())
            throw status::MISSING_FIELD_DEVICEID; // DeviceId is required for any request

        // get device from db
        json_type response;

        if (data["idx"].is_null() && data["ModelId"].is_null())
        {
            response = this->get(data["DeviceId"]);
        }
        else if (data["ModelId"].is_null())
        {
            response = this->get(data["DeviceId"], data["ModelId"]);
        }
        else
        {
            response = this->get(data["DeviceId"], data["ModelId"], data["idx"]);
        }

        // close
        session->close(restbed::OK,
                       response.dump(),
                       {
                            { "Content-Length", to_string(response.dump().size()) },
                            { "Connection",     "close" }
                       });
    }

    void DeviceModelAssocs<nlohmann::json>::POST(const std::shared_ptr<restbed::Session> session)
    {
        // get request
        const auto request = session->get_request();

        // get headers
        size_t content_length = (size_t) request->get_header("Content-Length", 0);

        // fetch data to access later
        session->fetch(content_length, [] (const shared_ptr<restbed::Session> session, const restbed::Bytes &bytes) {});

        // get json from request
        json_type data = get_json<json_type>(*request);

        // validate data
        if (data["DeviceId"].is_null())
            throw status::MISSING_FIELD_DEVICEID;

        if (data["ModelId"].is_null())
            throw status::MISSING_FIELD_MODELID;

        if (data["note"].is_null())
            data["note"] = nullptr;

        DeviceModelAssoc dm = data.get<DeviceModelAssoc>();
        auto idx = this->associate(dm.DeviceId, dm.ModelId, dm.note);
        json_type response{{"idx", idx}};

        // close
        session->close(restbed::OK,
                       response.dump(),
                       {
                            { "Content-Length", to_string(response.dump().size()) },
                            { "Connection",     "close" }
                       });
    }

    void DeviceModelAssocs<nlohmann::json>::DELETE(const std::shared_ptr<restbed::Session> session)
    {
        // get request
        const auto request = session->get_request();

        // get headers
        size_t content_length = (size_t) request->get_header("Content-Length", 0);

        // fetch data to access later
        session->fetch(content_length, [] (const shared_ptr<restbed::Session> session, const restbed::Bytes &bytes) {});

        // get json from request
        json_type data = get_json<json_type>(*request);

        // validate data
        if (data["DeviceId"].is_null())
            throw status::MISSING_FIELD_DEVICEID;

        if (data["ModelId"].is_null())
            throw status::MISSING_FIELD_MODELID;

        if (data["idx"].is_null())
            throw status::MISSING_FIELD_IDX;

        // dissasociate the device and the model
        this->dissasociate(data["DeviceId"], data["ModelId"], data["idx"]);

        // close
        session->close(restbed::OK);
    }
}
}

// --- SERIALIZERS -------------------
namespace soci
{
    using namespace celeste::resource;

    void type_conversion<DeviceModelAssoc>::from_base(values const& v, indicator , DeviceModelAssoc& p)
    {
        p = DeviceModelAssoc {
            .idx        = v.get<int>("idx"),
            .DeviceId   = v.get<string>("Device_id"),
            .ModelId    = v.get<string>("Model_id"),
            .note       = v.get<boost::optional<string>>("note", boost::none)
        };
    }

    void type_conversion<DeviceModelAssoc>::to_base(const DeviceModelAssoc& p, values& v, indicator& ind)
    {
        v.set("idx",        p.idx);
        v.set("Device_id",  p.DeviceId);
        v.set("Model_id",   p.ModelId);
        v.set("note",       p.note);
        ind = i_ok;
    }
}

namespace nlohmann
{
    using namespace celeste::resource;

    void adl_serializer<DeviceModelAssoc>::to_json(json& j, const DeviceModelAssoc& dm)
    {
        j = json {
            {"DeviceId",    dm.DeviceId},
            {"ModelId",     dm.ModelId},
            {"idx",         dm.idx},
            {"note",        dm.note}
        };
    }

    void adl_serializer<DeviceModelAssoc>::from_json(const json& j, DeviceModelAssoc& dm)
    {
        dm = DeviceModelAssoc {
            .DeviceId   = j.at("DeviceId"),
            .ModelId    = j.at("ModelId"),
            // .idx        = j.at("idx"),
            .note       = j.at("note")
        };
    }
}