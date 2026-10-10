
#ifndef SQLITE_DATA_OUTPUT_H
#define SQLITE_DATA_OUTPUT_H

#include "data-output-interface.h"

#include "ns3/nstime.h"

struct sqlite3_stmt;

namespace ns3 {

class SQLiteOutput;

class SqliteDataOutput : public DataOutputInterface {
public:
  SqliteDataOutput();
  ~SqliteDataOutput() override;

  static TypeId GetTypeId();

  void Output(DataCollector &dc) override;

private:
  class SqliteOutputCallback : public DataOutputCallback {
  public:
    SqliteOutputCallback(const Ptr<SQLiteOutput> &db, std::string run);

    ~SqliteOutputCallback() override;

    void OutputStatistic(std::string key, std::string variable,
                         const StatisticalSummary *statSum) override;

    void OutputSingleton(std::string key, std::string variable,
                         int val) override;

    void OutputSingleton(std::string key, std::string variable,
                         uint32_t val) override;

    void OutputSingleton(std::string key, std::string variable,
                         double val) override;

    void OutputSingleton(std::string key, std::string variable,
                         std::string val) override;

    void OutputSingleton(std::string key, std::string variable,
                         Time val) override;

  private:
    Ptr<SQLiteOutput> m_db;
    std::string m_runLabel;

    sqlite3_stmt *m_insertSingletonStatement;
  };

  Ptr<SQLiteOutput> m_sqliteOut;
};

} // namespace ns3

#endif
