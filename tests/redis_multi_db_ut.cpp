#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "common/dbconnector.h"
#include "common/json.hpp"
#include <unordered_map>

using namespace std;
using namespace swss;
using json = nlohmann::json;


TEST(DBConnector, multi_db_test)
{
    string file = "./tests/redis_multi_db_ut_config/database_config.json";
    string nonexisting_file = "./tests/redis_multi_db_ut_config/database_config_nonexisting.json";

    // by default , init should be false
    cout<<"Default : isInit = "<<SonicDBConfig::isInit()<<endl;
    EXPECT_FALSE(SonicDBConfig::isInit());


    // load nonexisting file, should throw exception with NO file existing
    try
    {
        cout<<"INIT: loading nonexisting db config file"<<endl;
        SonicDBConfig::initialize(nonexisting_file);
    }
    catch (exception &e)
    {
        EXPECT_TRUE(strstr(e.what(), "Sonic database config file doesn't exist"));
    }
    EXPECT_FALSE(SonicDBConfig::isInit());

    // load local config file, init should be true
    SonicDBConfig::initialize(file);
    cout<<"INIT: load local db config file, isInit = "<<SonicDBConfig::isInit()<<endl;
    EXPECT_TRUE(SonicDBConfig::isInit());

    // load local config file again, should throw exception with init already done
    try
    {
        cout<<"INIT: loading local config file again"<<endl;
        SonicDBConfig::initialize(file);
    }
    catch (exception &e)
    {
        EXPECT_TRUE(strstr(e.what(), "SonicDBConfig already initialized"));
    }

    //Invalid DB name input
    cout<<"GET: invalid dbname input for getDbInst()"<<endl;
    EXPECT_THROW(SonicDBConfig::getDbInst("INVALID_DBNAME"), out_of_range);

    cout<<"GET: invalid dbname input for getDbId()"<<endl;
    EXPECT_THROW(SonicDBConfig::getDbId("INVALID_DBNAME"), out_of_range);

    cout<<"GET: invalid dbname input for getDbSock()"<<endl;
    EXPECT_THROW(SonicDBConfig::getDbSock("INVALID_DBNAME"), out_of_range);

    cout<<"GET: invalid dbname input for getDbHostname()"<<endl;
    EXPECT_THROW(SonicDBConfig::getDbHostname("INVALID_DBNAME"), out_of_range);

    cout<<"GET: invalid dbname input for getDbPort()"<<endl;
    EXPECT_THROW(SonicDBConfig::getDbPort("INVALID_DBNAME"), out_of_range);

    // parse config file
    ifstream i(file);
    if (i.good())
    {
        json j;
        i >> j;
        unordered_map<string, pair<string, pair<string, int>>> m_inst_info;
        for (auto it = j["INSTANCES"].begin(); it!= j["INSTANCES"].end(); it++)
        {
           string instName = it.key();
           string socket = it.value().at("unix_socket_path");
           string hostname = it.value().at("hostname");
           int port = it.value().at("port");
           m_inst_info[instName] = {socket, {hostname, port}};
        }

        for (auto it = j["DATABASES"].begin(); it!= j["DATABASES"].end(); it++)
        {
           string dbName = it.key();
           string instName = it.value().at("instance");
           int dbId = it.value().at("id");
           cout<<"testing "<<dbName<<endl;
           cout<<instName<<"#"<<dbId<<"#"<<m_inst_info[instName].first<<"#"<<m_inst_info[instName].second.first<<"#"<<m_inst_info[instName].second.second<<endl;
           // dbInst info matches between get api and context in json file
           EXPECT_EQ(instName, SonicDBConfig::getDbInst(dbName));
           // dbId info matches between get api and context in json file
           EXPECT_EQ(dbId, SonicDBConfig::getDbId(dbName));
           // socket info matches between get api and context in json file
           EXPECT_EQ(m_inst_info[instName].first, SonicDBConfig::getDbSock(dbName));
           // hostname info matches between get api and context in json file
           EXPECT_EQ(m_inst_info[instName].second.first, SonicDBConfig::getDbHostname(dbName));
           // port info matches between get api and context in json file
           EXPECT_EQ(m_inst_info[instName].second.second, SonicDBConfig::getDbPort(dbName));
        }
    }
}
