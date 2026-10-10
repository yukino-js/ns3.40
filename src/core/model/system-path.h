#ifndef SYSTEM_PATH_H
#define SYSTEM_PATH_H

#include <list>
#include <string>

namespace ns3 {

namespace SystemPath {

std::string FindSelfDirectory();

std::string Append(std::string left, std::string right);

std::list<std::string> Split(std::string path);

std::string Join(std::list<std::string>::const_iterator begin,
                 std::list<std::string>::const_iterator end);

std::list<std::string> ReadFiles(std::string path);

std::string MakeTemporaryDirectoryName();

void MakeDirectories(std::string path);

bool Exists(const std::string path);

std::string CreateValidSystemPath(const std::string path);

} // namespace SystemPath

} // namespace ns3

#endif
