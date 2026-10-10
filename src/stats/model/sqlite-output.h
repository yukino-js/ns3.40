#ifndef SQLITE_OUTPUT_H
#define SQLITE_OUTPUT_H

#include "ns3/simple-ref-count.h"

#include <mutex>
#include <sqlite3.h>
#include <string>

namespace ns3 {

class SQLiteOutput : public SimpleRefCount<SQLiteOutput> {
public:
  SQLiteOutput(const std::string &name);

  ~SQLiteOutput();

  void SetJournalInMemory();

  bool SpinExec(const std::string &cmd) const;

  bool SpinExec(sqlite3_stmt *stmt) const;

  bool WaitExec(const std::string &cmd) const;

  bool WaitExec(sqlite3_stmt *stmt) const;

  bool WaitPrepare(sqlite3_stmt **stmt, const std::string &cmd) const;

  bool SpinPrepare(sqlite3_stmt **stmt, const std::string &cmd) const;

  template <typename T>
  bool Bind(sqlite3_stmt *stmt, int pos, const T &value) const;

  template <typename T> T RetrieveColumn(sqlite3_stmt *stmt, int pos) const;

  static int SpinStep(sqlite3_stmt *stmt);
  static int SpinFinalize(sqlite3_stmt *stmt);

  static int SpinReset(sqlite3_stmt *stmt);

protected:
  int WaitExec(sqlite3 *db, const std::string &cmd) const;

  int WaitExec(sqlite3 *db, sqlite3_stmt *stmt) const;

  int WaitPrepare(sqlite3 *db, sqlite3_stmt **stmt,
                  const std::string &cmd) const;

  static int SpinExec(sqlite3 *db, const std::string &cmd);

  static int SpinExec(sqlite3 *db, sqlite3_stmt *stmt);

  static int SpinPrepare(sqlite3 *db, sqlite3_stmt **stmt,
                         const std::string &cmd);

  [[noreturn]] static void Error(sqlite3 *db, const std::string &cmd);

  static bool CheckError(sqlite3 *db, int rc, const std::string &cmd,
                         bool hardExit);

private:
  std::string m_dBname;
  mutable std::mutex m_mutex;
  sqlite3 *m_db{nullptr};
};

} // namespace ns3
#endif
